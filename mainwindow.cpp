/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <QDebug>
#include <QMessageBox>
#include <QCloseEvent>
#include <QPixmap>
#include <QPainter>
#include <QTimer>
#include <QSettings>
#include <QDesktopServices>
#include <QDateTime>

#include "mainwindow.h"
#include "ui_mainwindow.h"

// This is for the colored balls used to display status
class CirclePixmap: public QPixmap {
public:
    CirclePixmap(QColor col);
};

CirclePixmap::CirclePixmap(QColor col)
    :QPixmap(15,15) {

    fill(QColor(255, 0, 0, 0));
    QPainter p(this);
    p.setRenderHint(QPainter::Antialiasing, true);
    QBrush brush(col);
    p.setBrush(brush);
    p.drawEllipse(0, 0, 15, 15);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_serviceState(ServiceMgr::Unknown)
    , m_networkState(NetworkMgr::Unknown)
    , updateState(Init)
    , m_configMgr(), m_serviceMgr(), m_networkMgr()
    , m_untrustedNetworkWidget(), m_trustedNetworkWidget()
    , m_hostileNetworkWidget(), m_networksWidget(), m_logMgr()
    , timer(), probeTimer(), quitAction(), trayIcon()
    , trayIconMenu(), greenPixmap(), yellowPixmap()
    , redPixmap(), greyPixmap()
{
    ui->setupUi(this);

//#ifdef Q_OS_MAC
//    QFont f = ui->tabWidget->font();
//    f.setPointSize(14);
//    ui->tabWidget->setFont(f);
//    ui->statusOutput->setFont(f);
//#endif

    // For now, make the Revert all button disapear
    ui->revertAllButton->setVisible(false);

    // TODO - add a 'clear status messages' button to the GUI
    statusMsg("Stubby Manager Started.");
    ui->runningStatus->setText("Checking status...");

    // Set up system tray
    quitAction = new QAction(tr("&Quit"), this);
    connect(quitAction, &QAction::triggered, this, &MainWindow::closeFromSystray);
    openAction = new QAction(tr("&Open"), this);
    connect(openAction, SIGNAL(triggered()), this, SLOT(show()));
    trayIconMenu = new QMenu(this);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(openAction);
    trayIconMenu->addAction(quitAction);
    trayIcon = new QSystemTrayIcon(this);
    trayIcon->setContextMenu(trayIconMenu);
    trayIcon->setIcon(QIcon(":/images/stubby@245x145.png"));
    trayIcon->show();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
             this, SLOT(iconActivated(QSystemTrayIcon::ActivationReason)));

    // Set up circle icons
    greenPixmap = new CirclePixmap(Qt::green);
    yellowPixmap = new CirclePixmap(Qt::yellow);
    redPixmap = new CirclePixmap(Qt::red);
    greyPixmap = new CirclePixmap(Qt::lightGray);
    ui->serviceStatus->setPixmap(*greyPixmap);
    ui->networkStatus->setPixmap(*greyPixmap);
    ui->connectStatus->setPixmap(*greyPixmap);
    ui->stubbyStatus->setPixmap(*greyPixmap);

    // Set up config
    m_configMgr = ConfigMgr::factory(this);
    if (!m_configMgr) {
        qFatal("Could not initialise Config Mgr");
        abort();
    }
    m_configMgr->init();
    m_configMgr->load();

    m_untrustedNetworkWidget = new NetworkProfileWidget(*m_configMgr, Config::NetworkProfile::untrusted);
    m_trustedNetworkWidget = new NetworkProfileWidget(*m_configMgr, Config::NetworkProfile::trusted);
    m_hostileNetworkWidget = new NetworkProfileWidget(*m_configMgr, Config::NetworkProfile::hostile);

    ui->networkProfileConfig->clear();
    ui->networkProfileConfig->addTab(m_untrustedNetworkWidget, QString::fromUtf8("Untrusted"));
    ui->networkProfileConfig->addTab(m_trustedNetworkWidget, QString::fromUtf8("Trusted"));
    ui->networkProfileConfig->addTab(m_hostileNetworkWidget, QString::fromUtf8("Hostile"));

    connect(m_untrustedNetworkWidget, &NetworkProfileWidget::userProfileEditInProgress,
            this, &MainWindow::on_userProfileEditInProgress);
    connect(m_trustedNetworkWidget, &NetworkProfileWidget::userProfileEditInProgress,
            this, &MainWindow::on_userProfileEditInProgress);
    connect(m_hostileNetworkWidget, &NetworkProfileWidget::userProfileEditInProgress,
            this, &MainWindow::on_userProfileEditInProgress);

    connect(m_configMgr, &ConfigMgr::configChanged,
            this, &MainWindow::on_SavedConfigChanged);

    // Set up service state
    m_serviceMgr = ServiceMgr::factory(this);
    if (!m_serviceMgr) {
        qFatal("Could not initialise Service Mgr");
        abort();
    }
    connect(m_serviceMgr, SIGNAL(serviceStateChanged(ServiceMgr::ServiceState)), this, SLOT(on_serviceStateChanged(ServiceMgr::ServiceState)));

    // Set up network manager
    m_networkMgr = NetworkMgr::factory(this);
    if (!m_networkMgr) {
        qFatal("Could not initialise Service Mgr");
        abort();
    }
    connect(m_networkMgr, SIGNAL(DNSStateChanged(NetworkMgr::NetworkState)), this, SLOT(on_DNSStateChanged(NetworkMgr::NetworkState)));
    connect(m_networkMgr, SIGNAL(testQueryResult(bool)), this, SLOT(on_testQueryResult(bool)));

    // Set up networks tab.
    m_networksWidget = new NetworksWidget(*m_configMgr, this);
    ui->mainTabWidget->removeTab(2);
    ui->mainTabWidget->insertTab(2, m_networksWidget, "Networks");

    connect(m_networksWidget, &NetworksWidget::userNetworksEditInProgress,
            this, &MainWindow::on_userNetworksEditInProgress);

    // Create a log manager
    m_logMgr = ILogMgr::factory(this);
    if (!m_networkMgr) {
        qFatal("Could not initialise Service Mgr");
        abort();
    }

    // Set initially displayed tab.
    ui->mainTabWidget->setCurrentIndex(0);
    ui->statusTab->setFocus();

    m_serviceMgr->getState();
    m_networkMgr->getDNSState(true);
    m_networksWidget->setNWGuiState();
    m_untrustedNetworkWidget->setNPWGuiState();
    m_trustedNetworkWidget->setNPWGuiState();
    m_hostileNetworkWidget->setNPWGuiState();
    setTopPanelNetworkInfo();

    // Create some timers
    timer = new QTimer(this);
    timer->setSingleShot(true);
    connect(timer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::timerExpired));
    probeTimer = new QTimer(this);
    connect(probeTimer, &QTimer::timeout, this, QOverload<>::of(&MainWindow::probeTimerExpired));
    probeTimer->start(300000);

    // Create the settings
    stubbySettings = new QSettings("Sinodun.com", "Stubby Manager");
    QVariant details = stubbySettings->value("info/hideDetails");
    if (!details.isNull())
        ui->hideDetailsCheckBox->setChecked(details.toBool());
    updateState = None;

}

