// SPDX-FileCopyrightText: 2023 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "xdgactivation_p.h"

#include <DGuiApplicationHelper>
#include <QEventLoop>
#include <QGuiApplication>
#include <QLoggingCategory>
#include <QTimer>

#include <private/qwaylanddisplay_p.h>
#include <private/qwaylandinputdevice_p.h>
#include <private/qwaylandwindow_p.h>

Q_LOGGING_CATEGORY(dsXdgActivation, "org.deepin.ds.xdgactivation")

DS_BEGIN_NAMESPACE

// ---------------------------------------------------------------------------
// XdgActivationPrivate
// ---------------------------------------------------------------------------

XdgActivationPrivate::XdgActivationPrivate(XdgActivation *qq)
    : DObjectPrivate(qq)
    , QWaylandClientExtensionTemplate<XdgActivationPrivate>(1)
{
    initialize();
}

XdgActivationPrivate::~XdgActivationPrivate()
{
    if (isInitialized())
        destroy();
}

XdgActivationTokenV1 *XdgActivationPrivate::createTokenProvider(QWindow *window, const QString &appId)
{
    auto *provider = new XdgActivationTokenV1;
    provider->init(get_activation_token());

    if (window) {
        if (auto *waylandWindow = dynamic_cast<QtWaylandClient::QWaylandWindow *>(window->handle())) {
            if (auto *surface = waylandWindow->wlSurface()) {
                provider->set_surface(surface);
            }
            if (auto *inputDevice = waylandWindow->display()->lastInputDevice()) {
                provider->set_serial(inputDevice->serial(), inputDevice->wl_seat());
            }
        }
    }

    if (!appId.isEmpty())
        provider->set_app_id(appId);

    provider->commit();
    return provider;
}

// ---------------------------------------------------------------------------
// XdgActivation
// ---------------------------------------------------------------------------

Q_GLOBAL_STATIC(XdgActivation, s_xdgActivation)

XdgActivation *XdgActivation::instance()
{
    return s_xdgActivation;
}

XdgActivation::XdgActivation(QObject *parent)
    : QObject(parent)
    , DObject(*new XdgActivationPrivate(this))
{
}

XdgActivation::~XdgActivation() = default;

bool XdgActivation::isActive() const
{
    if (!Dtk::Gui::DGuiApplicationHelper::testAttribute(Dtk::Gui::DGuiApplicationHelper::IsWaylandPlatform)) {
        qCDebug(dsXdgActivation) << "not running on Wayland, isActive returns false";
        return false;
    }
    D_DC(XdgActivation);
    bool active = d->isActive();
    qCDebug(dsXdgActivation) << "isActive:" << active;
    return active;
}

QString XdgActivation::requestToken(QWindow *window, const QString &appId)
{
    D_D(XdgActivation);

    if (!d->isActive()) {
        qCWarning(dsXdgActivation) << "xdg_activation_v1 is not active; token request skipped";
        return {};
    }

    auto *provider = d->createTokenProvider(window, appId);

    QString token;
    QEventLoop loop;
    QTimer timeout;
    timeout.setSingleShot(true);
    timeout.setInterval(2000);

    connect(provider, &XdgActivationTokenV1::done, &loop, [&token, &loop](const QString &t) {
        token = t;
        loop.quit();
    });
    connect(&timeout, &QTimer::timeout, &loop, &QEventLoop::quit);

    timeout.start();
    loop.exec();

    if (token.isEmpty())
        qCWarning(dsXdgActivation) << "XDG activation token request timed out for app:" << appId;
    else
        qCDebug(dsXdgActivation) << "Received XDG activation token for app:" << appId;

    provider->deleteLater();
    return token;
}

void XdgActivation::requestTokenAsync(QWindow *window, const QString &appId, std::function<void(const QString &)> callback)
{
    D_D(XdgActivation);

    if (!d->isActive()) {
        qCWarning(dsXdgActivation) << "xdg_activation_v1 is not active; async token request skipped";
        callback({});
        return;
    }

    auto *provider = d->createTokenProvider(window, appId);

    connect(provider, &XdgActivationTokenV1::done, this, [callback, provider, appId](const QString &token) {
        if (token.isEmpty())
            qCWarning(dsXdgActivation) << "XDG activation token missing for app:" << appId;
        else
            qCDebug(dsXdgActivation) << "Async XDG activation token received for app:" << appId;
        callback(token);
        provider->deleteLater();
    });
}

DS_END_NAMESPACE

#include "xdgactivation.moc"
