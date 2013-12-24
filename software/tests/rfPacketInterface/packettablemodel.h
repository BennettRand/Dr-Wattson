#ifndef PACKETTABLEMODEL_H
#define PACKETTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QByteArray>
#include <stdint.h>
#include <QDebug>

#include "protocol.h"

class PacketTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PacketTableModel(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void addData(rxHeader_t header, QByteArray data);

signals:

public slots:

private:
    QList<rxHeader_t> header_list;
    QList<QByteArray> data_list;

};

#endif // PACKETTABLEMODEL_H
