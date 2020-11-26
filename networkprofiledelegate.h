/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#ifndef NETWORKPROFILEDELEGATE_H
#define NETWORKPROFILEDELEGATE_H

#include <QStyledItemDelegate>

class NetworkProfileDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    NetworkProfileDelegate(QObject* parent = nullptr);

    QWidget* createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;
};


#endif

/* Local Variables: */
/* mode: c++        */
/* End:             */
