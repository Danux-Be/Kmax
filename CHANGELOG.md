# Changelog

All notable changes to **Kmax** are documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

## [Unreleased]

## [0.1.0] - 2026-05-22

### Added
- Initial project skeleton: CMake build system with Qt 6 and KDE Frameworks 6.
- Native KDE main window (`KXmlGuiWindow`) with menu, toolbar and status bar.
- Configurable working directory in settings — lets users place intermediate
  frames and caches on a larger external disk to avoid filling `$HOME`.
- KConfig-based settings (working directory, default upscaler, GPU device,
  default encoder, parallel job count).
- Plugin architecture based on `KPluginFactory` / `KPluginMetaData` with four
  interface types:
  - `ISource` — import sources (video files, DVDs, future capture devices).
  - `IFilter` — frame-in / frame-out processing filters.
  - `IUpscaler` — AI upscaling backends.
  - `IEncoder` — output encoders / containers.
- Built-in backends:
  - `FileSource` — any FFmpeg-readable container (avi, mp4, mkv, mov, …).
  - `DvdSource` — VOB/IFO via libdvdread/libdvdnav (when found at build time).
  - `RealEsrganUpscaler` — wraps `realesrgan-ncnn-vulkan` (vendor-agnostic
    Vulkan, runs on AMD, NVIDIA and Intel GPUs).
  - `FFmpegEncoder` — H.264 / H.265 / AV1 via libx264, libx265, libsvtav1.
  - Built-in FFmpeg-based filters: deinterlace (yadif), denoise (hqdn3d),
    color correction (eq).
- Job queue with serial / configurable-parallel execution.
- Example plugin: `kmax_filter_sepia` — demonstrates how to write an out-of-tree
  filter plugin.
- Plugin developer documentation (`docs/plugin-api.md`).
- Internationalisation scaffolding via `KI18n` (translation catalogues to come).

[Unreleased]: https://github.com/Danux-Be/Kmax/compare/v0.1.0...HEAD
[0.1.0]: https://github.com/Danux-Be/Kmax/releases/tag/v0.1.0
