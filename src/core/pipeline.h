// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>

namespace Kmax {

class Job;
class PluginManager;

/**
 * Runs a single Job end-to-end on the calling thread. Call from a worker
 * thread (QtConcurrent::run) — the steps block (QProcess::waitForFinished).
 */
class Pipeline : public QObject
{
    Q_OBJECT
public:
    Pipeline(PluginManager *plugins, QObject *parent = nullptr);

    /** Run the job synchronously. Returns true on success. */
    bool run(Job *job);

    void cancel();

private:
    bool extractFrames(Job *job, const QString &inputFile,
                       const QString &outDir, const QString &filterChain);
    QString buildFilterChain(Job *job) const;
    static QString frameDirForJob(const QString &workingDir, const QString &jobId, const char *stage);

    PluginManager *m_plugins;
    bool m_cancelled = false;
};

} // namespace Kmax
