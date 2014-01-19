#include "packettablemodel.h"

PacketTableModel::PacketTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
    currentEncoding = ASCII;
}

void PacketTableModel::addData(rxHeader_t header, QByteArray data, QDateTime time)
{
    beginInsertRows(QModelIndex(),header_list.size(),header_list.size());
    time_list.append(time);
    header_list.append(header);
    data_list.append(data);
    endInsertRows();
}

QByteArray PacketTableModel::getRawData(int row)
{
    return data_list.at(row);
}

void PacketTableModel::encodingChanged(enum dataEncoding encoding)
{
    currentEncoding = encoding;
    emit dataChanged(createIndex(0,3),createIndex(header_list.size(),3));
}

int PacketTableModel::rowCount(const QModelIndex &parent) const
{
    return header_list.size();
}

int PacketTableModel::columnCount(const QModelIndex &parent) const
{
    return 5;
}

Qt::ItemFlags PacketTableModel::flags(const QModelIndex &index) const
{
    return Qt::ItemIsSelectable | Qt::ItemIsEnabled | Qt::ItemNeverHasChildren;
}

QVariant PacketTableModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        switch(index.column())
        {
        case 0:
            return time_list.at(index.row()).toString(QString("yyyy-M-d h:mm AP"));
        case 1:
            return QString().setNum(header_list.at(index.row()).sourceAddr);
        case 2:
            return QString().setNum(header_list.at(index.row()).rssi);
        case 3:
            return QString().setNum(header_list.at(index.row()).size);
        case 4:
            if (currentEncoding == ASCII)
                return QString(data_list.at(index.row()));
            else if (currentEncoding == HEX)
                return QString(data_list.at(index.row()).toHex()).toUpper();
        default:
            return QVariant();
        }
    }
    else
        return QVariant();
}

QVariant PacketTableModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole)
    {
        if (orientation == Qt::Horizontal)
        {
            switch(section) {
            case 0:
                return "Time";
            case 1:
                return "Source Address";
            case 2:
                return "RSSI";
            case 3:
                return "Length";
            case 4:
                return "Data";
            default: return "ERROR";
            }
        }
        else
        {
            return QString().setNum(section+1);
        }
    }
    else
        return QVariant();
}

void PacketTableModel::writeDataFile(QString filename)
{
    QFile dataFile(filename);
    dataFile.open(QIODevice::WriteOnly);
    QDataStream outStream(&dataFile);

    QByteArray dataString("Time,Source Address,RSSI,Length,Data\n");
    outStream.writeRawData(dataString.data(), dataString.length());

    for (int val = 0; val < header_list.length(); val++)
    {
        dataString = time_list[val].toString(QString("yyyy-M-d h:mm AP")).toUtf8();
        outStream.writeRawData(dataString.data(), dataString.length());
        outStream.writeRawData(",",1);

        dataString = QString().setNum(header_list[val].sourceAddr).toUtf8();
        outStream.writeRawData(dataString.data(), dataString.length());
        outStream.writeRawData(",",1);

        dataString = QString().setNum(header_list[val].rssi).toUtf8();
        outStream.writeRawData(dataString.data(), dataString.length());
        outStream.writeRawData(",",1);

        dataString = QString().setNum(header_list[val].size).toUtf8();
        outStream.writeRawData(dataString.data(), dataString.length());
        outStream.writeRawData(",",1);

        dataString = QString(data_list[val].toHex()).toUpper().toUtf8();
        outStream.writeRawData(dataString.data(), dataString.length());
        outStream.writeRawData("\n",1);
    }

    dataFile.close();
}
