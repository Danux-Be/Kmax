// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "jobwidget.h"

#include "../core/job.h"

#include <KLocalizedString>

#include <QHBoxLayout>
#include <QLabel>
#include <QProgressBar>
#include <QToolButton>
#include <QVBoxLayout>

namespace Kmax {

JobWidget::JobWidget(Job *job, QWidget *parent)
    : QWidget(parent)
    , m_job(job)
{
    m_titleLabel = new QLabel(job->source.title, this);
    QFont boldFont = m_titleLabel->font();
    boldFont.setBold(true);
    m_titleLabel->setFont(boldFont);

    m_statusLabel = new QLabel(i18n("Pending"), this);
    m_statusLabel->setForegroundRole(QPalette::PlaceholderText);

    m_progress = new QProgressBar(this);
    m_progress->setRange(0, 100);
    m_progress->setValue(0);

    m_removeButton = new QToolButton(this);
    m_removeButton->setIcon(QIcon::fromTheme(QStringLiteral("edit-delete")));
    m_removeButton->setToolTip(i18n("Remove from queue"));
    m_removeButton->setAutoRaise(true);

    auto *vbox = new QVBoxLayout;
    auto *top = new QHBoxLayout;
    top->addWidget(m_titleLabel, 1);
    top->addWidget(m_removeButton);
    vbox->addLayout(top);
    vbox->addWidget(m_progress);
    vbox->addWidget(m_statusLabel);

    auto *outer = new QHBoxLayout(this);
    outer->addLayout(vbox);

    connect(m_removeButton, &QToolButton::clicked, this, [this]() {
        Q_EMIT removeRequested(m_job);
    });
    connect(m_job, &Job::progressChanged, this, &JobWidget::refresh);
    connect(m_job, &Job::stateChanged, this, &JobWidget::refresh);
    refresh();
}

void JobWidget::refresh()
{
    m_progress->setValue(m_job->progress());
    QString status;
    switch (m_job->state()) {
    case Job::State::Pending:    status = i18n("Pending"); break;
    case Job::State::Extracting: status = i18n("Extracting frames…"); break;
    case Job::State::Upscaling:  status = i18n("Upscaling…"); break;
    case Job::State::Encoding:   status = i18n("Encoding…"); break;
    case Job::State::Finished:   status = i18n("Done — %1", m_job->outputFile); break;
    case Job::State::Failed:     status = i18n("Failed: %1", m_job->errorString()); break;
    case Job::State::Cancelled:  status = i18n("Cancelled"); break;
    }
    if (!m_job->message().isEmpty() && m_job->state() != Job::State::Finished
        && m_job->state() != Job::State::Failed) {
        status += QStringLiteral(" — ") + m_job->message();
    }
    m_statusLabel->setText(status);
}

} // namespace Kmax
