// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#include "iplugin.h"

namespace Kmax {

IPlugin::IPlugin(QObject *parent)
    : QObject(parent)
{
}

IPlugin::~IPlugin() = default;

} // namespace Kmax
