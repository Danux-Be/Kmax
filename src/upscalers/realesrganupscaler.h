// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "../plugins/iupscaler.h"

class QProcess;

namespace Kmax {

class RealEsrganUpscaler : public IUpscaler
{
    Q_OBJECT
    Q_INTERFACES(Kmax::IUpscaler)
public:
    explicit RealEsrganUpscaler(QObject *parent = nullptr);
    ~RealEsrganUpscaler() override;

    QString id() const override { return QStringLiteral("realesrgan-ncnn-vulkan"); }
    QString displayName() const override { return tr("Real-ESRGAN (Vulkan)"); }
    QStringList availableModels() const override;
    QList<int> supportedScales() const override { return {2, 3, 4}; }
    bool isAvailable() const override;

    bool run(const UpscaleRequest &request) override;
    void cancel() override;

private:
    static QString findBinary();
    QProcess *m_process = nullptr;
    bool m_cancelled = false;
};

} // namespace Kmax
