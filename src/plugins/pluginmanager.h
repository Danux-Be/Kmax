// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QHash>
#include <QObject>
#include <QString>
#include <QStringList>

#include "iencoder.h"
#include "ifilter.h"
#include "isource.h"
#include "iupscaler.h"

namespace Kmax {

/**
 * Discovers built-in and external Kmax plugins via KPluginMetaData and
 * KPluginFactory. External plugins are searched in:
 *   - $XDG_DATA_DIRS/kmax/plugins/
 *   - $XDG_DATA_HOME/kmax/plugins/
 *   - $KMAX_PLUGIN_PATH (colon-separated, for development)
 */
class PluginManager : public QObject
{
    Q_OBJECT
public:
    explicit PluginManager(QObject *parent = nullptr);
    ~PluginManager() override;

    void discover();

    QList<ISource *> sources() const { return m_sources.values(); }
    QList<IFilter *> filters() const { return m_filters.values(); }
    QList<IUpscaler *> upscalers() const { return m_upscalers.values(); }
    QList<IEncoder *> encoders() const { return m_encoders.values(); }

    ISource *source(const QString &id) const { return m_sources.value(id); }
    IFilter *filter(const QString &id) const { return m_filters.value(id); }
    IUpscaler *upscaler(const QString &id) const { return m_upscalers.value(id); }
    IEncoder *encoder(const QString &id) const { return m_encoders.value(id); }

Q_SIGNALS:
    void pluginsChanged();

private:
    void registerBuiltins();
    void loadExternal();
    QStringList pluginSearchPaths() const;

    QHash<QString, ISource *> m_sources;
    QHash<QString, IFilter *> m_filters;
    QHash<QString, IUpscaler *> m_upscalers;
    QHash<QString, IEncoder *> m_encoders;
};

} // namespace Kmax
