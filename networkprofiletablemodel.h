/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NETWORKPROFILETABLEMODEL_H
#define NETWORKPROFILETABLEMODEL_H

#include <QAbstractTableModel>
#include <QColor>
#include "configmanager.h"

class NetworkProfileTableModel : public QAbstractTableModel
{
    Q_OBJECT

public:
    NetworkProfileTableModel(Config& config, QObject* parent = nullptr);
    virtual ~NetworkProfileTableModel();

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    Qt::ItemFlags flags(const QModelIndex& index) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;

    void configChanged();

private:
    Config& m_config;
};

#endif

/* Local Variables: */
/* mode: c++        */
/* End:             */