MainWindow::~MainWindow()
{
    delete ui;
    delete m_configMgr;
    delete m_networkMgr;
    delete m_serviceMgr;
    delete m_untrustedNetworkWidget;
    delete m_trustedNetworkWidget;
    delete m_hostileNetworkWidget;
    delete m_networksWidget;
    delete timer;
    delete probeTimer;
    delete quitAction;
    delete trayIcon;
    delete trayIconMenu;
    delete greenPixmap;
    delete yellowPixmap;
    delete redPixmap;
    delete greyPixmap;
}

void MainWindow::iconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason){
        case QSystemTrayIcon::Trigger:
            if(!this->isVisible()){
                this->show();
            } else {
                this->hide();
            }
            break;
        default:
            break;
    }
}

void MainWindow::timerExpired() {
    if (updateState == None)
        return;
    statusMsg("Stubby timed out trying to complete an action");
    setTopPanelStatus();
    updateState = None;
}

void MainWindow::probeTimerExpired() {
    if (updateState != None)
        return;
    statusMsg("\nProbing State");
    updateState = Probe;
    m_serviceMgr->getState();
}

void MainWindow::closeFromSystray() {
    if (handleUnsavedChanges() == 1 )
        return;
    qApp->quit();
    return;
}

void MainWindow::closeEvent(QCloseEvent *event) {

#ifdef Q_OS_OSX
    if (!event->spontaneous() || !isVisible()) {
        return;
    }
#endif

    if (handleUnsavedChanges() == 1) {
        event->ignore();
        return;
    }

    if (trayIcon->isVisible()) {
       QVariant exitMessage = stubbySettings->value("app/exitMessage");
       if (!exitMessage.isNull() && exitMessage.toBool() == false) {
           hide();
           event->ignore();
           return;
       }
       QMessageBox msgBox;
       setMinimumSize(600,200);
       msgBox.setText("Stubby Manager will keep running in the system tray.");
       msgBox.setInformativeText("*We recommend you keep Stubby Manager running to actively monitor Stubby*<br><br>"
                                 "To terminate Stubby Manager, choose 'Quit' in the context menu "
                                 "of the system tray entry (this won't stop Stubby itself!).<br><br>"
                                 "Hit 'Ok' if you don't want to see this message everytime the window is closed,"
                                 "otherwise hit 'Close'.");
       msgBox.setStandardButtons(QMessageBox::Ok | QMessageBox::Close);
       msgBox.setDefaultButton(QMessageBox::Ok);
       int ret = msgBox.exec();
       switch (ret) {
         case QMessageBox::Ok:
            stubbySettings->setValue("app/exitMessage", false);
            break;
         case QMessageBox::Close:
            break;
         default:
            // should never be reached
            break;
      }
       hide();
       event->ignore();
    }
}

