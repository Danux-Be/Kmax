// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "iupscaler.h"

namespace Kmax {

IUpscaler::IUpscaler(QObject *parent)
    : IPlugin(parent)
{
}

IUpscaler::~IUpscaler() = default;

} // namespace Kmax
