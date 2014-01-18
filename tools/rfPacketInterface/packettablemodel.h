#ifndef PACKETTABLEMODEL_H
#define PACKETTABLEMODEL_H

#include <QAbstractTableModel>
#include <QList>
#include <QByteArray>
#include <stdint.h>
#include <QDebug>
#include <QDateTime>

#include "../../hardware/firmware/baseStationFirmware/protocol.h"

class PacketTableModel : public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit PacketTableModel(QObject *parent = 0);

    enum dataEncoding {ASCII, HEX};

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex &parent) const;
    QVariant data(const QModelIndex &index, int role) const;
    QVariant headerData(int section, Qt::Orientation orientation, int role) const;
    Qt::ItemFlags flags(const QModelIndex &index) const;
    void addData(rxHeader_t header, QByteArray data, QDateTime time);
    void encodingChanged(enum dataEncoding encoding);
    QByteArray getRawData(int row);

signals:

public slots:

private:
    QList<rxHeader_t> header_list;
    QList<QByteArray> data_list;
    QList<QDateTime> time_list;
    enum dataEncoding currentEncoding;

};

#endif // PACKETTABLEMODEL_H
