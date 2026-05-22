// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "jobqueue.h"

#include "../plugins/pluginmanager.h"
#include "job.h"
#include <kmaxsettings.h>
#include "pipeline.h"

#include <QThread>
#include <QtConcurrent>

namespace Kmax {

JobQueue::JobQueue(PluginManager *plugins, QObject *parent)
    : QObject(parent)
    , m_plugins(plugins)
{
}

JobQueue::~JobQueue() = default;

void JobQueue::addJob(Job *job)
{
    job->setParent(this);
    m_jobs << job;
    Q_EMIT jobAdded(job);
    runNext();
}

void JobQueue::removeJob(Job *job)
{
    if (!m_jobs.removeOne(job)) {
        return;
    }
    Q_EMIT jobRemoved(job);
    job->deleteLater();
}

void JobQueue::cancelAll()
{
    for (auto *u : m_plugins->upscalers()) {
        u->cancel();
    }
    for (auto *e : m_plugins->encoders()) {
        e->cancel();
    }
}

void JobQueue::runNext()
{
    const int maxParallel = qMax(1, KmaxSettings::self()->parallelJobs());
    while (m_running < maxParallel) {
        Job *next = nullptr;
        for (auto *j : std::as_const(m_jobs)) {
            if (j->state() == Job::State::Pending) {
                next = j;
                break;
            }
        }
        if (!next) {
            if (m_running == 0) {
                Q_EMIT queueIdle();
            }
            return;
        }
        ++m_running;
        next->setState(Job::State::Extracting);
        auto *plugins = m_plugins;
        QPointer<Job> guard(next);
        (void)QtConcurrent::run([plugins, guard]() {
            if (!guard) {
                return;
            }
            Pipeline pipeline(plugins);
            pipeline.run(guard.data());
        }).then(this, [this](auto &&) {
            --m_running;
            runNext();
        });
    }
}

} // namespace Kmax
