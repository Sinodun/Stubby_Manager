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
#include "networkprofilewidget.h"
#include "networkswidget.h"
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
    void systrayMsg(QString status_msg);
    void systrayAlert(QString status_msg);
    void logMsg(QString log_msg);
    enum UpdateState{Init, Start, Stop, Restart, Probe, None};
    int getUpdateState() const {return updateState;};
    bool isServiceRunning() const;
    void refreshNetworks(std::map<std::string, NetworkMgr::interfaceInfo> running_networks);
    void alertOnNetworksUpdatedRestart();
    void alertOnNewNetwork(std::string network, Config::NetworkProfile profile);

protected:
    void closeEvent(QCloseEvent *event) override;


private slots:

    void on_hideDetailsCheckBox_toggled();
    void on_onOffSlider_stateChanged();
    void on_restartButton_clicked();
    void on_probeButton_clicked();
    void on_testButton_clicked();
    void on_testQueryResult(bool result);
    void on_showLogButton_toggled();
    void on_helpButton_clicked();
    void on_applyAllButton_clicked();
    void on_discardAllButton_clicked();
    void on_revertAllButton_clicked();

    void on_serviceStateChanged(ServiceMgr::ServiceState state);
    void on_DNSStateChanged(NetworkMgr::NetworkState state);

    void on_userProfileEditInProgress();
    void on_userNetworksEditInProgress();
    void on_SavedConfigChanged(bool restart);

    void closeFromSystray();
    void iconActivated(QSystemTrayIcon::ActivationReason reason);

private:
    void setMainButtonStates();
    void setTopPanelStatus();
    void setTopPanelNetworkInfo();
    void firstRunPopUp();
    void setToolTips();

    void timerExpired();
    void probeTimerExpired();
    void handleError();
    void handleCancel();
    int  handleUnsavedChanges();


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

    NetworksWidget *m_networksWidget;

    ILogMgr *m_logMgr;

    UpdateState updateState;

    QTimer *timer;
    QTimer *probeTimer;

    QAction *quitAction;
    QAction *openAction;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;
    QSettings *stubbySettings;

    QPixmap *greenPixmap;
    QPixmap *yellowPixmap;
    QPixmap *redPixmap;
    QPixmap *greyPixmap;

};
#endif // MAINWINDOW_H
