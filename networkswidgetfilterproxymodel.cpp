#include "networkswidgetfilterproxymodel.h"
#include "networkprofiletablemodel.h"

NetworksWidgetFilterProxyModel::NetworksWidgetFilterProxyModel(QObject *parent, QString view): QSortFilterProxyModel(parent), m_view(view) {}

bool NetworksWidgetFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const {
    QModelIndex index = sourceModel()->index(sourceRow, 2, sourceParent);
    return(sourceModel()->data(index).toString().contains(m_view));
}

QVariant NetworksWidgetFilterProxyModel::headerData(int section, Qt::Orientation orientation, int role) const
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


bool NetworksWidgetFilterProxyModel::lessThan(const QModelIndex &left,
                                              const QModelIndex &right) const {
    QVariant leftData  = sourceModel()->data(left);
    QVariant rightData = sourceModel()->data(right);
    QColor leftColour  = sourceModel()->data(left, Qt::BackgroundRole).value<QColor>();
    QColor rightColour = sourceModel()->data(right, Qt::BackgroundRole).value<QColor>();

    if (leftColour != rightColour) {
        if (leftColour == QColor::fromRgb(222, 255, 222))
            return false;
        else
            return true;
    }
    else if (leftData.userType() == QMetaType::QString)
         return QString::localeAwareCompare(leftData.toString(), rightData.toString()) > 0;
}
