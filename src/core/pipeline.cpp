// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "pipeline.h"

#include "../plugins/iencoder.h"
#include "../plugins/ifilter.h"
#include "../plugins/isource.h"
#include "../plugins/iupscaler.h"
#include "../plugins/pluginmanager.h"
#include "job.h"
#include <kmaxsettings.h>

#include <QCryptographicHash>
#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>
#include <QStorageInfo>

#include <algorithm>

namespace Kmax {

Pipeline::Pipeline(PluginManager *plugins, QObject *parent)
    : QObject(parent)
    , m_plugins(plugins)
{
}

void Pipeline::cancel()
{
    m_cancelled = true;
    for (auto *u : m_plugins->upscalers()) {
        u->cancel();
    }
    for (auto *e : m_plugins->encoders()) {
        e->cancel();
    }
}

QString Pipeline::frameDirForJob(const QString &workingDir, const QString &jobId, const char *stage)
{
    const QString hash = QString::fromLatin1(
        QCryptographicHash::hash(jobId.toUtf8(), QCryptographicHash::Sha1).toHex().left(12));
    return workingDir + QStringLiteral("/jobs/") + hash + QStringLiteral("/") + QString::fromLatin1(stage);
}

QString Pipeline::buildFilterChain(Job *job) const
{
    QList<IFilter *> chain;
    for (const auto &id : std::as_const(job->filterIds)) {
        if (auto *f = m_plugins->filter(id); f && f->isEnabled()) {
            chain << f;
        }
    }
    std::sort(chain.begin(), chain.end(), [](IFilter *a, IFilter *b) {
        return a->order() < b->order();
    });
    QStringList parts;
    for (auto *f : std::as_const(chain)) {
        const QString expr = f->ffmpegExpression();
        if (!expr.isEmpty()) {
            parts << expr;
        }
    }
    return parts.join(QLatin1Char(','));
}

bool Pipeline::extractFrames(Job *job, const QString &inputFile, const QString &outDir,
                             const QString &filterChain)
{
    const QString ffmpeg = QStandardPaths::findExecutable(QStringLiteral("ffmpeg"));
    if (ffmpeg.isEmpty()) {
        job->setError(tr("ffmpeg not found in PATH"));
        return false;
    }
    QDir().mkpath(outDir);

    QStringList args{
        QStringLiteral("-y"),
        QStringLiteral("-i"), inputFile,
    };
    if (!filterChain.isEmpty()) {
        args << QStringLiteral("-vf") << filterChain;
    }
    args << QStringLiteral("-qscale:v") << QStringLiteral("2")
         << outDir + QStringLiteral("/frame_%08d.png");

    QProcess proc;
    proc.setProcessChannelMode(QProcess::MergedChannels);
    static const QRegularExpression timeRe(QStringLiteral("time=(\\d+):(\\d+):(\\d+\\.\\d+)"));
    QObject::connect(&proc, &QProcess::readyReadStandardOutput, &proc, [&proc, job]() {
        const auto bytes = proc.readAllStandardOutput();
        const auto match = timeRe.match(QString::fromUtf8(bytes));
        if (match.hasMatch() && job->source.durationMs > 0) {
            const qint64 ms = (match.captured(1).toLongLong() * 3600
                               + match.captured(2).toLongLong() * 60) * 1000
                              + qRound(match.captured(3).toDouble() * 1000);
            job->setProgress(int(qMin<qint64>(100, ms * 100 / job->source.durationMs)),
                             tr("Extracting frames…"));
        }
    });

    proc.start(ffmpeg, args);
    proc.waitForFinished(-1);
    if (m_cancelled) {
        job->setState(Job::State::Cancelled);
        return false;
    }
    if (proc.exitCode() != 0) {
        job->setError(tr("Frame extraction failed (ffmpeg exit %1)").arg(proc.exitCode()));
        return false;
    }
    return true;
}

bool Pipeline::run(Job *job)
{
    m_cancelled = false;
    auto *settings = KmaxSettings::self();
    const QString workingDir = settings->workingDirectory().toLocalFile();
    QDir().mkpath(workingDir);

    QStorageInfo storage(workingDir);
    if (storage.isValid() && storage.bytesAvailable() / (1024LL * 1024 * 1024)
                                 < settings->minFreeSpaceGiB()) {
        job->setError(tr("Not enough free space in working directory: %1 GiB available, %2 GiB required")
                          .arg(storage.bytesAvailable() / (1024LL * 1024 * 1024))
                          .arg(settings->minFreeSpaceGiB()));
        return false;
    }

    auto *source = m_plugins->source(job->sourcePluginId);
    if (!source) {
        job->setError(tr("Source plugin not found: %1").arg(job->sourcePluginId));
        return false;
    }
    auto *upscaler = m_plugins->upscaler(job->upscalerId);
    if (!upscaler || !upscaler->isAvailable()) {
        job->setError(tr("Upscaler not available: %1").arg(job->upscalerId));
        return false;
    }
    auto *encoder = m_plugins->encoder(job->encoderId);
    if (!encoder) {
        job->setError(tr("Encoder plugin not found: %1").arg(job->encoderId));
        return false;
    }

    job->setState(Job::State::Extracting);
    job->setProgress(0, tr("Preparing source…"));
    const QString localFile = source->materialise(job->source, workingDir);
    if (localFile.isEmpty()) {
        job->setError(source->lastError());
        return false;
    }

    const QString inFrames = frameDirForJob(workingDir, job->source.id, "in");
    const QString outFrames = frameDirForJob(workingDir, job->source.id, "up");

    if (!extractFrames(job, localFile, inFrames, buildFilterChain(job))) {
        return false;
    }
    if (m_cancelled) {
        job->setState(Job::State::Cancelled);
        return false;
    }

    job->setState(Job::State::Upscaling);
    UpscaleRequest ureq;
    ureq.inputFramesDir = inFrames;
    ureq.outputFramesDir = outFrames;
    ureq.scale = job->upscaleScale;
    ureq.modelName = job->upscaleModel;
    ureq.gpuDeviceIndex = job->gpuDeviceIndex;
    ureq.tileSize = job->tileSize;
    QObject::connect(upscaler, &IUpscaler::progress, job,
                     [job](int p, const QString &m) { job->setProgress(p, m); });
    const bool upOk = upscaler->run(ureq);
    QObject::disconnect(upscaler, &IUpscaler::progress, job, nullptr);
    if (!upOk) {
        if (m_cancelled) {
            job->setState(Job::State::Cancelled);
        } else {
            job->setError(upscaler->lastError());
        }
        return false;
    }

    job->setState(Job::State::Encoding);
    EncodeRequest ereq;
    ereq.inputFramesDir = outFrames;
    ereq.outputFile = job->outputFile;
    ereq.audioSourceFile = localFile;
    ereq.fps = job->source.fps > 0 ? job->source.fps : 25.0;
    ereq.codec = job->outputCodec;
    ereq.crf = job->crf;
    ereq.extra.insert(QStringLiteral("durationMs"), QVariant::fromValue(job->source.durationMs));
    QObject::connect(encoder, &IEncoder::progress, job,
                     [job](int p, const QString &m) { job->setProgress(p, m); });
    const bool encOk = encoder->run(ereq);
    QObject::disconnect(encoder, &IEncoder::progress, job, nullptr);
    if (!encOk) {
        if (m_cancelled) {
            job->setState(Job::State::Cancelled);
        } else {
            job->setError(encoder->lastError());
        }
        return false;
    }

    if (!settings->keepIntermediateFiles()) {
        QDir(inFrames).removeRecursively();
        QDir(outFrames).removeRecursively();
    }

    job->setState(Job::State::Finished);
    job->setProgress(100, tr("Done"));
    return true;
}

} // namespace Kmax
