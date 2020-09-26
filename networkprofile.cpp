/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <QDebug>
#include <QCheckBox>

#include "networkprofile.h"

NetworkProfileWidget::NetworkProfileWidget(ConfigMgr& configMgr, Config::NetworkProfile np, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::NetworkProfileWidget),
      m_configMgr(configMgr),
      m_np(np)

{
    ui->setupUi(this);

    m_serverTableModel = new ProfileServersTableModel(m_configMgr.displayedConfig, m_np);
    ui->serverTable->setModel(m_serverTableModel);
    ui->serverTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    connect(m_serverTableModel, &ProfileServersTableModel::dataChanged,
            this, &NetworkProfileWidget::on_serverTableDataChanged);
    connect(&m_configMgr, &ConfigMgr::configChanged,
            this, &NetworkProfileWidget::on_globalConfigChanged);
    setButtonStates();
}

NetworkProfileWidget::~NetworkProfileWidget()
{
    delete ui;

    delete m_serverTableModel;
}

void NetworkProfileWidget::on_useAsDefaultProfile_stateChanged(int state)
{
    if ( state == Qt::CheckState::Checked && m_configMgr.displayedConfig.defaultNewNetworkProfile != m_np )
    {
        m_configMgr.displayedConfig.defaultNewNetworkProfile = m_np;
        m_configMgr.displayedConfig.defaultNewNetworkProfileSet = true;
        emit globalConfigChanged();
    }
    else if ( state == Qt::CheckState::Unchecked && m_configMgr.displayedConfig.defaultNewNetworkProfile == m_np )
    {
        m_configMgr.displayedConfig.defaultNewNetworkProfileSet = false;
        emit globalConfigChanged();
    }
    setButtonStates();
}

void NetworkProfileWidget::on_alwaysAuthenticate_stateChanged(int state)
{
    m_configMgr.displayedConfig.profiles[m_np].alwaysAuthenticate = (state == Qt::CheckState::Checked);
    setButtonStates();
}

void NetworkProfileWidget::on_encryptAllTraffic_stateChanged(int state)
{
    m_configMgr.displayedConfig.profiles[m_np].encryptAll = (state == Qt::CheckState::Checked);
    setButtonStates();
}

void NetworkProfileWidget::on_roundRobin_stateChanged(int state)
{
    m_configMgr.displayedConfig.profiles[m_np].roundRobin = (state == Qt::CheckState::Checked);
    setButtonStates();
}

void NetworkProfileWidget::on_validateData_stateChanged(int state)
{
    m_configMgr.displayedConfig.profiles[m_np].validateData = (state == Qt::CheckState::Checked);
    setButtonStates();
}

void NetworkProfileWidget::on_applyButton_clicked()
{
    m_configMgr.saveProfile(m_np);
    setButtonStates();
    m_serverTableModel->configChanged();
}

void NetworkProfileWidget::on_discardButton_clicked()
{
    m_configMgr.profileRestoreSaved(m_np);
    setButtonStates();
    m_serverTableModel->configChanged();
}

void NetworkProfileWidget::on_revertButton_clicked()
{
    m_configMgr.profileRestoreFactory(m_np);
    setButtonStates();
    m_serverTableModel->configChanged();
}

void NetworkProfileWidget::on_globalConfigChanged()
{
    setGuiState();
    m_serverTableModel->configChanged();
}

void NetworkProfileWidget::on_serverTableDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    setButtonStates();
}

void NetworkProfileWidget::setGuiState()
{
    ui->useAsDefaultProfile->setCheckState(m_configMgr.displayedConfig.defaultNewNetworkProfileSet && m_configMgr.displayedConfig.defaultNewNetworkProfile == m_np ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    ui->alwaysAuthenticate->setCheckState(m_configMgr.displayedConfig.profiles[m_np].alwaysAuthenticate ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->encryptAllTraffic->setCheckState(m_configMgr.displayedConfig.profiles[m_np].encryptAll ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->validateData->setCheckState(m_configMgr.displayedConfig.profiles[m_np].validateData ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->roundRobin->setCheckState(m_configMgr.displayedConfig.profiles[m_np].roundRobin ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    setButtonStates();
}

void NetworkProfileWidget::setButtonStates()
{
    bool unsaved = m_configMgr.profileModifiedFromSaved(m_np);
    bool notdefault = m_configMgr.profileModifiedFromFactory(m_np);
    bool not_default_profile = ( m_configMgr.displayedConfig.defaultNewNetworkProfile != m_np );

    ui->applyButton->setEnabled(unsaved);
    ui->discardButton->setEnabled(unsaved);
    ui->revertButton->setEnabled(notdefault);
    ui->useAsDefaultProfile->setEnabled(not_default_profile);

    emit stateUpdated(m_np, unsaved, notdefault);
}
