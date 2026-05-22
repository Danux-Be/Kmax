// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QWidget>

class QLabel;
class QProgressBar;
class QToolButton;

namespace Kmax {

class Job;

class JobWidget : public QWidget
{
    Q_OBJECT
public:
    explicit JobWidget(Job *job, QWidget *parent = nullptr);

    Job *job() const { return m_job; }

Q_SIGNALS:
    void removeRequested(Kmax::Job *job);

private:
    void refresh();
    Job *m_job;
    QLabel *m_titleLabel;
    QLabel *m_statusLabel;
    QProgressBar *m_progress;
    QToolButton *m_removeButton;
};

} // namespace Kmax
