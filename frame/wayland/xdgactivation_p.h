// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "xdgactivation.h"

#include "qwayland-xdg-activation-v1.h"
#include <dobject_p.h>

#include <QtWaylandClient/QWaylandClientExtension>

DS_BEGIN_NAMESPACE

class XdgActivationTokenV1 : public QObject, public QtWayland::xdg_activation_token_v1
{
    Q_OBJECT
public:
    ~XdgActivationTokenV1() override
    {
        destroy();
    }

Q_SIGNALS:
    void done(const QString &token);

protected:
    void xdg_activation_token_v1_done(const QString &token) override
    {
        Q_EMIT done(token);
    }
};

class XdgActivationPrivate : public DTK_CORE_NAMESPACE::DObjectPrivate,
                             public QWaylandClientExtensionTemplate<XdgActivationPrivate>,
                             public QtWayland::xdg_activation_v1
{
public:
    explicit XdgActivationPrivate(XdgActivation *qq);
    ~XdgActivationPrivate() override;

    XdgActivationTokenV1 *createTokenProvider(QWindow *window, const QString &appId);

    D_DECLARE_PUBLIC(XdgActivation)
};

DS_END_NAMESPACE
