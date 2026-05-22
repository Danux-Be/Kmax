// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <KXmlGuiWindow>

class QDragEnterEvent;
class QDropEvent;
class QVBoxLayout;
class QScrollArea;

namespace Kmax {

class Job;
class JobQueue;
class JobWidget;
class PluginManager;

class MainWindow : public KXmlGuiWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

protected:
    void dragEnterEvent(QDragEnterEvent *event) override;
    void dropEvent(QDropEvent *event) override;
    void closeEvent(QCloseEvent *event) override;

private Q_SLOTS:
    void openFiles();
    void openDvd();
    void showSettings();
    void showAboutPlugins();
    void onJobAdded(Kmax::Job *job);
    void onJobRemoved(Kmax::Job *job);

private:
    void setupActions();
    void enqueueUrl(const QUrl &url);
    Job *buildJobFromSource(const QString &sourceId, const QUrl &uri);

    PluginManager *m_plugins;
    JobQueue *m_queue;
    QScrollArea *m_scroll;
    QWidget *m_jobsContainer;
    QVBoxLayout *m_jobsLayout;
    QHash<Job *, JobWidget *> m_widgets;
};

} // namespace Kmax
