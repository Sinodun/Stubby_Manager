#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

#ifdef Q_OS_MACOS
#include "servicemanager_macos.h"
#endif

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:

    void on_startStopButton_clicked();

    void on_serviceStateChanged(ServiceMgr::ServiceState state);

private:
    Ui::MainWindow *ui;

    ServiceMgr *m_serviceMgr;
    ServiceMgr::ServiceState m_serviceState;

    QString getServiceStateString(const ServiceMgr::ServiceState state);

    void updateMainTab();

};
#endif // MAINWINDOW_H