void MainWindow::statusMsg(QString statusMsg) {
    ui->statusOutput->moveCursor (QTextCursor::End);
    ui->statusOutput->insertPlainText (QDateTime::currentDateTime().toString());
    ui->statusOutput->insertPlainText (":  ");
    ui->statusOutput->insertPlainText (statusMsg);
    ui->statusOutput->insertPlainText ("\n");
    ui->statusOutput->moveCursor (QTextCursor::End);
}

void MainWindow::systrayMsg(QString status_msg) {
    trayIcon->showMessage("Stubby message",
    status_msg, QSystemTrayIcon::Information, 60*1000);
}

void MainWindow::logMsg(QString logMsg) {
    ui->logOutput->moveCursor (QTextCursor::End);
    ui->logOutput->insertPlainText (logMsg);
    ui->logOutput->insertPlainText ("\n");
    ui->logOutput->moveCursor (QTextCursor::End);
}

void MainWindow::firstRunPopUp()
{
    // On the very first run pop up a useful message
    QVariant firstRun = stubbySettings->value("app/firstRun");
    if (!firstRun.isNull())
        return;
    QMessageBox::information(this, "Systray",
                             "Stubby Manager runs as a system tray application.<br> "
                             "We recommend that you make the Stubby Manager icon visible "
                             "in your system tray so you can easily see the state of "
                             "Stubby Manager (select the Start menu and type 'select "
                             "which icons appear on the taskbar').<br>");
    stubbySettings->setValue("app/firstRun", false);
}

/*
 * Slots functions
 */

void MainWindow::on_onOffSlider_stateChanged()
{
    qInfo("Slider toggled");
    if (updateState != None)
        return;
    statusMsg("");
    bool value = ui->onOffSlider->isChecked();
    if (value == true) {
        firstRunPopUp();
        // Currently we handle the service status first and based on the result of that action we later update the system DNS settings
        updateState = Start;
        ui->runningStatus->setText("Stubby starting...");
        if (m_serviceState != ServiceMgr::Running && m_serviceState != ServiceMgr::Starting) {
            if (handleUnsavedChanges() == 1) {
                handleCancel();
                return;
            }
            if (m_serviceMgr->start(*m_configMgr))
               handleError();
        }
        else if (m_networkState != NetworkMgr::Localhost) {
            if (m_networkMgr->setLocalhost())
                handleError();
        }
        else {
            // Nothing to do.... possilby recovering from error?
            setTopPanelStatus();
            updateState = None;
        }
    }
    else {
        // Currently we handle network status first and based on the result of that action we later update the service
        updateState = Stop;
        ui->connectStatus->setPixmap(*greyPixmap);
        ui->runningStatus->setText("Stubby stopping...");
        if (m_networkState != NetworkMgr::NotLocalhost) {
             if (m_networkMgr->unsetLocalhost())
                 handleError();
        }
        else if (m_serviceState != ServiceMgr::Stopped && m_serviceState != ServiceMgr::Stopping) {
            if (m_serviceMgr->stop())
                handleError();
        }
        else {
            // Nothing to do.... possilby recovering from error?
            setTopPanelStatus();
            updateState = None;
        }
    }
    timer->start(20000);
}

