// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "job.h"

namespace Kmax {

Job::Job(QObject *parent)
    : QObject(parent)
{
}

void Job::setState(State s)
{
    if (m_state == s) {
        return;
    }
    m_state = s;
    Q_EMIT stateChanged(s);
}

void Job::setProgress(int percent, const QString &message)
{
    m_progress = percent;
    if (!message.isEmpty()) {
        m_message = message;
    }
    Q_EMIT progressChanged(percent, m_message);
}

void Job::setError(const QString &error)
{
    m_errorString = error;
    setState(State::Failed);
}

} // namespace Kmax
