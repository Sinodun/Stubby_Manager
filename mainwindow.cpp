#include <QDebug>

#include "mainwindow.h"
#include "servicemanager.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serviceState(ServiceMgr::Unknown)
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

void MainWindow::on_start_stop_button_clicked()
{
    // if state is stopped, start service, update button to 'stop'
    // if state is running, stop service, update button to 'start'
}

void MainWindow::on_serviceStateChanged(ServiceMgr::ServiceState state) {

    qInfo("Service state changed from %d to %d ", m_serviceState, state);
    m_serviceState = state;
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
    ui->running_status->setText(getServiceStateString(m_serviceState));
}
