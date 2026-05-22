// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "realesrganupscaler.h"

#include <QDir>
#include <QFileInfo>
#include <QProcess>
#include <QRegularExpression>
#include <QStandardPaths>

namespace Kmax {

RealEsrganUpscaler::RealEsrganUpscaler(QObject *parent)
    : IUpscaler(parent)
{
}

RealEsrganUpscaler::~RealEsrganUpscaler() = default;

QString RealEsrganUpscaler::findBinary()
{
    static const QStringList candidates{
        QStringLiteral("realesrgan-ncnn-vulkan"),
        QStringLiteral("realesrgan-ncnn-vulkan-bin"),
    };
    for (const auto &c : candidates) {
        const QString found = QStandardPaths::findExecutable(c);
        if (!found.isEmpty()) {
            return found;
        }
    }
    return {};
}

bool RealEsrganUpscaler::isAvailable() const
{
    return !findBinary().isEmpty();
}

QStringList RealEsrganUpscaler::availableModels() const
{
    // The binary ships models in /usr/share/realesrgan/models on the AUR package.
    // When models live elsewhere, the user can place them in
    // $XDG_DATA_HOME/kmax/realesrgan-models/ and we'll pick those up.
    QStringList models;
    const QStringList searchDirs{
        QStringLiteral("/usr/share/realesrgan/models"),
        QStandardPaths::writableLocation(QStandardPaths::AppDataLocation)
            + QStringLiteral("/realesrgan-models"),
    };
    for (const auto &dir : searchDirs) {
        QDir d(dir);
        if (!d.exists()) {
            continue;
        }
        const auto entries = d.entryList({QStringLiteral("*.param")}, QDir::Files);
        for (const auto &e : entries) {
            const QString name = QFileInfo(e).completeBaseName();
            if (!models.contains(name)) {
                models << name;
            }
        }
    }
    if (models.isEmpty()) {
        // Fall back to the canonical set bundled with the upstream release.
        models << QStringLiteral("realesrgan-x4plus")
               << QStringLiteral("realesrgan-x4plus-anime")
               << QStringLiteral("realesr-animevideov3");
    }
    return models;
}

bool RealEsrganUpscaler::run(const UpscaleRequest &request)
{
    m_cancelled = false;
    const QString bin = findBinary();
    if (bin.isEmpty()) {
        setLastError(tr("realesrgan-ncnn-vulkan binary not found"));
        return false;
    }

    QDir().mkpath(request.outputFramesDir);

    QStringList args{
        QStringLiteral("-i"), request.inputFramesDir,
        QStringLiteral("-o"), request.outputFramesDir,
        QStringLiteral("-s"), QString::number(request.scale),
        QStringLiteral("-f"), QStringLiteral("png"),
    };
    if (!request.modelName.isEmpty()) {
        args << QStringLiteral("-n") << request.modelName;
    }
    if (request.gpuDeviceIndex >= 0) {
        args << QStringLiteral("-g") << QString::number(request.gpuDeviceIndex);
    }
    if (request.tileSize > 0) {
        args << QStringLiteral("-t") << QString::number(request.tileSize);
    }

    m_process = new QProcess(this);
    m_process->setProcessChannelMode(QProcess::MergedChannels);
    static const QRegularExpression progressRe(QStringLiteral("(\\d+(?:\\.\\d+)?)%"));
    connect(m_process, &QProcess::readyReadStandardOutput, this, [this]() {
        const auto bytes = m_process->readAllStandardOutput();
        const auto match = progressRe.match(QString::fromUtf8(bytes));
        if (match.hasMatch()) {
            Q_EMIT progress(int(match.captured(1).toDouble()), tr("Upscaling…"));
        }
    });

    m_process->start(bin, args);
    if (!m_process->waitForStarted(10000)) {
        setLastError(tr("Failed to start %1: %2").arg(bin, m_process->errorString()));
        m_process->deleteLater();
        m_process = nullptr;
        return false;
    }
    // Wait indefinitely — the pipeline runs us off the GUI thread.
    m_process->waitForFinished(-1);
    const int rc = m_process->exitCode();
    m_process->deleteLater();
    m_process = nullptr;

    if (m_cancelled) {
        setLastError(tr("Upscaling cancelled by user"));
        return false;
    }
    if (rc != 0) {
        setLastError(tr("realesrgan-ncnn-vulkan exited with code %1").arg(rc));
        return false;
    }
    Q_EMIT progress(100, tr("Upscaling complete"));
    return true;
}

void RealEsrganUpscaler::cancel()
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
