/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include "networkprofiletablemodel.h"

NetworkProfileTableModel::NetworkProfileTableModel(Config& config, QObject* parent)
    : m_config(config), QAbstractTableModel(parent)
{
}

NetworkProfileTableModel::~NetworkProfileTableModel()
{
}

int NetworkProfileTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 2;
}

int NetworkProfileTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return static_cast<int>(m_config.networks.size());
}

QVariant NetworkProfileTableModel::data(const QModelIndex& index, int role) const
{
    if ( !index.isValid() )
        return QVariant();

    int row = index.row();

    for ( const auto& n : m_config.networks )
    {
        if ( row-- > 0 )
            continue;

        if ( role == Qt::DisplayRole )
        {
            switch (index.column())
            {
            case 0:
                return QString::fromStdString(n.first);

            case 1:
                return QString::fromStdString(Config::networkProfileChoiceDisplayName(n.second));
            }
        }
        else if ( role == Qt::EditRole)
        {
            switch (index.column())
            {
            case 1:
                return QVariant(static_cast<int>(n.second));
            }
        }
    }

    return QVariant();
}

Qt::ItemFlags NetworkProfileTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags res = Qt::ItemIsEnabled | Qt::ItemIsSelectable;

    switch ( index.column() )
    {
    case 1:
        res |= Qt::ItemIsEditable;
        break;
    }

    return res;
}

QVariant NetworkProfileTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role != Qt::DisplayRole || orientation != Qt::Horizontal )
        return QVariant();

    switch (section)
    {
    case 0: return QString("Network");
    case 1: return QString("Profile");
    }

    return QVariant();
}

bool NetworkProfileTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ( !index.isValid() || role != Qt::EditRole || index.column() != 1 )
        return false;

    int row = index.row();

    for ( const auto& n : m_config.networks )
    {
        if ( row-- > 0 )
            continue;

        m_config.networks[n.first] = Config::NetworkProfileChoice(value.toInt());
        emit dataChanged(index, index);
        return true;
    }
    return false;
}

void NetworkProfileTableModel::configChanged()
{
    beginResetModel();
    endResetModel();
}
