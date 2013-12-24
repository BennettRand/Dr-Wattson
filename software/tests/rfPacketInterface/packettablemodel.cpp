#include "packettablemodel.h"

PacketTableModel::PacketTableModel(QObject *parent) :
    QAbstractTableModel(parent)
{
}

void PacketTableModel::addData(rxHeader_t header, QByteArray data)
{
    beginInsertRows(QModelIndex(),header_list.size(),header_list.size());
    header_list.append(header);
    data_list.append(data);
    endInsertRows();
}

int PacketTableModel::rowCount(const QModelIndex &parent) const
{
    return header_list.size();
}

int PacketTableModel::columnCount(const QModelIndex &parent) const
{
    return 4;
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
            return QString().setNum(header_list.at(index.row()).sourceAddr);
        case 1:
            return QString().setNum(header_list.at(index.row()).rssi);
        case 2:
            return QString().setNum(header_list.at(index.row()).size);
        case 3:
            return QString(data_list.at(index.row()));
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
                return "Source Address";
            case 1:
                return "RSSI";
            case 2:
                return "Length";
            case 3:
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
