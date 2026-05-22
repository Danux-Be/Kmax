// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "../plugins/ifilter.h"

namespace Kmax {

/**
 * A built-in filter whose contribution to the FFmpeg `-vf` chain is fixed
 * at construction time. Used to surface the three default filters
 * (deinterlace, denoise, color correction) without needing three classes.
 */
class FFmpegFilter : public IFilter
{
    Q_OBJECT
    Q_INTERFACES(Kmax::IFilter)
public:
    FFmpegFilter(QString id, QString displayName, QString expression, int order,
                 QObject *parent = nullptr);

    QString id() const override { return m_id; }
    QString displayName() const override { return m_displayName; }
    QString ffmpegExpression() const override { return m_expression; }
    int order() const override { return m_order; }

private:
    QString m_id;
    QString m_displayName;
    QString m_expression;
    int m_order;
};

} // namespace Kmax
