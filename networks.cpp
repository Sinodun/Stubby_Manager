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
#include "networks.h"

NetworkListWidget::NetworkListWidget(ConfigMgr& configMgr, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::NetworkList),
      m_configMgr(configMgr)

{
    ui->setupUi(this);

    ui->defaultProfile->addItem(QString::fromStdString(Config::networkProfileDisplayName(Config::NetworkProfile::trusted)), QVariant(static_cast<int>(Config::NetworkProfile::trusted)));
    ui->defaultProfile->addItem(QString::fromStdString(Config::networkProfileDisplayName(Config::NetworkProfile::untrusted)), QVariant(static_cast<int>(Config::NetworkProfile::untrusted)));
    ui->defaultProfile->addItem(QString::fromStdString(Config::networkProfileDisplayName(Config::NetworkProfile::hostile)), QVariant(static_cast<int>(Config::NetworkProfile::hostile)));

    m_networkTableModel = new NetworkProfileTableModel(m_configMgr.displayedConfig);
    ui->networkTable->setModel(m_networkTableModel);
    ui->networkTable->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->networkTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->networkTable->setSelectionMode(QAbstractItemView::ExtendedSelection);
    ui->networkTable->setItemDelegateForColumn(1, new NetworkProfileDelegate(ui->networkTable));

    m_selectionModel = ui->networkTable->selectionModel();
    ui->forgetButton->setEnabled(m_selectionModel->hasSelection());

    connect(m_networkTableModel, &NetworkProfileTableModel::dataChanged,
            this, &NetworkListWidget::on_networkTableDataChanged);
    connect(&m_configMgr, &ConfigMgr::configChanged,
            this, &NetworkListWidget::on_globalConfigChanged);
    connect(m_selectionModel, &QItemSelectionModel::selectionChanged,
            this, &NetworkListWidget::on_networkTableSelectionChanged);
}

NetworkListWidget::~NetworkListWidget()
{
    delete ui;

    delete m_networkTableModel;
}

void NetworkListWidget::setGuiState()
{
    ui->defaultProfile->setCurrentText(QString::fromStdString(Config::networkProfileDisplayName(m_configMgr.displayedConfig.defaultNetworkProfile)));
    setButtonStates();
}

void NetworkListWidget::on_defaultProfile_activated(int index)
{
    int i = ui->defaultProfile->itemData(index).toInt();
    Config::NetworkProfile np = static_cast<Config::NetworkProfile>(i);

    if ( m_configMgr.displayedConfig.defaultNetworkProfile != np )
    {
        m_configMgr.displayedConfig.defaultNetworkProfile = np;
        setGuiState();
        emit globalConfigChanged();
    }
}

void NetworkListWidget::on_applyButton_clicked()
{
    m_configMgr.saveNetworks();
    setButtonStates();
    m_networkTableModel->configChanged();
}

void NetworkListWidget::on_discardButton_clicked()
{
    m_configMgr.networksRestoreSaved();
    setButtonStates();
    m_networkTableModel->configChanged();
}

void NetworkListWidget::on_forgetButton_clicked()
{
    auto& networks = m_configMgr.displayedConfig.networks;

    for ( const auto& row : m_selectionModel->selectedRows() )
    {
        std::string name = m_networkTableModel->data(row).toString().toStdString();
        networks.erase(networks.find(name));
    }
    m_selectionModel->reset();

    setButtonStates();
    m_networkTableModel->configChanged();
}

void NetworkListWidget::on_globalConfigChanged()
{
    setGuiState();
    m_networkTableModel->configChanged();
}

void NetworkListWidget::on_networkTableDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    setButtonStates();
}

void NetworkListWidget::on_networkTableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected)
{
    ui->forgetButton->setEnabled(!selected.empty());
}

void NetworkListWidget::setButtonStates()
{
    bool unsaved = m_configMgr.networksModifiedFromSaved();

    ui->applyButton->setEnabled(unsaved);
    ui->discardButton->setEnabled(unsaved);

    ui->forgetButton->setEnabled(m_selectionModel->hasSelection());

    emit stateUpdated(unsaved);
}
