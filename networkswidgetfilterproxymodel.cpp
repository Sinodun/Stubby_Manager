#include "networkswidgetfilterproxymodel.h"
#include "networkprofiletablemodel.h"

NetworkListFilterProxyModel::NetworkListFilterProxyModel(QObject *parent, QString view): QSortFilterProxyModel(parent), m_view(view) {}

bool NetworkListFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
    return(sourceModel()->data(index).toString().contains(m_view));
}

QVariant NetworkListFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if ( role != Qt::DisplayRole || orientation != Qt::Horizontal )
        return QVariant();

    if ( m_view == "Ethernet" ) {
        switch (section)
        {
        case 0: return tr("Wired Networks");
        case 1: return tr("Profile");
        }
    } else if (m_view == "WiFi") {
        switch (section)
        {
        case 0: return tr("WiFi Networks");
        case 1: return tr("Profile");
        }
    }
    return QVariant();
}
