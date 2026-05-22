// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "iplugin.h"

#include <QStringList>

namespace Kmax {

struct UpscaleRequest {
    QString inputFramesDir;
    QString outputFramesDir;
    int scale = 4;
    QString modelName;
    int gpuDeviceIndex = -1;
    int tileSize = 0;
    QVariantMap extra;
};

/**
 * An Upscaler turns a directory of input frames into a directory of
 * upscaled frames. Built-in: RealEsrganUpscaler (wraps the
 * realesrgan-ncnn-vulkan binary). Plugins may add Waifu2x, RIFE, etc.
 */
class IUpscaler : public IPlugin
{
    Q_OBJECT
public:
    explicit IUpscaler(QObject *parent = nullptr);
    ~IUpscaler() override;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual QStringList availableModels() const = 0;
    virtual QList<int> supportedScales() const = 0;
    virtual bool isAvailable() const = 0;

    /** Synchronously run upscaling. Emits progress(). Returns false on error. */
    virtual bool run(const UpscaleRequest &request) = 0;
    virtual void cancel() = 0;

    virtual QString lastError() const { return m_lastError; }

Q_SIGNALS:
    void progress(int percent, const QString &message);

protected:
    void setLastError(const QString &error) { m_lastError = error; }

private:
    QString m_lastError;
};

} // namespace Kmax

Q_DECLARE_INTERFACE(Kmax::IUpscaler, "org.kde.kmax.IUpscaler/1.0")
