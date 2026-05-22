// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "../plugins/isource.h"

namespace Kmax {

/**
 * Built-in source for any local video file FFmpeg can read.
 * Uses ffprobe for metadata; materialise() is a no-op returning the
 * original path.
 */
class FileSource : public ISource
{
    Q_OBJECT
    Q_INTERFACES(Kmax::ISource)
public:
    explicit FileSource(QObject *parent = nullptr);

    QString id() const override { return QStringLiteral("file"); }
    QString displayName() const override { return tr("Video file"); }
    QStringList supportedSchemes() const override;
    QStringList supportedExtensions() const override;
    bool canHandle(const QUrl &uri) const override;
    QList<SourceItem> probe(const QUrl &uri) override;
    QString materialise(const SourceItem &item, const QString &workingDir) override;
};

} // namespace Kmax
