/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NETWORKPROFILE_H
#define NETWORKPROFILE_H

#include <QModelIndex>
#include <QWidget>

#include "configmanager.h"
#include "serverstablemodel.h"
#include "ui_networkprofilewidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NetworkProfileWidget; }
QT_END_NAMESPACE

class NetworkProfileWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworkProfileWidget(ConfigMgr& configMsg, Config::NetworkProfile np, QWidget* parent = nullptr);
    virtual ~NetworkProfileWidget();

    void setNPWGuiState();

signals:
    void userProfileEditInProgress();

public slots:
    void on_alwaysAuthenticate_stateChanged(int state);
    void on_encryptAllTraffic_stateChanged(int state);
    void on_roundRobin_stateChanged(int state);
    void on_validateData_stateChanged(int state);

    void on_applyButton_clicked();
    void on_discardButton_clicked();
    void on_revertButton_clicked();
    //void on_serverTable_clicked();

    void on_serverTableDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    void onTableClicked(const QModelIndex &index);
private:
    void setNPWButtonStates();

    Ui::NetworkProfileWidget* ui;

    ConfigMgr& m_configMgr;
    Config::NetworkProfile m_networkProfile;
    ServersTableModel* m_serverTableModel;
};

#endif

/* Local Variables: */
/* mode: c++        */
/* End:             */
