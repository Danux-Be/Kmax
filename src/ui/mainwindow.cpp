// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "mainwindow.h"

#include "../core/job.h"
#include "../core/jobqueue.h"
#include <kmaxsettings.h>
#include "../plugins/iencoder.h"
#include "../plugins/ifilter.h"
#include "../plugins/isource.h"
#include "../plugins/iupscaler.h"
#include "../plugins/pluginmanager.h"
#include "jobwidget.h"
#include "settingsdialog.h"

#include <KActionCollection>
#include <KLocalizedString>
#include <KMessageBox>
#include <KStandardAction>

#include <QApplication>
#include <QDialog>
#include <QDragEnterEvent>
#include <QFileDialog>
#include <QFileInfo>
#include <QFormLayout>
#include <QLabel>
#include <QListWidget>
#include <QMimeData>
#include <QScrollArea>
#include <QStatusBar>
#include <QUrl>
#include <QVBoxLayout>

namespace Kmax {

MainWindow::MainWindow(QWidget *parent)
    : KXmlGuiWindow(parent)
{
    m_plugins = new PluginManager(this);
    m_plugins->discover();

    m_queue = new JobQueue(m_plugins, this);
    connect(m_queue, &JobQueue::jobAdded, this, &MainWindow::onJobAdded);
    connect(m_queue, &JobQueue::jobRemoved, this, &MainWindow::onJobRemoved);

    auto *central = new QWidget(this);
    auto *centralLayout = new QVBoxLayout(central);

    auto *banner = new QLabel(
        i18n("<b>Drop video files here</b> — or use <i>File → Add file…</i> / <i>File → Add DVD…</i>"),
        central);
    banner->setAlignment(Qt::AlignCenter);
    banner->setMinimumHeight(80);
    banner->setStyleSheet(QStringLiteral(
        "QLabel { border: 2px dashed palette(mid); border-radius: 6px; padding: 12px; }"));
    centralLayout->addWidget(banner);

    m_scroll = new QScrollArea(central);
    m_scroll->setWidgetResizable(true);
    m_jobsContainer = new QWidget(m_scroll);
    m_jobsLayout = new QVBoxLayout(m_jobsContainer);
    m_jobsLayout->addStretch();
    m_scroll->setWidget(m_jobsContainer);
    centralLayout->addWidget(m_scroll, 1);

    setCentralWidget(central);
    setAcceptDrops(true);

    setupActions();
    setupGUI(Default, QStringLiteral("kmaxui.rc"));

    statusBar()->showMessage(i18n("Ready — %1 source plugin(s), %2 upscaler(s), %3 encoder(s)",
                                   m_plugins->sources().size(),
                                   m_plugins->upscalers().size(),
                                   m_plugins->encoders().size()));
}

MainWindow::~MainWindow() = default;

void MainWindow::setupActions()
{
    auto *open = actionCollection()->addAction(QStringLiteral("file_add"), this, &MainWindow::openFiles);
    open->setText(i18n("Add file…"));
    open->setIcon(QIcon::fromTheme(QStringLiteral("document-open")));
    actionCollection()->setDefaultShortcut(open, QKeySequence::Open);

    auto *dvd = actionCollection()->addAction(QStringLiteral("file_add_dvd"), this, &MainWindow::openDvd);
    dvd->setText(i18n("Add DVD…"));
    dvd->setIcon(QIcon::fromTheme(QStringLiteral("media-optical")));

    auto *plugins = actionCollection()->addAction(QStringLiteral("plugins_about"), this,
                                                  &MainWindow::showAboutPlugins);
    plugins->setText(i18n("Available plugins…"));
    plugins->setIcon(QIcon::fromTheme(QStringLiteral("preferences-plugin")));

    KStandardAction::preferences(this, &MainWindow::showSettings, actionCollection());
    KStandardAction::quit(qApp, &QApplication::closeAllWindows, actionCollection());
}

void MainWindow::openFiles()
{
    QStringList nameFilters;
    QStringList extensions;
    for (auto *src : m_plugins->sources()) {
        for (const auto &ext : src->supportedExtensions()) {
            extensions << QStringLiteral("*.") + ext;
        }
    }
    extensions.removeDuplicates();
    if (!extensions.isEmpty()) {
        nameFilters << i18n("Video files (%1)", extensions.join(QLatin1Char(' ')));
    }
    nameFilters << i18n("All files (*)");

    const auto urls = QFileDialog::getOpenFileUrls(
        this, i18n("Add video files"),
        KmaxSettings::self()->defaultOutputDirectory(),
        nameFilters.join(QStringLiteral(";;")));
    for (const auto &u : urls) {
        enqueueUrl(u);
    }
}

void MainWindow::openDvd()
{
    auto *dvd = m_plugins->source(QStringLiteral("dvd"));
    if (!dvd) {
        KMessageBox::information(this,
            i18n("This build of Kmax has no DVD support. Rebuild with libdvdread and libdvdnav installed."),
            i18n("DVD support unavailable"));
        return;
    }
    const QString dir = QFileDialog::getExistingDirectory(this, i18n("Pick a DVD mount point or VIDEO_TS folder"),
                                                          QStringLiteral("/run/media"));
    if (dir.isEmpty()) {
        return;
    }
    enqueueUrl(QUrl::fromLocalFile(dir));
}

void MainWindow::enqueueUrl(const QUrl &url)
{
    ISource *match = nullptr;
    for (auto *src : m_plugins->sources()) {
        if (src->canHandle(url)) {
            match = src;
            break;
        }
    }
    if (!match) {
        KMessageBox::error(this, i18n("No source plugin can handle %1", url.toDisplayString()));
        return;
    }
    const auto items = match->probe(url);
    if (items.isEmpty()) {
        KMessageBox::error(this, i18n("Failed to probe %1: %2", url.toDisplayString(), match->lastError()));
        return;
    }

    for (const auto &item : items) {
        auto *job = new Job;
        job->source = item;
        job->sourcePluginId = match->id();

        // Apply user defaults from settings
        auto *settings = KmaxSettings::self();
        job->upscalerId = settings->defaultUpscaler();
        job->upscaleModel = settings->defaultUpscaleModel();
        job->upscaleScale = settings->defaultUpscaleFactor();
        job->tileSize = settings->tileSize();
        job->gpuDeviceIndex = settings->gpuDeviceIndex();
        job->encoderId = settings->defaultEncoder();
        switch (settings->defaultVideoCodec()) {
        case KmaxSettings::EnumDefaultVideoCodec::H264:
            job->outputCodec = QStringLiteral("libx264");
            break;
        case KmaxSettings::EnumDefaultVideoCodec::H265:
            job->outputCodec = QStringLiteral("libx265");
            break;
        case KmaxSettings::EnumDefaultVideoCodec::AV1:
            job->outputCodec = QStringLiteral("libsvtav1");
            break;
        }
        job->crf = settings->defaultCrf();

        if (settings->autoDeinterlace()) {
            job->filterIds << QStringLiteral("deinterlace");
            m_plugins->filter(QStringLiteral("deinterlace"))->setEnabled(true);
        }
        if (settings->autoDenoise()) {
            job->filterIds << QStringLiteral("denoise");
            m_plugins->filter(QStringLiteral("denoise"))->setEnabled(true);
        }
        if (settings->autoColorCorrect()) {
            job->filterIds << QStringLiteral("color-auto");
            m_plugins->filter(QStringLiteral("color-auto"))->setEnabled(true);
        }

        const QString outBase = settings->defaultOutputDirectory().toLocalFile();
        job->outputFile = outBase + QStringLiteral("/") + item.title
                          + QStringLiteral("_kmax_x") + QString::number(job->upscaleScale)
                          + QStringLiteral(".mp4");

        m_queue->addJob(job);
    }
}

void MainWindow::onJobAdded(Job *job)
{
    auto *widget = new JobWidget(job, m_jobsContainer);
    connect(widget, &JobWidget::removeRequested, m_queue, &JobQueue::removeJob);
    m_jobsLayout->insertWidget(m_jobsLayout->count() - 1, widget);
    m_widgets.insert(job, widget);
}

void MainWindow::onJobRemoved(Job *job)
{
    auto *widget = m_widgets.take(job);
    if (widget) {
        m_jobsLayout->removeWidget(widget);
        widget->deleteLater();
    }
}

void MainWindow::showSettings()
{
    if (SettingsDialog::showDialog(QStringLiteral("settings"))) {
        return;
    }
    auto *dlg = new SettingsDialog(this, QStringLiteral("settings"));
    dlg->show();
}

void MainWindow::showAboutPlugins()
{
    QDialog dlg(this);
    dlg.setWindowTitle(i18n("Loaded Kmax plugins"));
    auto *layout = new QFormLayout(&dlg);
    auto *src = new QListWidget(&dlg);
    for (auto *s : m_plugins->sources()) {
        src->addItem(QStringLiteral("%1 — %2").arg(s->metaObject()->className(),
                                                    s->supportedExtensions().join(QLatin1Char(' '))));
    }
    layout->addRow(i18n("Sources:"), src);
    auto *flt = new QListWidget(&dlg);
    for (auto *f : m_plugins->filters()) {
        flt->addItem(QStringLiteral("%1 — %2").arg(f->id(), f->displayName()));
    }
    layout->addRow(i18n("Filters:"), flt);
    auto *up = new QListWidget(&dlg);
    for (auto *u : m_plugins->upscalers()) {
        up->addItem(QStringLiteral("%1 — %2 %3").arg(u->id(), u->displayName(),
                                                      u->isAvailable() ? QStringLiteral("✓")
                                                                       : i18n("(unavailable)")));
    }
    layout->addRow(i18n("Upscalers:"), up);
    auto *enc = new QListWidget(&dlg);
    for (auto *e : m_plugins->encoders()) {
        enc->addItem(QStringLiteral("%1 — %2").arg(e->id(), e->displayName()));
    }
    layout->addRow(i18n("Encoders:"), enc);
    dlg.resize(560, 520);
    dlg.exec();
}

void MainWindow::dragEnterEvent(QDragEnterEvent *event)
{
    if (event->mimeData()->hasUrls()) {
        event->acceptProposedAction();
    }
}

void MainWindow::dropEvent(QDropEvent *event)
{
    const auto urls = event->mimeData()->urls();
    for (const auto &u : urls) {
        enqueueUrl(u);
    }
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    m_queue->cancelAll();
    KXmlGuiWindow::closeEvent(event);
}

} // namespace Kmax
