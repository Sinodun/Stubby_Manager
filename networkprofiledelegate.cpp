/*
 * Copyright 2020 Sinodun Internet Technologies Ltd.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, you can obtain one at https://mozilla.org/MPL/2.0/.
 */

#include <QComboBox>

#include "config.h"
#include "networkprofiledelegate.h"

NetworkProfileDelegate::NetworkProfileDelegate(QObject* parent)
    : QStyledItemDelegate(parent)
{
}

QWidget* NetworkProfileDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    QComboBox* cb = new QComboBox(parent);

    cb->addItem(QString::fromStdString(Config::networkProfileChoiceDisplayName(Config::NetworkProfileChoice::default)));
    cb->addItem(QString::fromStdString(Config::networkProfileChoiceDisplayName(Config::NetworkProfileChoice::trusted)));
    cb->addItem(QString::fromStdString(Config::networkProfileChoiceDisplayName(Config::NetworkProfileChoice::untrusted)));
    cb->addItem(QString::fromStdString(Config::networkProfileChoiceDisplayName(Config::NetworkProfileChoice::hostile)));
    return cb;
}

void NetworkProfileDelegate::setEditorData(QWidget* editor, const QModelIndex& index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(cb);
    cb->setCurrentIndex(index.data(Qt::EditRole).toInt());
}

void NetworkProfileDelegate::setModelData(QWidget* editor, QAbstractItemModel* model, const QModelIndex& index) const
{
    QComboBox* cb = qobject_cast<QComboBox*>(editor);
    Q_ASSERT(cb);
    model->setData(index, cb->currentIndex(), Qt::EditRole);
}