void MainWindow::on_restartButton_clicked() {

    if (handleUnsavedChanges() == 1) {
        handleCancel();
        return;
    }
    statusMsg("");
    // Currently we handle the service status first and based on the result of that action we later update the system DNS settings
    updateState = Restart;
    ui->connectStatus->setPixmap(*greyPixmap);
    ui->runningStatus->setText("Stubby restarting...");
    if (m_serviceMgr->restart())
        handleError();
    timer->start(20000);
}

void MainWindow::on_probeButton_clicked() {
    statusMsg("\nProbing State");
    updateState = Probe;
    m_serviceMgr->getState();
}

void MainWindow::on_testButton_clicked() {
    if (!(m_serviceState == ServiceMgr::Running && m_networkState == NetworkMgr::Localhost)) {
        statusMsg("Stubby not running - no connection test performed");
        return;
    }
    statusMsg("Testing connection");
    ui->connectStatus->setPixmap(*yellowPixmap);
    m_networkMgr->testQuery();
}

void MainWindow::on_testQueryResult(bool result) {
    if (result) {
        statusMsg("Connection test was a success");
        ui->connectStatus->setPixmap(*greenPixmap);
    }
    else {
        statusMsg("Connection test failed");
        ui->connectStatus->setPixmap(*redPixmap);
        trayIcon->showMessage("Connection Test Failed",
        "There was a problem with a test connection to the active server. Please check your settings.", QSystemTrayIcon::Critical, 60*1000);
    }
}

void MainWindow::alertOnNewNetwork(std::string network, Config::NetworkProfile profile) {
    if (updateState == Init)
        return;
    QString message = "A new network was joined which will use the default ";
    message.append(Config::networkProfileDisplayName(profile).c_str());
    message.append(" network profile. If you want to change the profile for this network go to the Networks tab.");
    trayIcon->showMessage("New network joined",
    message, QSystemTrayIcon::Information, 60*1000);
    statusMsg(message);
}

void MainWindow::alertOnNetworksUpdatedRestart() {
    if (updateState == Init)
        return;
    QString message = "There was a change in the active networks - Stubby is restarting to switch to the ";
    message.append(m_configMgr->getCurrentNetworksString().c_str());
    message.append(" network profile.");
    trayIcon->showMessage("Stubby is restarting",
    message, QSystemTrayIcon::Information, 60*1000);
    statusMsg(message);
}

void MainWindow::on_showLogButton_toggled() {
    if (ui->showLogButton->isChecked()) {
        logMsg("\n**Log display started**\n");
        m_logMgr->start();
    }
    else {
        logMsg("\n**Log display stopped**\n");
        m_logMgr->stop();
    }
}

void MainWindow::on_hideDetailsCheckBox_toggled() {
    if (ui->hideDetailsCheckBox->isChecked()) {
        ui->detailsLabel->setVisible(false);
        stubbySettings->setValue("info/hideDetails", true);
    }
    else {
        ui->detailsLabel->setVisible(true);
        stubbySettings->setValue("info/hideDetails", false);
    }
}

void MainWindow::on_helpButton_clicked() {
    QDesktopServices::openUrl (QUrl("https://dnsprivacy.org/wiki/display/DP/Stubby+Manager+GUI"));
}

