// SPDX-FileCopyrightText: 2026 Dany Petit <danypetit.be@gmail.com>
// SPDX-License-Identifier: GPL-3.0-or-later
#pragma once

#include "iplugin.h"

#include <QStringList>

namespace Kmax {

/**
 * A Filter operates on the video stream during the pipeline.
 *
 * v0.1.0 filters declare an FFmpeg filter chain via ffmpegExpression(); the
 * pipeline composes them into a single FFmpeg invocation. A future plugin
 * API revision will expose a per-frame Qt/Vulkan path for filters that need
 * GPU access or non-trivial logic.
 */
class IFilter : public IPlugin
{
    Q_OBJECT
public:
    explicit IFilter(QObject *parent = nullptr);
    ~IFilter() override;

    virtual QString id() const = 0;
    virtual QString displayName() const = 0;

    /** FFmpeg `-vf` expression contribution (e.g. "hqdn3d=2:1:2:1"). */
    virtual QString ffmpegExpression() const = 0;

    /** Order hint within the chain. Lower runs first. */
    virtual int order() const { return 100; }

    /** Whether the filter is currently enabled. */
    virtual bool isEnabled() const { return m_enabled; }
    virtual void setEnabled(bool enabled) { m_enabled = enabled; }

private:
    bool m_enabled = true;
};

} // namespace Kmax

Q_DECLARE_INTERFACE(Kmax::IFilter, "org.kde.kmax.IFilter/1.0")
