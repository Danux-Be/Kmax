// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "isource.h"

namespace Kmax {

ISource::ISource(QObject *parent)
    : IPlugin(parent)
{
}

ISource::~ISource() = default;

} // namespace Kmax
