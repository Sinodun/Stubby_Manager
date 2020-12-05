/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <QColor>
#include <QFont>

#include "serverstablemodel.h"

ServersTableModel::ServersTableModel(Config& config, Config::NetworkProfile networkProfile, QObject* parent)
    : m_config(config), m_networkProfile(networkProfile), QAbstractTableModel(parent)
{

}

ServersTableModel::~ServersTableModel()
{
}

int ServersTableModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 6;
}

int ServersTableModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);

    std::size_t res = 0;

    for ( const auto& s : m_config.servers )
        if ( s.hidden.find(m_networkProfile) == s.hidden.end() )
            res += s.addresses.size();

    return static_cast<int>(res);
}

QVariant ServersTableModel::data(const QModelIndex& index, int role) const
{
    if ( !index.isValid() )
        return QVariant();

    int serverIndex, addressIndex;
    serverFromRow(index.row(), serverIndex, addressIndex);

    if ( serverIndex < 0 )
        return QVariant();

    const Config::Server& server(m_config.servers[serverIndex]);
    const std::string& address(server.addresses[addressIndex]);

    if ( role == Qt::DisplayRole )
    {
        switch (index.column())
        {
        case 1:     // Name
            return QString::fromStdString(server.name);

        case 2:     // Link
            return QString::fromStdString(server.link);

        case 3:     // Address
            return QString::fromStdString(address);

        case 4:     // TLS auth name
            return QString::fromStdString(server.tlsAuthName);

        case 5:     // Key digest value
            return QString::fromStdString(server.pubKeyDigestValue);
        }
    } else if ( role == Qt::CheckStateRole )
    {
        if ( index.column() == 0 )
        {
            if ( server.inactive.find(m_networkProfile) == server.inactive.end() )
                return Qt::Checked;
            else
                return Qt::Unchecked;
        }
    }
    else if (role == Qt::FontRole && index.column() == 2) {
            QFont font;
            font.setUnderline(true);
            return font;
    } else if (role == Qt::ForegroundRole && index.column() == 2) {
            return QColor(Qt::darkBlue);
    }

    return QVariant();
}

Qt::ItemFlags ServersTableModel::flags(const QModelIndex& index) const
{
    Qt::ItemFlags res = Qt::NoItemFlags;

    switch ( index.column() )
    {
    case 0:
        res |= Qt::ItemIsUserCheckable | Qt::ItemIsEditable | Qt::ItemIsEnabled;
        break;
    case 1:
        res |= Qt::ItemIsEnabled;
    case 2:
        res |= Qt::ItemIsEnabled | Qt::ItemIsSelectable;
        break;
    }
    return res;
}

QVariant ServersTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role != Qt::DisplayRole || orientation != Qt::Horizontal )
        return QVariant();

    switch (section)
    {
    case 0: return QString("Active");
    case 1: return QString("Name");
    case 2: return QString("Website");
    case 3: return QString("Main Address");
    case 4: return QString("TLS auth name");
    case 5: return QString("Key digest value");
    }

    return QVariant("");
}

bool ServersTableModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if ( !index.isValid() )
        return false;

    int serverIndex, addressIndex;
    serverFromRow(index.row(), serverIndex, addressIndex);

    if ( serverIndex < 0 )
        return false;

    if ( index.column() == 0 && role == Qt::CheckStateRole )
    {
        Config::Server& server(m_config.servers[serverIndex]);
        if ( static_cast<Qt::CheckState>(value.toInt()) == Qt::Checked )
            server.inactive.erase(m_networkProfile);
        else
            server.inactive.insert(m_networkProfile);

        // All rows for this server have changed.
        int startRow = index.row() - addressIndex;
        int endRow = startRow + static_cast<int>(server.addresses.size()) - 1;
        emit(dataChanged(index.siblingAtRow(startRow), index.siblingAtRow(endRow)));
        return true;
    }

    return false;
}

void ServersTableModel::STPConfigChanged()
{
    beginResetModel();
    endResetModel();
}

void ServersTableModel::serverFromRow(int row, int& serverIndex, int& addressIndex) const
{
    serverIndex = 0;
    addressIndex = 0;
    bool found = false;

    for ( auto& s : m_config.servers )
    {
        if ( s.hidden.find(m_networkProfile) == s.hidden.end() )
        {
            addressIndex = 0;
            for ( auto& a : s.addresses )
            {
                if ( row == 0 )
                {
                    found = true;
                    break;
                }

                --row;
                ++addressIndex;
            }

            if ( found )
                break;
        }
        ++serverIndex;
    }

    if ( !found )
    {
        serverIndex = -1;
        addressIndex = -1;
    }
}
