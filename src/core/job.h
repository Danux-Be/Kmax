// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <QObject>
#include <QString>
#include <QStringList>

#include "../plugins/isource.h"

namespace Kmax {

class Job : public QObject
{
    Q_OBJECT
public:
    enum class State {
        Pending,
        Extracting,
        Upscaling,
        Encoding,
        Finished,
        Failed,
        Cancelled,
    };
    Q_ENUM(State)

    explicit Job(QObject *parent = nullptr);

    SourceItem source;
    QString sourcePluginId;
    QStringList filterIds;
    QString upscalerId;
    QString upscaleModel;
    int upscaleScale = 4;
    int tileSize = 0;
    int gpuDeviceIndex = -1;
    QString encoderId;
    QString outputCodec;
    int crf = 20;
    QString outputFile;

    State state() const { return m_state; }
    int progress() const { return m_progress; }
    QString message() const { return m_message; }
    QString errorString() const { return m_errorString; }

    void setState(State s);
    void setProgress(int percent, const QString &message = {});
    void setError(const QString &error);

Q_SIGNALS:
    void stateChanged(Kmax::Job::State state);
    void progressChanged(int percent, const QString &message);

private:
    State m_state = State::Pending;
    int m_progress = 0;
    QString m_message;
    QString m_errorString;
};

} // namespace Kmax
