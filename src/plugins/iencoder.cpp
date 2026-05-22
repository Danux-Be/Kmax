// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "iencoder.h"

namespace Kmax {

IEncoder::IEncoder(QObject *parent)
    : IPlugin(parent)
{
}

IEncoder::~IEncoder() = default;

} // namespace Kmax
