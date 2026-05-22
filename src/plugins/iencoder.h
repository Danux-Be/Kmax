// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "iplugin.h"

#include <QStringList>

namespace Kmax {

struct EncodeRequest {
    QString inputFramesDir;
    QString outputFile;
    QString audioSourceFile;
    double fps = 0.0;
    QString codec;
    int crf = 20;
    QString preset;
    QVariantMap extra;
};

/**
 * An Encoder turns a directory of frames (plus optional audio source) into
 * a final video file. Built-in: FFmpegEncoder. Plugins may add hardware
 * encoders, alternative containers, or HDR-aware encoders.
 */
class IEncoder : public IPlugin
{
    Q_OBJECT
public:
    explicit IEncoder(QObject *parent = nullptr);
    ~IEncoder() override;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;
    virtual QStringList supportedCodecs() const = 0;
    virtual QStringList supportedContainers() const = 0;

    virtual bool run(const EncodeRequest &request) = 0;
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

Q_DECLARE_INTERFACE(Kmax::IEncoder, "org.kde.kmax.IEncoder/1.0")
