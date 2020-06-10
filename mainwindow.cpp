#include <QDebug>
#include <QMessageBox>
#include <QCloseEvent>

#include "mainwindow.h"
#include "ui_mainwindow.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serviceState(ServiceMgr::Unknown)
    , m_networkState(NetworkMgr::Unknown)
    , m_startStopFromMainTab(false)
{
    ui->setupUi(this);

    ui->main_tab->setFocus();

    // TODO - add a 'clear status messages' button to the GUI
    statusMsg("Stubby Manager Started.");
    ui->runningStatus->setText("Checking status...");

    // Discover service state
    m_serviceMgr = ServiceMgr::factory(this);
    if (!m_serviceMgr) {
        qFatal("Could not initialise Service Mgr");
        abort();
    }
    connect(m_serviceMgr, SIGNAL(serviceStateChanged(ServiceMgr::ServiceState)), this, SLOT(on_serviceStateChanged(ServiceMgr::ServiceState)));
    m_serviceMgr->getState();

    // Check system DNS settings
    m_networkMgr = NetworkMgr::factory(this);
    if (!m_networkMgr) {
        qFatal("Could not initialise Service Mgr");
        abort();
    }
    connect(m_networkMgr, SIGNAL(networkStateChanged(NetworkMgr::NetworkState)), this, SLOT(on_networkStateChanged(NetworkMgr::NetworkState)));
    m_networkMgr->getState();

    // Discover network and profile

    // Update connection settings display

    // Set up system tray
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, qApp, &QCoreApplication::quit);
    trayIconMenu = new QMenu(this);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);

    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/images/stubby@245x145.png"));
    trayIcon->show();

}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::closeEvent(QCloseEvent *event) {

#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif

    if (trayIcon->isVisible()) {
// TODO: Restore system tray
// Comment this out whilst in development....
//        QMessageBox::information(this, tr("Systray"),
//                                 tr("Stubby Manager will keep running in the system tray. "
//                                    "**We recommend you keep Stubby Manager running to "
//                                    "properly monitor your system.**"
//                                    "To terminate Stubby Manager, "
//                                    "choose <b>Quit</b> in the context menu "
//                                    "of the system tray entry (this won't stop Stubby itself!)."));
        hide();
        event->ignore();
    }
}

// crude mechanism to output status changes to GUI...
// TODO - have manager class connected to a slot for this and derive other classes from that
void MainWindow::statusMsg(QString statusMsg) {
    ui->statusOutput->moveCursor (QTextCursor::End);
    ui->statusOutput->insertPlainText (statusMsg);
    ui->statusOutput->insertPlainText ("\n");
    ui->statusOutput->moveCursor (QTextCursor::End);
}


/*
 * Slots functions
 */

void MainWindow::on_startStopButton_clicked()
{
    statusMsg("");
    // Currently we handle the service status first and based on the result of that action we later update the system DNS settings
    m_startStopFromMainTab = true;
    if (m_serviceState != ServiceMgr::Running && m_serviceState != ServiceMgr::Starting &&  m_serviceState != ServiceMgr::Stopping) {
        ui->startStopButton->setText("Stubby starting...");
        m_serviceMgr->start();
    }
    else if (m_serviceState == ServiceMgr::Running && m_networkState != NetworkMgr::Localhost) {
        ui->startStopButton->setText("Stubby starting...");
        m_networkMgr->setLocalhost();
    }
    else {
        ui->startStopButton->setText("Stubby stopping...");
        m_serviceMgr->stop();
    }
    ui->startStopButton->setStyleSheet("background-color: rgb(145, 145, 145);");
    // TODO: Disable button until state known?
    // Also - add a timer incase the start processes do not return?
}

void MainWindow::on_serviceStateChanged(ServiceMgr::ServiceState state) {

    qDebug("Stubby Service state changed from %s to %s ", getServiceStateString(m_serviceState).toLatin1().data(), getServiceStateString(state).toLatin1().data());
    m_serviceState = state;
    if (m_startStopFromMainTab) {
        if (m_serviceState == ServiceMgr::Running)
            m_networkMgr->setLocalhost();
        else if (m_serviceState != ServiceMgr::Running && m_serviceState != ServiceMgr::Starting)
            m_networkMgr->unsetLocalhost();
    }
    updateMainTab();
}

void MainWindow::on_networkStateChanged(NetworkMgr::NetworkState state) {
    qDebug("Network DNS state changed from %s to %s ", getNetworkStateString(m_networkState).toLatin1().data(), getNetworkStateString(state).toLatin1().data());
    m_networkState = state;
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

QString MainWindow::getNetworkStateString(const NetworkMgr::NetworkState state)
{
    switch (state) {
        case NetworkMgr::Localhost    : return "Localhost";
        case NetworkMgr::NotLocalhost : return "Not Localhost";
        //case NetworkMgr::Error      : return "Error determining state";
        case NetworkMgr::Unknown :
        default : return "Unknown";
    }
}


void MainWindow::updateMainTab() {
    //TODO: May need to handle more states..
    qDebug ("Updating state with service %s and network %s ", getServiceStateString(m_serviceState).toLatin1().data(), getNetworkStateString(m_networkState).toLatin1().data());
    if (m_serviceState   == ServiceMgr::Running &&
        m_networkState == NetworkMgr::Localhost) {
        ui->runningStatus->setText(getServiceStateString(m_serviceState));
        ui->startStopButton->setText("Stop Stubby");
        ui->startStopButton->setStyleSheet("background-color: rgb(85, 170, 255);");
    }
    else if (m_serviceState   == ServiceMgr::Stopped &&
             m_networkState == NetworkMgr::NotLocalhost) {
        ui->runningStatus->setText(getServiceStateString(m_serviceState));
        ui->startStopButton->setText("Start Stubby");
        ui->startStopButton->setStyleSheet("background-color: rgb(29, 163, 18);");
    }
    else if (m_serviceState   == ServiceMgr::Unknown ||
             m_networkState == NetworkMgr::Unknown) {
        ui->runningStatus->setText(getServiceStateString(ServiceMgr::Unknown));
        ui->startStopButton->setText("Start Stubby");
        ui->startStopButton->setStyleSheet("background-color: rgb(29, 163, 18);");
    }
    else {
        ui->runningStatus->setText("Partially running...");
        ui->startStopButton->setText("Start Stubby");
        ui->startStopButton->setStyleSheet("background-color: rgb(29, 163, 18);");
    }
}
