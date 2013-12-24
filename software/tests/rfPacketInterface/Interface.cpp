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
//    qDebug()<<port->readAll();

    inbuffer.append(port->readAll());

    if (inbuffer.at(0) == 0)
        inbuffer = inbuffer.right(inbuffer.size() - 1);

    if (inbuffer.at(0) <= inbuffer.size()) {
        rxHeader_t packetHeader;
        memcpy(&packetHeader, inbuffer.data(), RX_HEADER_SIZE);
        inbuffer = inbuffer.right(inbuffer.size()-RX_HEADER_SIZE);
        QByteArray data = inbuffer.left(packetHeader.size - RX_HEADER_SIZE);
        inbuffer = inbuffer.right((inbuffer.size() - packetHeader.size + RX_HEADER_SIZE));

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
