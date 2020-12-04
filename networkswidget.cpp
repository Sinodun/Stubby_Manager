/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <QDebug>

#include "config.h"
#include "networkprofiledelegate.h"
#include "networkswidget.h"
#include "networkswidgetfilterproxymodel.h"
#include "mainwindow.h"

NetworksWidget::NetworksWidget(ConfigMgr& configMgr, MainWindow* main, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::NetworkList),
      m_configMgr(configMgr),
      m_mainWindow(main)

{
    ui->setupUi(this);

    ui->defaultProfile->addItem(QString::fromStdString(Config::networkProfileDisplayName(Config::NetworkProfile::trusted)), QVariant(static_cast<int>(Config::NetworkProfile::trusted)));
    ui->defaultProfile->addItem(QString::fromStdString(Config::networkProfileDisplayName(Config::NetworkProfile::untrusted)), QVariant(static_cast<int>(Config::NetworkProfile::untrusted)));
    ui->defaultProfile->addItem(QString::fromStdString(Config::networkProfileDisplayName(Config::NetworkProfile::hostile)), QVariant(static_cast<int>(Config::NetworkProfile::hostile)));

    m_networkTableModel = new NetworkProfileTableModel(m_configMgr.displayedConfig);

    m_wifiModel = new NetworksWidgetFilterProxyModel(this, "WiFi");
    m_wifiModel->setSourceModel(m_networkTableModel);
    m_wiredModel = new NetworksWidgetFilterProxyModel(this, "Ethernet");
    m_wiredModel->setSourceModel(m_networkTableModel);

    ui->networkTable->setModel(m_wifiModel);
    ui->networkTable->hideColumn(2);
    ui->networkTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->networkTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->networkTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->networkTable->setItemDelegateForColumn(1, new NetworkProfileDelegate(ui->networkTable));
    ui->networkTable->setSortingEnabled(true);
    m_selectionWifiModel = ui->networkTable->selectionModel();

    ui->networkWiredTable->setModel(m_wiredModel);
    ui->networkWiredTable->hideColumn(2);
    ui->networkWiredTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->networkWiredTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->networkWiredTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->networkWiredTable->setItemDelegateForColumn(1, new NetworkProfileDelegate(ui->networkWiredTable));
    ui->networkWiredTable->setSortingEnabled(true);
    m_selectionWiredModel = ui->networkWiredTable->selectionModel();

    bool sel=m_selectionWifiModel->hasSelection();
    ui->forgetButton->setEnabled(sel);

    connect(m_networkTableModel, &NetworkProfileTableModel::dataChanged,
            this, &NetworksWidget::on_networkTableDataChanged);
    connect(m_selectionWifiModel, &QItemSelectionModel::selectionChanged,
            this, &NetworksWidget::on_networkTableSelectionChanged);
//    connect(m_selectionWiredModel, &QItemSelectionModel::selectionChanged,
//            this, &NetworksWidget::on_networkTableSelectionChanged);
    connect(m_networkTableModel, &NetworkProfileTableModel::modelReset,
             this, &NetworksWidget::PersistentEdit);

}

NetworksWidget::~NetworksWidget()
{
    delete ui;
    delete m_networkTableModel;
}

void NetworksWidget::PersistentEdit()
{
    for (int i=0; i<m_wifiModel->rowCount(); ++i)
        ui->networkTable->openPersistentEditor(m_wifiModel->index(i, 1));
    for (int i=0; i<m_wiredModel->rowCount(); ++i)
        ui->networkWiredTable->openPersistentEditor(m_wiredModel->index(i, 1));
}

void NetworksWidget::on_applyButton_clicked()
{
    m_configMgr.saveNetworks();
}

void NetworksWidget::on_discardButton_clicked()
{
    // Only affects displayed config (so local)
    m_configMgr.networksRestoreSaved();
    setNWGuiState();
}

void NetworksWidget::on_defaultProfile_activated(int /*index*/) {
    m_configMgr.displayedConfig.defaultNetworkProfile = Config::networkProfileFromYamlKey(ui->defaultProfile->currentText().toLower().toUtf8().constData());
    setNWButtonStates();
}

void NetworksWidget::on_networkTableDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    setNWButtonStates();
}

void NetworksWidget::setNWGuiState()
{
    qInfo("Setting Network Widget State");
    ui->defaultProfile->setCurrentText(QString::fromStdString(Config::networkProfileDisplayName(m_configMgr.displayedConfig.defaultNetworkProfile)));
    setNWButtonStates();
    m_networkTableModel->NPTMConfigChanged();
}

void NetworksWidget::setNWButtonStates()
{
    bool unsaved = m_configMgr.networksModifiedFromSaved();

    ui->applyButton->setEnabled(unsaved);
    ui->discardButton->setEnabled(unsaved);
    bool sel=m_selectionWifiModel->hasSelection();
    ui->forgetButton->setEnabled(sel);

    if (unsaved)
        emit userNetworksEditInProgress();
}

void NetworksWidget::on_forgetButton_clicked()
{
    auto& networks = m_configMgr.displayedConfig.networks;

    for ( const auto& row : m_selectionWifiModel->selectedRows() )
    {
        std::string name = m_wifiModel->data(row).toString().toStdString();
        if (m_wifiModel->data(row, Qt::BackgroundRole).value<QColor>() == QColor::fromRgb(222, 255, 222)) {
            QString message = "Ignoring 'Forget' for active Wifi network ";
           message.append(name.c_str());
           m_mainWindow->systrayMsg(message);
           m_mainWindow->statusMsg(message);
           continue;
        }
        networks.erase(networks.find(name));
        qInfo("Forgetting wireless network %s", name.c_str());
    }
//    for ( const auto& row : m_selectionWiredModel->selectedRows() )
//    {
//        std::string name = m_wiredModel->data(row).toString().toStdString();
//        if (m_wiredModel->data(row, Qt::BackgroundRole).value<QColor>() == QColor::fromRgb(222, 255, 222)) {
//           qInfo("Ignoring active network %s ", name.c_str());
//           continue;
//        }
//        networks.erase(networks.find(name));
//        qInfo("Forgetting wired network %s", name.c_str());
//    }
//    m_selectionWiredModel->reset();
    m_selectionWifiModel->reset();

    setNWButtonStates();
    m_networkTableModel->NPTMConfigChanged();
}

void NetworksWidget::on_networkTableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    ui->forgetButton->setEnabled(!selected.empty());
}
