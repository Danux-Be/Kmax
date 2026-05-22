// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "../plugins/isource.h"

namespace Kmax {

/**
 * DVD source backed by libdvdread.
 *
 * - probe() enumerates titles (chapter count, duration, dimensions).
 * - materialise() dumps the title's VOB stream into a single .vob file
 *   inside the working directory so the rest of the pipeline (FFmpeg) can
 *   read it as a normal MPEG-PS file.
 */
class DvdSource : public ISource
{
    Q_OBJECT
    Q_INTERFACES(Kmax::ISource)
public:
    explicit DvdSource(QObject *parent = nullptr);

    QString id() const override { return QStringLiteral("dvd"); }
    QString displayName() const override { return tr("DVD"); }
    QStringList supportedSchemes() const override;
    QStringList supportedExtensions() const override { return {}; }
    bool canHandle(const QUrl &uri) const override;
    QList<SourceItem> probe(const QUrl &uri) override;
    QString materialise(const SourceItem &item, const QString &workingDir) override;
};

} // namespace Kmax
