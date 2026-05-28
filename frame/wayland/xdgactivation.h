// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include "dsglobal.h"

#include <DObject>
#include <QObject>
#include <QWindow>

#include <functional>

DS_BEGIN_NAMESPACE

class XdgActivationPrivate;
class DS_SHARE XdgActivation : public QObject, public DTK_CORE_NAMESPACE::DObject
{
    Q_OBJECT
    D_DECLARE_PRIVATE(XdgActivation)
public:
    static XdgActivation *instance();

    ~XdgActivation() override;

    bool isActive() const;

    QString requestToken(QWindow *window, const QString &appId = {});
    void requestTokenAsync(QWindow *window, const QString &appId, std::function<void(const QString &)> callback);

    explicit XdgActivation(QObject *parent = nullptr);
};

DS_END_NAMESPACE
