// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QHash>
#include <QList>
#include <QObject>

namespace Kmax {

class Job;
class Pipeline;
class PluginManager;

class JobQueue : public QObject
{
    Q_OBJECT
public:
    explicit JobQueue(PluginManager *plugins, QObject *parent = nullptr);
    ~JobQueue() override;

    /** Enqueue and take ownership of the job. */
    void addJob(Job *job);
    void removeJob(Job *job);
    void cancelAll();

    QList<Job *> jobs() const { return m_jobs; }

Q_SIGNALS:
    void jobAdded(Kmax::Job *job);
    void jobRemoved(Kmax::Job *job);
    void queueIdle();

private:
    void runNext();

    PluginManager *m_plugins;
    QList<Job *> m_jobs;
    int m_running = 0;
};

} // namespace Kmax
