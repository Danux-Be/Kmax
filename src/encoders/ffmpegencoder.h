// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "../plugins/iencoder.h"

class QProcess;

namespace Kmax {

class FFmpegEncoder : public IEncoder
{
    Q_OBJECT
    Q_INTERFACES(Kmax::IEncoder)
public:
    explicit FFmpegEncoder(QObject *parent = nullptr);
    ~FFmpegEncoder() override;

    QString id() const override { return QStringLiteral("ffmpeg"); }
    QString displayName() const override { return tr("FFmpeg encoder"); }
    QStringList supportedCodecs() const override;
    QStringList supportedContainers() const override;

    bool run(const EncodeRequest &request) override;
    void cancel() override;

private:
    QProcess *m_process = nullptr;
    bool m_cancelled = false;
};

} // namespace Kmax
