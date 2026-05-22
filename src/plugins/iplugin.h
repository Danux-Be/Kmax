// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QString>
#include <QVariantMap>

namespace Kmax {

// Plugin API contract version. Plugins MUST be built against the same
// major version as the host. Bump on breaking changes only.
constexpr int PluginApiVersionMajor = 1;
constexpr int PluginApiVersionMinor = 0;

/**
 * Base class for every Kmax plugin. Concrete plugins inherit one of the
 * specialised interfaces (ISource, IFilter, IUpscaler, IEncoder).
 *
 * Plugins are KDE shared libraries that expose a single factory via the
 * K_PLUGIN_CLASS_WITH_JSON() macro and ship a metadata.json file with at
 * least these fields:
 *   - KPlugin.Id          unique plugin identifier
 *   - KPlugin.Name         human-readable name (translatable)
 *   - KPlugin.Description  short description (translatable)
 *   - KPlugin.Authors      list of authors
 *   - KPlugin.Version      plugin version (SemVer)
 *   - KPlugin.License      SPDX expression
 *   - X-Kmax-Type          one of: Source, Filter, Upscaler, Encoder
 *   - X-Kmax-ApiVersion    integer matching PluginApiVersionMajor
 */
class IPlugin : public QObject
{
    Q_OBJECT
public:
    explicit IPlugin(QObject *parent = nullptr);
    ~IPlugin() override;

    /** Free-form options forwarded from the host (settings, runtime hints). */
    virtual void configure(const QVariantMap &options) { Q_UNUSED(options); }
};

} // namespace Kmax
