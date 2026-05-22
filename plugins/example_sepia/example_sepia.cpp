// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
//
// Example Kmax plugin: a sepia tone filter contributing an FFmpeg `-vf`
// expression. Real plugins live in their own repositories — this one is
// included in-tree to demonstrate the API.

#include "plugins/ifilter.h"

#include <KPluginFactory>

using namespace Kmax;

class SepiaFilter : public IFilter
{
    Q_OBJECT
    Q_INTERFACES(Kmax::IFilter)
public:
    SepiaFilter(QObject *parent, const QVariantList &args)
        : IFilter(parent)
    {
        Q_UNUSED(args);
        setEnabled(true);
    }

    QString id() const override { return QStringLiteral("example-sepia"); }
    QString displayName() const override { return tr("Sepia tone (example plugin)"); }

    QString ffmpegExpression() const override
    {
        return QStringLiteral(
            "colorchannelmixer=.393:.769:.189:0:.349:.686:.168:0:.272:.534:.131");
    }

    int order() const override { return 200; }
};

K_PLUGIN_CLASS_WITH_JSON(SepiaFilter, "metadata.json")

#include "example_sepia.moc"
