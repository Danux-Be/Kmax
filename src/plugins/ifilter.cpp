// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "ifilter.h"

namespace Kmax {

IFilter::IFilter(QObject *parent)
    : IPlugin(parent)
{
}

IFilter::~IFilter() = default;

} // namespace Kmax
