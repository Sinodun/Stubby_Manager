#ifndef NETWORKLISTFILTERPROXYMODEL_H
#define NETWORKLISTFILTERPROXYMODEL_H

#include <QSortFilterProxyModel>

class NetworksWidgetFilterProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT
public:
    explicit NetworksWidgetFilterProxyModel(QObject *parent = nullptr, QString view = "");
    bool filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

private:
    QString m_view;
};

#endif // NETWORKLISTFILTERPROXYMODEL_H
