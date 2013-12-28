#include "Interface.h"
#include "ui_widget.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);

    portList = QSerialPortInfo::availablePorts();
    for (int i = 0; i < portList.length(); i++) // popluate the port list
        ui->portCombo->addItem(portList.at(i).portName());

    port = new QSerialPort(this);

    connect(port,SIGNAL(readyRead()),this,SLOT(dataReceived()));

    ui->tableView->setModel(&tableModel);
}

Widget::~Widget()
{
    delete ui;
}

void Widget::dataReceived()
{
    inbuffer.append(port->readAll());

    if (inbuffer.at(0) == 0)
        inbuffer = inbuffer.right(inbuffer.size() - 1);

    if ((inbuffer.at(0) + sizeof(rxHeader_t)) <= inbuffer.size()) {
        rxHeader_t packetHeader;
        memcpy(&packetHeader, inbuffer.data(), sizeof(rxHeader_t));
        inbuffer = inbuffer.right(inbuffer.size() - sizeof(rxHeader_t));
        QByteArray data = inbuffer.left(packetHeader.size);
        inbuffer = inbuffer.right(inbuffer.size() - packetHeader.size);

        tableModel.addData(packetHeader, data);
        ui->tableView->scrollToBottom();
    }
}

void Widget::on_connectButton_clicked(bool checked)
{
    if (checked)
    {
        port->setPort(portList.at(ui->portCombo->currentIndex()));
        if (port->open(QIODevice::ReadWrite))
        {
            port->setBaudRate(QSerialPort::Baud38400);
            port->setDataBits(QSerialPort::Data8);
            port->setDataErrorPolicy(QSerialPort::IgnorePolicy);
            port->setFlowControl(QSerialPort::NoFlowControl);
            port->setParity(QSerialPort::NoParity);
            port->setStopBits(QSerialPort::OneStop);
            ui->connectButton->setText("Close Port");
        }
        else
        {
            ui->connectButton->setText("Open Port");
            ui->connectButton->setChecked(false);
            QMessageBox::warning(this,"Error","Cound not open selected port");
        }
    }
    else
    {
        port->close();
        ui->connectButton->setText("Open Port");
    }
}

void Widget::on_recanPortsButton_clicked()
{
    ui->portCombo->clear();
    portList = QSerialPortInfo::availablePorts();
    for (int i = 0; i < portList.length(); i++) // popluate the port list
        ui->portCombo->addItem(portList.at(i).portName());
}

void Widget::on_transmitButton_clicked()
{
    txHeader_t packetHeader;
    packetHeader.destAddr = ui->addressEdit->text().toInt(0,16);

    if (ui->asciiRadioButton->isChecked()) {
        packetHeader.size = ui->dataEdit->toPlainText().length();
        port->write((char*)(&packetHeader), sizeof(packetHeader));
        port->write(ui->dataEdit->toPlainText().toLocal8Bit());
    }
    else if (ui->hexRadioButton->isChecked()) {
        QString str = ui->dataEdit->toPlainText().toUpper().remove(QRegExp("[^0123456789ABCDEF]"));
        QByteArray bytes;
        while (str.length() > 0)
        {
            bytes.append(str.left(2).toInt(0,16));
            str = str.right((str.length()-2) > 0 ? str.length()-2 : 0);
        }
        packetHeader.size = bytes.size();
        port->write((char*)(&packetHeader), sizeof(txHeader_t));
        port->write(bytes);
    }
}
