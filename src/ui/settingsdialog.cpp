// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "settingsdialog.h"

#include <kmaxsettings.h>

#include <KLocalizedString>
#include <KUrlRequester>

#include <QCheckBox>
#include <QComboBox>
#include <QFormLayout>
#include <QLabel>
#include <QSpinBox>
#include <QVBoxLayout>
#include <QWidget>

namespace Kmax {

SettingsDialog::SettingsDialog(QWidget *parent, const QString &name)
    : KConfigDialog(parent, name, KmaxSettings::self())
{
    setFaceType(KPageDialog::List);

    // --- Storage page -------------------------------------------------
    auto *storage = new QWidget;
    auto *storageLayout = new QFormLayout(storage);

    auto *workingDir = new KUrlRequester(storage);
    workingDir->setObjectName(QStringLiteral("kcfg_workingDirectory"));
    workingDir->setMode(KFile::Directory | KFile::LocalOnly);
    storageLayout->addRow(i18n("Working directory:"), workingDir);
    auto *workingHint = new QLabel(
        i18n("Intermediate frames are stored here. Pick a disk with plenty of free space — "
             "a 90-minute 1080p video can produce 50–200 GiB of temporary data."), storage);
    workingHint->setWordWrap(true);
    workingHint->setForegroundRole(QPalette::PlaceholderText);
    storageLayout->addRow(QString(), workingHint);

    auto *outputDir = new KUrlRequester(storage);
    outputDir->setObjectName(QStringLiteral("kcfg_defaultOutputDirectory"));
    outputDir->setMode(KFile::Directory | KFile::LocalOnly);
    storageLayout->addRow(i18n("Default output directory:"), outputDir);

    auto *minFree = new QSpinBox(storage);
    minFree->setObjectName(QStringLiteral("kcfg_minFreeSpaceGiB"));
    minFree->setRange(1, 10000);
    minFree->setSuffix(QStringLiteral(" GiB"));
    storageLayout->addRow(i18n("Warn below free space:"), minFree);

    auto *keepIntermediate = new QCheckBox(i18n("Keep intermediate frames after processing"), storage);
    keepIntermediate->setObjectName(QStringLiteral("kcfg_keepIntermediateFiles"));
    storageLayout->addRow(QString(), keepIntermediate);

    addPage(storage, i18n("Storage"), QStringLiteral("drive-harddisk-symbolic"));

    // --- Processing page ---------------------------------------------
    auto *processing = new QWidget;
    auto *procLayout = new QFormLayout(processing);

    auto *parallelJobs = new QSpinBox(processing);
    parallelJobs->setObjectName(QStringLiteral("kcfg_parallelJobs"));
    parallelJobs->setRange(1, 16);
    procLayout->addRow(i18n("Parallel jobs:"), parallelJobs);

    auto *scale = new QSpinBox(processing);
    scale->setObjectName(QStringLiteral("kcfg_defaultUpscaleFactor"));
    scale->setRange(2, 4);
    scale->setSuffix(QStringLiteral("×"));
    procLayout->addRow(i18n("Default upscale factor:"), scale);

    auto *gpu = new QSpinBox(processing);
    gpu->setObjectName(QStringLiteral("kcfg_gpuDeviceIndex"));
    gpu->setRange(-1, 8);
    gpu->setSpecialValueText(i18n("Auto-detect"));
    procLayout->addRow(i18n("Vulkan GPU index:"), gpu);

    auto *tile = new QSpinBox(processing);
    tile->setObjectName(QStringLiteral("kcfg_tileSize"));
    tile->setRange(0, 1024);
    tile->setSpecialValueText(i18n("Auto"));
    procLayout->addRow(i18n("Tile size:"), tile);

    addPage(processing, i18n("Processing"), QStringLiteral("preferences-system-symbolic"));

    // --- Encoding page ------------------------------------------------
    auto *encoding = new QWidget;
    auto *encLayout = new QFormLayout(encoding);

    auto *codec = new QComboBox(encoding);
    codec->setObjectName(QStringLiteral("kcfg_defaultVideoCodec"));
    codec->addItem(QStringLiteral("H.264"));
    codec->addItem(QStringLiteral("H.265 (HEVC)"));
    codec->addItem(QStringLiteral("AV1"));
    encLayout->addRow(i18n("Default codec:"), codec);

    auto *crf = new QSpinBox(encoding);
    crf->setObjectName(QStringLiteral("kcfg_defaultCrf"));
    crf->setRange(0, 51);
    encLayout->addRow(i18n("Default CRF (quality):"), crf);

    addPage(encoding, i18n("Encoding"), QStringLiteral("video-x-generic-symbolic"));

    // --- Filters page -------------------------------------------------
    auto *filters = new QWidget;
    auto *fLayout = new QVBoxLayout(filters);
    for (const auto &[obj, label] : std::initializer_list<std::pair<QString, QString>>{
             {QStringLiteral("kcfg_autoDeinterlace"), i18n("Automatically deinterlace interlaced content")},
             {QStringLiteral("kcfg_autoDenoise"),     i18n("Apply temporal denoising")},
             {QStringLiteral("kcfg_autoColorCorrect"), i18n("Apply auto colour correction")},
         }) {
        auto *cb = new QCheckBox(label, filters);
        cb->setObjectName(obj);
        fLayout->addWidget(cb);
    }
    fLayout->addStretch();
    addPage(filters, i18n("Filters"), QStringLiteral("view-filter-symbolic"));
}

} // namespace Kmax
