#ifndef NETWORKLISTFILTERPROXYMODEL_H
#define NETWORKLISTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class NetworkListFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit NetworkListFilterProxyModel(QObject *parent = nullptr, QString view = "");
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
private:
    QString m_view;
};

#endif // NETWORKLISTFILTERPROXYMODEL_H
