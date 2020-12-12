/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <QDebug>
#include <QCheckBox>
#include <QTableView>
#include <QDesktopServices>
#include <QUrl>

#include "networkprofilewidget.h"
#include "serverdatadialog.h"

NetworkProfileWidget::NetworkProfileWidget(ConfigMgr& configMgr, Config::NetworkProfile np, QWidget* parent)
    : QWidget(parent),
      ui(new Ui::NetworkProfileWidget),
      m_configMgr(configMgr),
      m_networkProfile(np)

{
    ui->setupUi(this);

    m_serverTableModel = new ServersTableModel(m_configMgr.displayedConfig, m_networkProfile);
    ui->serverTable->setModel(m_serverTableModel);
    ui->serverTable->horizontalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
    ui->serverTable->hideColumn(5);
    connect(ui->serverTable, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onTableClicked(const QModelIndex &)));
    connect(m_serverTableModel, &ServersTableModel::dataChanged,
            this, &NetworkProfileWidget::on_serverTableDataChanged);
    ui->editServerButton->setEnabled(false);
    ui->deleteServerButton->setVisible(false);
    ui->addServerButton->setVisible(false);
    ui->filterServersButton->setVisible(false);
    ui->validateData->setVisible(false);
    setNPWButtonStates();
}

NetworkProfileWidget::~NetworkProfileWidget()
{
    delete ui;
    delete m_serverTableModel;
}

void NetworkProfileWidget::onTableClicked(const QModelIndex &index) {
    if (index.column() != 2)
        return;
    QVariant link = m_serverTableModel->data(index.siblingAtColumn(2), Qt::DisplayRole);
    QString url = "https://" + link.toString();
    QDesktopServices::openUrl (QUrl(url));
}

void NetworkProfileWidget::on_serverTable_clicked() {
    // Enable buttons if one and only one row is selected
    QItemSelectionModel *select = ui->serverTable->selectionModel();
    QModelIndexList selection = select->selectedIndexes();
    if (selection.count() == 0) {
        ui->editServerButton->setEnabled(false);
        return;
    }
    int count = 0, row;
    foreach (QModelIndex index, selection) {
        if (count == 0) {
         row = index.row();
         count++;
         continue;
        }
        if (row != index.row()) {
            ui->editServerButton->setEnabled(false);
            return;
        }
    }
    ui->editServerButton->setEnabled(true);
}

void NetworkProfileWidget::on_editServerButton_clicked() {
    QItemSelectionModel *select = ui->serverTable->selectionModel();
    QModelIndexList selection = select->selectedIndexes();
    if (selection.count() == 0)
        return;
    int row = selection.begin()->row();
    Config::Server& server = m_configMgr.displayedConfig.servers[row];
    ServerDataDialog dialog(server, this);
    dialog.exec();
    ui->editServerButton->setEnabled(false);
    setNPWGuiState();
}

void NetworkProfileWidget::on_alwaysAuthenticate_stateChanged(int state)
{
    m_configMgr.displayedConfig.profiles[m_networkProfile].alwaysAuthenticate = (state == Qt::CheckState::Checked);
    setNPWButtonStates();
}

void NetworkProfileWidget::on_encryptAllTraffic_stateChanged(int state)
{
    m_configMgr.displayedConfig.profiles[m_networkProfile].encryptAll = (state == Qt::CheckState::Checked);
    // If encryption is not enabled, authentication has no meaning
    ui->alwaysAuthenticate->setCheckState((Qt::CheckState)state);
    ui->alwaysAuthenticate->setEnabled(state == Qt::CheckState::Checked);
    setNPWButtonStates();
}

void NetworkProfileWidget::on_roundRobin_stateChanged(int state)
{
    m_configMgr.displayedConfig.profiles[m_networkProfile].roundRobin = (state == Qt::CheckState::Checked);
    setNPWButtonStates();
}

void NetworkProfileWidget::on_validateData_stateChanged(int state)
{
    m_configMgr.displayedConfig.profiles[m_networkProfile].validateData = (state == Qt::CheckState::Checked);
    setNPWButtonStates();
}

void NetworkProfileWidget::on_applyButton_clicked()
{
    m_configMgr.saveProfile(m_networkProfile);
}

void NetworkProfileWidget::on_discardButton_clicked()
{
    m_configMgr.profileRestoreSaved(m_networkProfile);
    setNPWButtonStates();
    m_serverTableModel->STPConfigChanged();
}

void NetworkProfileWidget::on_revertButton_clicked()
{
    m_configMgr.profileRestoreFactory(m_networkProfile);
    setNPWButtonStates();
    m_serverTableModel->STPConfigChanged();
}

void NetworkProfileWidget::on_serverTableDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles)
{
    setNPWButtonStates();
}

void NetworkProfileWidget::setNPWGuiState()
{
    ui->alwaysAuthenticate->setCheckState(m_configMgr.displayedConfig.profiles[m_networkProfile].alwaysAuthenticate ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->encryptAllTraffic->setCheckState(m_configMgr.displayedConfig.profiles[m_networkProfile].encryptAll ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->validateData->setCheckState(m_configMgr.displayedConfig.profiles[m_networkProfile].validateData ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);
    ui->roundRobin->setCheckState(m_configMgr.displayedConfig.profiles[m_networkProfile].roundRobin ? Qt::CheckState::Checked : Qt::CheckState::Unchecked);

    m_serverTableModel->STPConfigChanged();
    setNPWButtonStates();
}

void NetworkProfileWidget::setNPWButtonStates()
{
    bool unsaved = m_configMgr.profileModifiedFromSaved(m_networkProfile);
    bool notdefault = m_configMgr.profileModifiedFromFactory(m_networkProfile);

    ui->applyButton->setEnabled(unsaved);
    ui->discardButton->setEnabled(unsaved);
    ui->revertButton->setEnabled(notdefault);

    //if (unsaved)
    emit userProfileEditInProgress();
}
