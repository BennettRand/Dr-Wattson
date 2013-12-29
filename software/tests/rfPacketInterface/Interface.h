#ifndef INTERFACE_H
#define INTERFACE_H

#include <QWidget>
#include <QtSerialPort>
#include <QByteArray>
#include <QMessageBox>
#include <QDebug>
#include <QScrollBar>

#include <string.h>

#include "protocol.h"
#include "packettablemodel.h"

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

private:
    Ui::Widget *ui;
    QSerialPort *port;
    QByteArray inbuffer;
    QList<QSerialPortInfo> portList;
    PacketTableModel tableModel;

};

#endif // INTERFACE_H
