#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSystemTrayIcon>

#include "servicemanager.h"
#include "networkmanager.h"

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

protected:
     void closeEvent(QCloseEvent *event) override;

private slots:

    void on_startStopButton_clicked();

    void on_serviceStateChanged(ServiceMgr::ServiceState state);

    void on_networkStateChanged(NetworkMgr::NetworkState state);

private:
    Ui::MainWindow *ui;

    ServiceMgr *m_serviceMgr;
    ServiceMgr::ServiceState m_serviceState;
    QString getServiceStateString(const ServiceMgr::ServiceState state);

    NetworkMgr *m_networkMgr;
    NetworkMgr::NetworkState m_networkState;
    QString getNetworkStateString(const NetworkMgr::NetworkState state);

    bool m_startStopFromMainTab;
    void updateMainTab();

    QAction *quitAction;
    QSystemTrayIcon *trayIcon;
    QMenu *trayIconMenu;

};
#endif // MAINWINDOW_H
