# Kmax plugin API — v1.0

Kmax plugins are KDE shared libraries discovered at runtime via
`KPluginMetaData` and instantiated through `KPluginFactory`. A plugin is a
single C++ class that inherits one of four interfaces:

| Interface    | Purpose                                                  |
|--------------|----------------------------------------------------------|
| `ISource`    | Make a new kind of input available (capture, ISO, …).    |
| `IFilter`    | Add a processing step to the FFmpeg `-vf` chain.         |
| `IUpscaler`  | Provide an alternative AI upscaling backend.             |
| `IEncoder`   | Add output formats / codecs.                             |

## Discovery

At startup the `PluginManager` searches the following paths in order:

1. `$XDG_DATA_DIRS/kmax/plugins/`
2. `$XDG_DATA_HOME/kmax/plugins/`
3. `$KMAX_PLUGIN_PATH` (colon-separated, for development)
4. `<application-dir>/../plugins/`

Each plugin folder must contain a shared library plus a `metadata.json`
file embedded via the `K_PLUGIN_CLASS_WITH_JSON()` macro.

## Required metadata fields

```json
{
    "KPlugin": {
        "Id":          "kmax_filter_yourfilter",
        "Name":        "Your filter",
        "Description": "What your filter does",
        "Authors":     [ { "Name": "...", "Email": "..." } ],
        "Version":     "0.1.0",
        "License":     "GPL-3.0-or-later",
        "Icon":        "view-filter"
    },
    "X-Kmax-Type":       "Filter",
    "X-Kmax-ApiVersion": 1
}
```

`X-Kmax-Type` must be one of `Source`, `Filter`, `Upscaler`, `Encoder`.
`X-Kmax-ApiVersion` must match `Kmax::PluginApiVersionMajor` of the host
(currently `1`). The host refuses to load plugins with a mismatching API
version.

## Minimal filter plugin

```cpp
#include "plugins/ifilter.h"
#include <KPluginFactory>

class MyFilter : public Kmax::IFilter
{
    Q_OBJECT
    Q_INTERFACES(Kmax::IFilter)
public:
    MyFilter(QObject *parent, const QVariantList &) : IFilter(parent) {}
    QString id() const override { return QStringLiteral("my-filter"); }
    QString displayName() const override { return tr("My filter"); }
    QString ffmpegExpression() const override { return QStringLiteral("eq=brightness=0.05"); }
};

K_PLUGIN_CLASS_WITH_JSON(MyFilter, "metadata.json")
#include "myfilter.moc"
```

## Building out-of-tree

```cmake
cmake_minimum_required(VERSION 3.20)
project(kmax_filter_myfilter)

find_package(ECM REQUIRED NO_MODULE)
list(APPEND CMAKE_MODULE_PATH ${ECM_MODULE_PATH})
include(KDEInstallDirs)
include(KDECMakeSettings)

find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(KF6 REQUIRED COMPONENTS CoreAddons I18n)
find_package(Kmax REQUIRED)   # not yet shipped; use Kmax headers from source for now

add_library(kmax_filter_myfilter MODULE myfilter.cpp)
target_link_libraries(kmax_filter_myfilter PRIVATE
    Qt6::Core KF6::CoreAddons KF6::I18n)
set_target_properties(kmax_filter_myfilter PROPERTIES AUTOMOC ON PREFIX "")
install(TARGETS kmax_filter_myfilter DESTINATION ${KDE_INSTALL_PLUGINDIR}/kmax)
```

See [`plugins/example_sepia/`](../plugins/example_sepia/) for a complete
working example.

## Interface contracts

### `ISource`

| Method                                    | Purpose                                    |
|-------------------------------------------|--------------------------------------------|
| `canHandle(QUrl)`                         | Quick test whether the plugin opens a URI. |
| `probe(QUrl)`                             | Return zero or more `SourceItem`s.         |
| `materialise(SourceItem, workingDir)`     | Produce a local file FFmpeg can read.      |

Long extractions should emit `progress(int, QString)`.

### `IFilter`

Filters in v1 declare an FFmpeg `-vf` expression via `ffmpegExpression()`.
A future API revision will expose a per-frame Qt/Vulkan path for filters
that need GPU access or non-trivial logic.

### `IUpscaler`

| Method                                | Purpose                                |
|---------------------------------------|----------------------------------------|
| `availableModels()`                   | Enumerate models for the UI.           |
| `supportedScales()`                   | List supported scale factors.          |
| `isAvailable()`                       | Runtime check (binary present, …).     |
| `run(UpscaleRequest)`                 | Synchronous run, blocking.             |

### `IEncoder`

| Method                                | Purpose                                |
|---------------------------------------|----------------------------------------|
| `supportedCodecs()`                   | Codec ids for the UI.                  |
| `supportedContainers()`               | Container ids for the UI.              |
| `run(EncodeRequest)`                  | Synchronous run, blocking.             |

## Threading

The host runs `Pipeline::run()` (and therefore `IUpscaler::run()` and
`IEncoder::run()`) on a worker thread via `QtConcurrent::run`. Plugins
**must not** touch Qt widgets directly. Communicate progress via the
`progress(int, QString)` signal — the host marshals it back to the GUI
thread.

## Versioning policy

The plugin API follows semantic versioning. The major version is
`Kmax::PluginApiVersionMajor`; bumping it requires a recompilation of
every plugin. Minor bumps add backward-compatible additions only.
