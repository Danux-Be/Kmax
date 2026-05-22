# Kmax

**Kmax** is a native KDE application for automatic video enhancement and AI
upscaling. It restores videos from heterogeneous sources — old AVI files,
DVDs, camcorder footage — and upscales them using
[Real-ESRGAN](https://github.com/xinntao/Real-ESRGAN) via the
`realesrgan-ncnn-vulkan` backend (Vulkan, vendor-agnostic — works on AMD,
NVIDIA and Intel GPUs).

Kmax is built on Qt 6 and KDE Frameworks 6, and ships an extensible plugin
architecture so the community can add new sources, filters, encoders and AI
upscaling models over time.

## Features (v0.1.0)

- Import videos from files (any FFmpeg-readable container) and DVDs
  (libdvdread / libdvdnav).
- Automatic enhancement pipeline: deinterlace, denoise, color correction.
- AI upscaling via Real-ESRGAN-ncnn-vulkan (cross-vendor GPU support).
- Job queue with configurable parallelism.
- **User-configurable working directory** — place intermediate frames on a
  larger external disk to avoid filling `$HOME`.
- Native KDE integration (KXmlGui, KConfig, KIO, KCrash, KI18n).
- Extensible plugin API with four interface types: `ISource`, `IFilter`,
  `IUpscaler`, `IEncoder`.

## Build dependencies

Arch / CachyOS:

```sh
sudo pacman -S --needed \
    cmake extra-cmake-modules \
    qt6-base qt6-multimedia \
    kcoreaddons ki18n kxmlgui kconfigwidgets kwidgetsaddons \
    kio kcrash kdbusaddons kiconthemes \
    ffmpeg libdvdread libdvdnav
paru -S --needed realesrgan-ncnn-vulkan-bin
```

## Build

```sh
cmake -B build -S . -DCMAKE_BUILD_TYPE=Release
cmake --build build -j
sudo cmake --install build
```

## Run

```sh
kmax
```

## Versioning & Changelog

Kmax follows [Semantic Versioning](https://semver.org/). All notable changes
are recorded in [`CHANGELOG.md`](./CHANGELOG.md) in the
[Keep a Changelog](https://keepachangelog.com/) format.

## Plugins

Plugins are KDE-style shared libraries discovered at runtime via
`KPluginMetaData`. See [`docs/plugin-api.md`](./docs/plugin-api.md) for the
developer guide and the [`plugins/example_sepia`](./plugins/example_sepia)
plugin for a complete working example.

## License

GPL-3.0-or-later — see [`LICENSE`](./LICENSE).
