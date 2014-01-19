#ifndef INTERFACE_H
#define INTERFACE_H

#include <QWidget>
#include <QtSerialPort>
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QScrollBar>
#include <QFileDialog>
#include <QDir>

#include <string.h>

#define EXTENDED_DATA_PACKET

#include "../../hardware/firmware/baseStationFirmware/protocol.h"
#include "packettablemodel.h"
#include "../../hardware/firmware/remoteMonitorFirmware/protocol.h"

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private slots:
    void on_connectButton_clicked(bool checked);
    void on_recanPortsButton_clicked();
    void dataReceived();

    void on_transmitButton_clicked();

    void on_rxASCIIButton_toggled(bool checked);
    void on_tableView_clicked(const QModelIndex &index);

    void on_sendBaconPacket_clicked();

    void on_sendConnectionAck_clicked();

    void on_sendDataRrequest_clicked();

    void on_sendDataAck_clicked();

    void on_saveDataButton_clicked();

private:
    Ui::Widget *ui;
    QSerialPort *port;
    QByteArray inbuffer;
    QList<QSerialPortInfo> portList;
    PacketTableModel tableModel;
    void transmitRawData(void* data, uint8_t size, int32_t address, bool panBroadcast);

};

#endif // INTERFACE_H
