// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "pluginmanager.h"

#include <KPluginFactory>
#include <KPluginMetaData>

#include <QCoreApplication>
#include <QDir>
#include <QDirIterator>
#include <QLoggingCategory>
#include <QStandardPaths>

#include "../encoders/ffmpegencoder.h"
#include "../filters/ffmpegfilter.h"
#include "../sources/filesource.h"
#include "../upscalers/realesrganupscaler.h"

#ifdef KMAX_HAVE_LIBDVD
#include "../sources/dvdsource.h"
#endif

Q_LOGGING_CATEGORY(KmaxPlugin, "org.kde.kmax.plugin")

namespace Kmax {

PluginManager::PluginManager(QObject *parent)
    : QObject(parent)
{
}

PluginManager::~PluginManager() = default;

void PluginManager::discover()
{
    qDeleteAll(m_sources);
    m_sources.clear();
    qDeleteAll(m_filters);
    m_filters.clear();
    qDeleteAll(m_upscalers);
    m_upscalers.clear();
    qDeleteAll(m_encoders);
    m_encoders.clear();

    registerBuiltins();
    loadExternal();

    Q_EMIT pluginsChanged();
}

void PluginManager::registerBuiltins()
{
    auto *file = new FileSource(this);
    m_sources.insert(QStringLiteral("file"), file);
#ifdef KMAX_HAVE_LIBDVD
    auto *dvd = new DvdSource(this);
    m_sources.insert(QStringLiteral("dvd"), dvd);
#endif

    auto addFilter = [this](const QString &id, const QString &name, const QString &expr, int order) {
        auto *f = new FFmpegFilter(id, name, expr, order, this);
        f->setEnabled(false);
        m_filters.insert(id, f);
    };
    addFilter(QStringLiteral("deinterlace"), tr("Deinterlace (yadif)"),
              QStringLiteral("yadif=1"), 10);
    addFilter(QStringLiteral("denoise"), tr("Denoise (hqdn3d)"),
              QStringLiteral("hqdn3d=2:1:2:1"), 20);
    addFilter(QStringLiteral("color-auto"), tr("Auto colour correction"),
              QStringLiteral("eq=contrast=1.05:saturation=1.05"), 30);

    auto *upscaler = new RealEsrganUpscaler(this);
    m_upscalers.insert(upscaler->id(), upscaler);

    auto *encoder = new FFmpegEncoder(this);
    m_encoders.insert(encoder->id(), encoder);
}

QStringList PluginManager::pluginSearchPaths() const
{
    QStringList paths;
    const auto data = QStandardPaths::standardLocations(QStandardPaths::GenericDataLocation);
    for (const auto &d : data) {
        paths << d + QStringLiteral("/kmax/plugins");
    }
    paths << QCoreApplication::applicationDirPath() + QStringLiteral("/../plugins");
    const QString env = qEnvironmentVariable("KMAX_PLUGIN_PATH");
    if (!env.isEmpty()) {
        paths += env.split(QLatin1Char(':'), Qt::SkipEmptyParts);
    }
    return paths;
}

void PluginManager::loadExternal()
{
    const auto paths = pluginSearchPaths();
    for (const auto &path : paths) {
        if (!QDir(path).exists()) {
            continue;
        }
        const auto metas = KPluginMetaData::findPlugins(path);
        for (const auto &meta : metas) {
            const QString type = meta.value(QStringLiteral("X-Kmax-Type"));
            const int api = meta.value(QStringLiteral("X-Kmax-ApiVersion"), 0);
            if (api != PluginApiVersionMajor) {
                qCWarning(KmaxPlugin) << "Skipping plugin" << meta.pluginId()
                                      << "(incompatible API version" << api
                                      << "expected" << PluginApiVersionMajor << ")";
                continue;
            }
            const auto result = KPluginFactory::instantiatePlugin<QObject>(meta, this);
            if (!result) {
                qCWarning(KmaxPlugin) << "Failed to load" << meta.pluginId()
                                      << ":" << result.errorString;
                continue;
            }
            QObject *obj = result.plugin;
            if (type == QStringLiteral("Source")) {
                if (auto *p = qobject_cast<ISource *>(obj)) {
                    m_sources.insert(meta.pluginId(), p);
                    continue;
                }
            } else if (type == QStringLiteral("Filter")) {
                if (auto *p = qobject_cast<IFilter *>(obj)) {
                    m_filters.insert(meta.pluginId(), p);
                    continue;
                }
            } else if (type == QStringLiteral("Upscaler")) {
                if (auto *p = qobject_cast<IUpscaler *>(obj)) {
                    m_upscalers.insert(meta.pluginId(), p);
                    continue;
                }
            } else if (type == QStringLiteral("Encoder")) {
                if (auto *p = qobject_cast<IEncoder *>(obj)) {
                    m_encoders.insert(meta.pluginId(), p);
                    continue;
                }
            }
            qCWarning(KmaxPlugin) << "Plugin" << meta.pluginId()
                                  << "did not implement the declared interface (" << type << ")";
            obj->deleteLater();
        }
    }
}

} // namespace Kmax
