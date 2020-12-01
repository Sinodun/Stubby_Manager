/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NETWORKS_H
#define NETWORKS_H

#include <QModelIndex>
#include <QSortFilterProxyModel>
#include <QWidget>

#include "configmanager.h"
#include "networkprofiletablemodel.h"
#include "ui_networkswidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class NetworkList; }
QT_END_NAMESPACE

class NetworksWidget : public QWidget
{
    Q_OBJECT

public:
    explicit NetworksWidget(ConfigMgr& configMgr, QWidget* parent = nullptr);
    virtual ~NetworksWidget();

    void setNWGuiState();

signals:
    void userNetworksEditInProgress();

public slots:
    void on_applyButton_clicked();
    void on_discardButton_clicked();
    //void on_forgetButton_clicked();
    void on_defaultProfile_activated(int index);

    void on_networkTableDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles = QVector<int>());
    //void on_networkTableSelectionChanged(const QItemSelection& selected, const QItemSelection& deselected);
    void PersistentEdit();

private:
    void setNWButtonStates();

    Ui::NetworkList* ui;

    ConfigMgr& m_configMgr;
    NetworkProfileTableModel* m_networkTableModel;
    QSortFilterProxyModel* m_wifiModel;
    QSortFilterProxyModel* m_wiredModel;
    QItemSelectionModel* m_selectionWifiModel;
    QItemSelectionModel* m_selectionWiredModel;
};


#endif

/* Local Variables: */
/* mode: c++        */
/* End:             */
