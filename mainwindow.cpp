#include <QDebug>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#ifdef Q_OS_MACOS
#include "servicemanager_macos.h"
#include "systemdnsmanager_macos.h"
#endif


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serviceState(ServiceMgr::Unknown)
    , m_systemDNSState(SystemDNSMgr::Unknown)
    , m_startStopFromMainTab(true)
{
    ui->setupUi(this);

    ui->main_tab->setFocus();

    // Discover service state
    m_serviceMgr = new ServiceMgr(this);
    if (!m_serviceMgr) {
        qFatal("Could not initialise Service Mgr");
        abort();
    }
    connect(m_serviceMgr, SIGNAL(serviceStateChanged(ServiceMgr::ServiceState)), this, SLOT(on_serviceStateChanged(ServiceMgr::ServiceState)));
    m_serviceMgr->getState();

    // Check system DNS settings
    m_systemDNSMgr = new SystemDNSMgr(this);
    if (!m_systemDNSMgr) {
        qFatal("Could not initialise Service Mgr");
        abort();
    }
    connect(m_systemDNSMgr, SIGNAL(systemDNSStateChanged(SystemDNSMgr::SystemDNSState)), this, SLOT(on_systemDNSStateChanged(SystemDNSMgr::SystemDNSState)));
    m_systemDNSMgr->getState();

    // Discover network and profile

    // Update connection settings

}

MainWindow::~MainWindow()
{
    delete ui;
}


/*
 * Slots functions
 */

void MainWindow::on_startStopButton_clicked()
{
    // Currently we handle the service status first and based on the result of that action we later update the system DNS settings
    m_startStopFromMainTab = true;
    if (m_serviceState != ServiceMgr::Running && m_serviceState != ServiceMgr::Starting &&  m_serviceState != ServiceMgr::Stopping) {
        ui->startStopButton->setText("Stop Stubby");
        ui->startStopButton->setStyleSheet("background-color: rgb(85, 170, 255);");
        m_serviceMgr->start();
    }
    else {
        ui->startStopButton->setText("Start Stubby");
        ui->startStopButton->setStyleSheet("background-color: rgb(33, 208, 109, 222);");
        m_serviceMgr->stop();
    }
}

void MainWindow::on_serviceStateChanged(ServiceMgr::ServiceState state) {

    qDebug("MAIN WINDOW: Service state changed from %s to %s ", getServiceStateString(m_serviceState).toLatin1().data(), getServiceStateString(state).toLatin1().data());
    m_serviceState = state;
    if (m_startStopFromMainTab) {
        if (m_serviceState == ServiceMgr::Running)
            m_systemDNSMgr->setLocalhost();
        else if (m_serviceState != ServiceMgr::Running && m_serviceState != ServiceMgr::Starting)
            m_systemDNSMgr->unsetLocalhost();
    }
    updateMainTab();
}

void MainWindow::on_systemDNSStateChanged(SystemDNSMgr::SystemDNSState state) {
    qDebug("MAIN WINDOW: System DNS state changed from %d to %d ", m_systemDNSState, state);
    m_systemDNSState = state;
    updateMainTab();
}

/*
 * Private functions
 */

QString MainWindow::getServiceStateString(const ServiceMgr::ServiceState state)
{
    switch (state) {
        case ServiceMgr::Stopped    : return "Not running";
        case ServiceMgr::Starting   : return "Starting";
        case ServiceMgr::Stopping   : return "Stopping";
        case ServiceMgr::Running    : return "Running";
        case ServiceMgr::Error      : return "Error determining state";
        case ServiceMgr::Unknown :
        default : return "Unknown";
    }
}

void MainWindow::updateMainTab() {
    //TODO: Handle all the permutations of the states correctly....
    if (m_serviceState == ServiceMgr::Running && m_systemDNSState == SystemDNSMgr::Localhost)
        ui->runningStatus->setText(getServiceStateString(m_serviceState));
    else if (m_serviceState == ServiceMgr::Stopped && m_systemDNSState == SystemDNSMgr::NotLocalhost)
        ui->runningStatus->setText(getServiceStateString(m_serviceState));
    else
        ui->runningStatus->setText(getServiceStateString(ServiceMgr::Unknown));
}