void MainWindow::on_serviceStateChanged(ServiceMgr::ServiceState state) {

    qDebug("Stubby Service state changed from %s to %s ", getServiceStateString(m_serviceState).toLatin1().data(), getServiceStateString(state).toLatin1().data());
    m_serviceState = state;
    switch (m_serviceState) {
        case ServiceMgr::Running:
            ui->serviceStatus->setPixmap(*greenPixmap);
            break;
        case ServiceMgr::Stopped:
            ui->serviceStatus->setPixmap(*greyPixmap);
            break;
        case ServiceMgr::Error:
            ui->serviceStatus->setPixmap(*redPixmap);
            if (updateState == Start || updateState == Stop || updateState == Restart) {
                setTopPanelStatus();
                updateState = None;
            }
            return;
        default:
            ui->serviceStatus->setPixmap(*yellowPixmap);
            break;
    }

    if (updateState == None)
        return;

    if (updateState == Start) {
        if (m_serviceState == ServiceMgr::Running) {
            if (m_networkMgr->setLocalhost())
                handleError();
        }
        else if (m_serviceState == ServiceMgr::Stopped) {
            // error, reset network
            if (m_networkMgr->unsetLocalhost())
                handleError();
        }
    }
    else if (updateState == Stop && m_serviceState == ServiceMgr::Stopped) {
        setTopPanelStatus();
        updateState = None;
        return;
    }
    else if (updateState == Restart) {
        if (m_serviceState == ServiceMgr::Stopped) {
            if (m_serviceMgr->start(*m_configMgr))
                handleError();
        }
        else if (m_serviceState == ServiceMgr::Running) {
            on_testButton_clicked();
            setTopPanelStatus();
            updateState = None;
            return;
        }
    }
    else if (updateState == Probe)
        m_networkMgr->getDNSState(true);

    setTopPanelStatus();
}

void MainWindow::on_DNSStateChanged(NetworkMgr::NetworkState state) {

    qDebug("Network DNS state changed from %s to %s ", getNetworkStateString(m_networkState).toLatin1().data(), getNetworkStateString(state).toLatin1().data());
    m_networkState = state;
    if (m_networkState == NetworkMgr::Localhost)  ui->networkStatus->setPixmap(*greenPixmap);
    else if (m_networkState == NetworkMgr::NotLocalhost)  ui->networkStatus->setPixmap(*greyPixmap);
    else ui->networkStatus->setPixmap(*yellowPixmap);

    //setTopPanelNetworkInfo();
    if (updateState == None)
        return;

    if (updateState == Stop && (m_serviceState == ServiceMgr::Running || m_serviceState == ServiceMgr::Starting)) {
        if (m_serviceMgr->stop())
            handleError();
        setTopPanelStatus();
        return;
    }
    setTopPanelStatus();
    on_testButton_clicked();
    updateState = None;
}

