#include "networklistfilterproxymodel.h"
#include "networkprofiletablemodel.h"

NetworkListFilterProxyModel::NetworkListFilterProxyModel(QObject *parent, QString view): QSortFilterProxyModel(parent), m_view(view) {}

bool NetworkListFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
    return(sourceModel()->data(index).toString().contains(m_view));
}
