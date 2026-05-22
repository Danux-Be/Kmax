// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "iplugin.h"

#include <QStringList>
#include <QUrl>

namespace Kmax {

struct SourceItem {
    QString id;
    QString title;
    qint64 durationMs = 0;
    int width = 0;
    int height = 0;
    double fps = 0.0;
    QUrl uri;
    QVariantMap extra;
};

/**
 * A Source produces a video stream that the pipeline can decode.
 * Built-in implementations include FileSource and (when libdvdread is
 * available) DvdSource. Plugins may add more (HDMI capture, V4L2, etc.).
 */
class ISource : public IPlugin
{
    Q_OBJECT
public:
    explicit ISource(QObject *parent = nullptr);
    ~ISource() override;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;

    virtual QStringList supportedSchemes() const = 0;
    virtual QStringList supportedExtensions() const = 0;

    virtual bool canHandle(const QUrl &uri) const = 0;

    /** Probe the URI and return zero or more items (e.g. DVD titles). */
    virtual QList<SourceItem> probe(const QUrl &uri) = 0;

    /**
     * Materialise the item into a single file path the downstream FFmpeg
     * stage can read. The returned path lives under @p workingDir.
     * Emits progress() during long extractions.
     */
    virtual QString materialise(const SourceItem &item, const QString &workingDir) = 0;

    virtual QString lastError() const { return m_lastError; }

Q_SIGNALS:
    void progress(int percent, const QString &message);

protected:
    void setLastError(const QString &error) { m_lastError = error; }

private:
    QString m_lastError;
};

} // namespace Kmax

Q_DECLARE_INTERFACE(Kmax::ISource, "org.kde.kmax.ISource/1.0")
