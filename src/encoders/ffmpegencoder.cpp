// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ffmpegencoder.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

namespace Kmax {

FFmpegEncoder::FFmpegEncoder(QObject *parent)
    : IEncoder(parent)
{
}

FFmpegEncoder::~FFmpegEncoder() = default;

QStringList FFmpegEncoder::supportedCodecs() const
{
    return {
        QStringLiteral("libx264"),
        QStringLiteral("libx265"),
        QStringLiteral("libsvtav1"),
        QStringLiteral("libaom-av1"),
        QStringLiteral("h264_nvenc"),
        QStringLiteral("hevc_nvenc"),
        QStringLiteral("h264_vaapi"),
        QStringLiteral("hevc_vaapi"),
    };
}

QStringList FFmpegEncoder::supportedContainers() const
{
    return {QStringLiteral("mp4"), QStringLiteral("mkv"), QStringLiteral("webm"), QStringLiteral("mov")};
}

bool FFmpegEncoder::run(const EncodeRequest &request)
{
    m_cancelled = false;
    const QString ffmpeg = QStandardPaths::findExecutable(QStringLiteral("ffmpeg"));
    if (ffmpeg.isEmpty()) {
        setLastError(tr("ffmpeg not found in PATH"));
        return false;
    }
    QFileInfo(request.outputFile).absoluteDir().mkpath(QStringLiteral("."));

    QStringList args{
        QStringLiteral("-y"),
        QStringLiteral("-framerate"), QString::number(request.fps > 0 ? request.fps : 25.0),
        QStringLiteral("-i"), request.inputFramesDir + QStringLiteral("/frame_%08d.png"),
    };
    if (!request.audioSourceFile.isEmpty()) {
        args << QStringLiteral("-i") << request.audioSourceFile
             << QStringLiteral("-map") << QStringLiteral("0:v:0")
             << QStringLiteral("-map") << QStringLiteral("1:a:0?")
             << QStringLiteral("-c:a") << QStringLiteral("aac")
             << QStringLiteral("-b:a") << QStringLiteral("192k");
    }
    const QString codec = request.codec.isEmpty() ? QStringLiteral("libx265") : request.codec;
    args << QStringLiteral("-c:v") << codec
         << QStringLiteral("-crf") << QString::number(request.crf)
         << QStringLiteral("-preset") << (request.preset.isEmpty() ? QStringLiteral("slow") : request.preset)
         << QStringLiteral("-pix_fmt") << QStringLiteral("yuv420p")
         << request.outputFile;

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    static const QRegularExpression timeRe(QStringLiteral("time=(\\d+):(\\d+):(\\d+\\.\\d+)"));
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this, request]() {
        const auto bytes = m_process->readAllStandardOutput();
        const auto match = timeRe.match(QString::fromUtf8(bytes));
        if (match.hasMatch() && request.extra.contains(QStringLiteral("durationMs"))) {
            const qint64 durMs = request.extra.value(QStringLiteral("durationMs")).toLongLong();
            if (durMs > 0) {
                const qint64 ms = (match.captured(1).toLongLong() * 3600
                                   + match.captured(2).toLongLong() * 60) * 1000
                                  + qRound(match.captured(3).toDouble() * 1000);
                Q_EMIT progress(int(qMin<qint64>(100, ms * 100 / durMs)), tr("Encoding…"));
            }
        }
    });

    m_process->start(ffmpeg, args);
    if (!m_process->waitForStarted(10000)) {
        setLastError(tr("Failed to start ffmpeg: %1").arg(m_process->errorString()));
        m_process->deleteLater();
        m_process = nullptr;
        return false;
    }
    m_process->waitForFinished(-1);
    const int rc = m_process->exitCode();
    m_process->deleteLater();
    m_process = nullptr;

    if (m_cancelled) {
        setLastError(tr("Encoding cancelled by user"));
        return false;
    }
    if (rc != 0) {
        setLastError(tr("ffmpeg exited with code %1").arg(rc));
        return false;
    }
    Q_EMIT progress(100, tr("Encoding complete"));
    return true;
}

void FFmpegEncoder::cancel()
{
    m_cancelled = true;
    if (m_process) {
        m_process->terminate();
        if (!m_process->waitForFinished(2000)) {
            m_process->kill();
        }
    }
}

} // namespace Kmax