void MainWindow::refreshNetworks(std::map<std::string, NetworkMgr::interfaceInfo> running_networks) {
    m_configMgr->updateNetworks(running_networks);
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

void MainWindow::handleError() {
    ui->runningStatus->setText("An error occurred");
    statusMsg("An Error occurred while stubby was starting or stopping");
    ui->stubbyStatus->setPixmap(*redPixmap);
    updateState = None;
    timer->stop();
}

void MainWindow::handleCancel() {
    statusMsg("The action was cancelled");
    setTopPanelStatus();
    updateState = None;
    timer->stop();
}

bool MainWindow::isServiceRunning() const {
    return (m_serviceState == ServiceMgr::Running);
}

int MainWindow::handleUnsavedChanges() {

    // Are there unsaved changes to any config?
    if (!ui->applyAllButton->isEnabled())
        return 0;

    //TODO: We should be able to offer saving just the bits that matter...
    QMessageBox msgBox;
    setMinimumSize(200,200);
    msgBox.setText("There are unsaved changes to profiles or networks");
    msgBox.setInformativeText("Do you want to save ALL your changes before continuing?");
    msgBox.setStandardButtons(QMessageBox::Save | QMessageBox::Discard | QMessageBox::Cancel);
    msgBox.setDefaultButton(QMessageBox::Save);
    int ret = msgBox.exec();
    int result = 0;
    switch (ret) {
      case QMessageBox::Save:
         m_configMgr->saveAll(true);
         break;
      case QMessageBox::Discard:
         m_configMgr->restoreSaved();
         break;
      case QMessageBox::Cancel:
         result = 1;
         break;
      default:
         // should never be reached
         break;
   }
   return result;
}


void MainWindow::setTopPanelStatus() {

    if (updateState == None)
        return;

    qDebug ("Updating overall state: Service is %s and Network %s ", getServiceStateString(m_serviceState).toLatin1().data(), getNetworkStateString(m_networkState).toLatin1().data());
    if (m_serviceState == ServiceMgr::Running &&
        m_networkState == NetworkMgr::Localhost) {
        ui->runningStatus->setText(getServiceStateString(m_serviceState));
        ui->stubbyStatus->setPixmap(*greenPixmap);
        trayIcon->setIcon(QIcon(":/images/stubby@245x145_green.png"));
        ui->onOffSlider->setChecked(true);
    }
    else if (m_serviceState == ServiceMgr::Stopped &&
             m_networkState == NetworkMgr::NotLocalhost) {
        ui->runningStatus->setText(getServiceStateString(m_serviceState));
        ui->stubbyStatus->setPixmap(*greyPixmap);
        trayIcon->setIcon(QIcon(":/images/stubby@245x145.png"));
        ui->onOffSlider->setChecked(false);
    }
    else if ((m_serviceState == ServiceMgr::Running &&
              m_networkState == NetworkMgr::NotLocalhost) ||
             (m_serviceState == ServiceMgr::Stopped &&
              m_networkState == NetworkMgr::Localhost)) {
        if (updateState == Start || updateState == Stop || updateState == Restart)
            ui->runningStatus->setText("Waiting...");
        else
            ui->runningStatus->setText("Partly running...");
        ui->stubbyStatus->setPixmap(*yellowPixmap);
        trayIcon->setIcon(QIcon(":/images/stubby@245x145_red.png"));
    }
    else if ((m_serviceState == ServiceMgr::Unknown &&
             m_networkState == NetworkMgr::Unknown) ||
             m_serviceState == ServiceMgr::Error ) {
             //m_networkState == NetworkMgr::Unknown) {
        ui->runningStatus->setText(getServiceStateString(m_serviceState));
        ui->stubbyStatus->setPixmap(*redPixmap);
        trayIcon->setIcon(QIcon(":/images/stubby@245x145_red.png"));
        ui->onOffSlider->setChecked(false);
    }
    else {
        ui->runningStatus->setText("Waiting...");
        ui->stubbyStatus->setPixmap(*yellowPixmap);
        trayIcon->setIcon(QIcon(":/images/stubby@245x145_red.png"));
    }

    ui->restartButton->setEnabled(m_serviceState == ServiceMgr::Running);

}

void MainWindow::on_userProfileEditInProgress()
{
    setMainButtonStates();
}

void MainWindow::on_userNetworksEditInProgress()
{
    setMainButtonStates();
}

void MainWindow::on_SavedConfigChanged(bool restart) {

    qInfo("Refreshing displayed Config and Current Info");
    m_networksWidget->setNWGuiState();
    m_untrustedNetworkWidget->setNPWGuiState();
    m_trustedNetworkWidget->setNPWGuiState();
    m_hostileNetworkWidget->setNPWGuiState();
    setMainButtonStates();
    setTopPanelNetworkInfo();

    if (m_serviceState == ServiceMgr::Running && restart && updateState != Init) {
        updateState = Restart;
        m_serviceMgr->restart();
    }
}

void MainWindow::setMainButtonStates()
{
    bool unsaved = m_configMgr->modifiedFromSavedConfig();
    bool notdefault = m_configMgr->modifiedFromFactoryDefaults();

    ui->applyAllButton->setEnabled(unsaved);
    ui->discardAllButton->setEnabled(unsaved);
    ui->revertAllButton->setEnabled(notdefault);
}

void MainWindow::on_applyAllButton_clicked()
{
    m_configMgr->saveAll(true);
}

void MainWindow::on_discardAllButton_clicked()
{
    m_configMgr->restoreSaved();
}

void MainWindow::on_revertAllButton_clicked()
{
    m_configMgr->restoreFactory();
}

void MainWindow::setTopPanelNetworkInfo()
{
    ui->network_profile->setText(m_configMgr->getCurrentProfileString().c_str());
    ui->network_name->setText(m_configMgr->getCurrentNetworksString().c_str());
}
