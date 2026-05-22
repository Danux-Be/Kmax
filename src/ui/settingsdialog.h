// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include <KConfigDialog>

namespace Kmax {

class SettingsDialog : public KConfigDialog
{
    Q_OBJECT
public:
    SettingsDialog(QWidget *parent, const QString &name);
};

} // namespace Kmax
