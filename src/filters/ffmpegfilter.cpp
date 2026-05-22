// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ffmpegfilter.h"

namespace Kmax {

FFmpegFilter::FFmpegFilter(QString id, QString displayName, QString expression, int order, QObject *parent)
    : IFilter(parent)
    , m_id(std::move(id))
    , m_displayName(std::move(displayName))
    , m_expression(std::move(expression))
    , m_order(order)
{
}

} // namespace Kmax
