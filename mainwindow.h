/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <unordered_map>

#include <QMainWindow>
#include <QSystemTrayIcon>
#include <QSettings>


#include "configmanager.h"
#include "servicemanager.h"
#include "networkmanager.h"
#include "networkprofile.h"
#include "networks.h"
#include "logmanager.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void statusMsg(QString status_msg);
    void logMsg(QString log_msg);
    enum UpdateState{Init, Start, Stop, Restart, Probe, None};
    int getUpdateState() const {return updateState;};
    Config::NetworkProfile getCurrentNetworkProfile() const {return m_currentNetworkProfile;};

protected:
     void closeEvent(QCloseEvent *event) override;

private slots:

    void on_hideDetailsCheckBox_toggled();

    void on_onOffSlider_stateChanged();

    void on_restartButton_clicked();

    void on_probeButton_clicked();

    void on_testButton_clicked();

    void on_serviceStateChanged(ServiceMgr::ServiceState state);

    void on_networkStateChanged(NetworkMgr::NetworkState state);

    void on_testQueryResult(bool result);

    void on_showLogButton_toggled();

    void on_networkProfileStateUpdated(Config::NetworkProfile np, bool unsaved, bool notdefault);
    void on_networksStateUpdated(bool unsaved);
    void on_applyAllButton_clicked();
    void on_discardAllButton_clicked();
    void on_revertAllButton_clicked();
    void on_networkConfigChanged();

private:
    void setButtonStates();
    void updateCurrentNetworkInfo();
    void firstRunPopUp();

    Ui::MainWindow *ui;

    ConfigMgr *m_configMgr;
    ServiceMgr *m_serviceMgr;
    ServiceMgr::ServiceState m_serviceState;
    QString getServiceStateString(const ServiceMgr::ServiceState state);

    NetworkMgr *m_networkMgr;
    NetworkMgr::NetworkState m_networkState;
    QString getNetworkStateString(const NetworkMgr::NetworkState state);

    NetworkProfileWidget *m_untrustedNetworkWidget;
    NetworkProfileWidget *m_trustedNetworkWidget;
    NetworkProfileWidget *m_hostileNetworkWidget;

    NetworkListWidget *m_networkListWidget;

    ILogMgr *m_logMgr;

    Config::NetworkProfile m_currentNetworkProfile;

    UpdateState updateState;
    void updateMainTab();
    QTimer *timer;
    void timerExpired();
    QTimer *probeTimer;
    void probeTimerExpired();
    void handleError();
    void handleCancel();
    int handleUnsavedChanges();

    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QSettings *stubbySettings;

    QPixmap *greenPixmap;
    QPixmap *yellowPixmap;
    QPixmap *redPixmap;
    QPixmap *greyPixmap;

};
#endif // MAINWINDOW_H
