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
    ui->tableView->setColumnWidth(0, 140);
    ui->tableView->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);

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

        tableModel.addData(packetHeader, data, QDateTime::currentDateTime());
        ui->tableView->scrollToBottom();

        if (ui->followDataCheckbox->isChecked())
        {
            ui->tableView->selectRow(tableModel.rowCount(QModelIndex())-1);
            on_tableView_clicked(tableModel.index(tableModel.rowCount(QModelIndex())-1,4));
        }

        if (ui->autoRespCheckBox->isChecked())
        {
            txHeader_t txPacketHeader;
            txPacketHeader.destAddr = packetHeader.sourceAddr;
            txPacketHeader.command = sendPacket;
            txPacketHeader.size = 13 + packetHeader.size;
            port->write((char*)(&txPacketHeader), sizeof(txPacketHeader));
            port->write("Got message: ", 13);
            port->write(data);
        }

        if ((ui->autoAckDataCheckbox->isChecked()) && (data[0] == 4) && (data.length() == sizeof(dataPacket_t))) {
            dataPacket_t *pkt = (dataPacket_t*)data.data();
            ui->dataAckSeqNumber->setValue(pkt->dataSequence);
            ui->sendDataAck->clicked();
        }
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
            uint16_t pan_id = ui->panLineEdit->text().toInt(0,16);
            txHeader_t packetHeader;
            packetHeader.destAddr = pan_id;
            packetHeader.command = setPAN;
            packetHeader.size = 0;

            port->write((char*)(&packetHeader), sizeof(txHeader_t));
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

    if (ui->panBroadcastCheckbox->isChecked())
        packetHeader.command = broadcastPacket;
    else
        packetHeader.command = sendPacket;

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


void Widget::on_rxASCIIButton_toggled(bool checked)
{
    if (checked)
        tableModel.encodingChanged(PacketTableModel::ASCII);
    else
        tableModel.encodingChanged(PacketTableModel::HEX);
}

void Widget::on_tableView_clicked(const QModelIndex &index)
{
    QByteArray selData = tableModel.getRawData(index.row());

    ui->packetDataEdit->clear();

    if ((selData[0] == bacon) && (selData.size() == sizeof(baconPacket_t))) {
        ui->packetDataEdit->appendPlainText("Beacon Packet (0x00)");
        baconPacket_t *pkt = (baconPacket_t*)selData.data();
        ui->packetDataEdit->appendPlainText(QString("PAN ID: ").append(QString().setNum(pkt->PAN_ID)));
        ui->packetDataEdit->appendPlainText(QString("Network Name: \"").append(QString(QByteArray().setRawData(pkt->name,16))).append("\""));
    }
    else if ((selData[0] == connectionRequest) && (selData.size() == sizeof(connectionRequestPacket_t))) {
        ui->packetDataEdit->appendPlainText("Connection Request Packet (0x01)");
        connectionRequestPacket_t *pkt = (connectionRequestPacket_t*)selData.data();
        ui->packetDataEdit->appendPlainText(QString("Raw Voltage Scaling 1: ").append(QString().setNum(pkt->channel1VoltageScaling)));
        ui->packetDataEdit->appendPlainText(QString("Raw Current Scaling 1: ").append(QString().setNum(pkt->channel1CurrentScaling)));
        ui->packetDataEdit->appendPlainText(QString("Raw Voltage Scaling 2: ").append(QString().setNum(pkt->channel2VoltageScaling)));
        ui->packetDataEdit->appendPlainText(QString("Raw Current Scaling 2: ").append(QString().setNum(pkt->channel2CurrentScaling)));
        ui->packetDataEdit->appendPlainText(QString("Raw Line Period Scaling: ").append(QString().setNum(pkt->linePeriodScalingFactor)));
        ui->packetDataEdit->appendPlainText("");

        ui->packetDataEdit->appendPlainText(QString("Voltage Scaling 1: ").append(QString().setNum(pkt->channel1VoltageScaling * 0.0000001).append(" Volts/Count")));
        ui->packetDataEdit->appendPlainText(QString("Current Scaling 1: ").append(QString().setNum(pkt->channel1CurrentScaling * 0.0000001).append(" Amps/Count")));
        ui->packetDataEdit->appendPlainText(QString("Voltage Scaling 2: ").append(QString().setNum(pkt->channel2VoltageScaling * 0.0000001).append(" Volts/Count")));
        ui->packetDataEdit->appendPlainText(QString("Current Scaling 2: ").append(QString().setNum(pkt->channel2CurrentScaling * 0.0000001).append(" Amps/Count")));
        ui->packetDataEdit->appendPlainText(QString("Line Period Scaling: ").append(QString().setNum(pkt->linePeriodScalingFactor * 0.000001).append(" Seconds/Count")));

        ui->voltage1ScalingSpinbox->setValue(pkt->channel1VoltageScaling * 0.0000001);
        ui->current1ScalingSpinbox->setValue(pkt->channel1CurrentScaling * 0.0000001);
        ui->voltage2ScalingSpinbox->setValue(pkt->channel2VoltageScaling * 0.0000001);
        ui->current2ScalingSpinbox->setValue(pkt->channel2CurrentScaling * 0.0000001);
        ui->linePeriodScalingSpinbox->setValue(pkt->linePeriodScalingFactor * 0.000000001);

    }
    else if ((selData[0] == connectionAck) && (selData.size() == sizeof(connectionAckPacket_t))) {
        ui->packetDataEdit->appendPlainText("Connection Ack Packet (0x02)");
    }
    else if ((selData[0] == dataRequest) && (selData.size() == sizeof(dataRequestPacket_t))) {
        ui->packetDataEdit->appendPlainText("Data Request Packet (0x03)");
        ui->packetDataEdit->appendPlainText(QString("Request Sequence Number: ").append(QString().setNum(selData[1])));
    }
    else if ((selData[0] == 4) && (selData.size() == sizeof(dataPacket_t))) {
        ui->packetDataEdit->appendPlainText("Data Packet (0x04)");
        dataPacket_t *pkt = (dataPacket_t*)selData.data();

        ui->packetDataEdit->appendPlainText(QString("Request Sequence Number: ").append(QString().setNum(pkt->requestSequence)));
        ui->packetDataEdit->appendPlainText(QString("Data Sequence Number: ").append(QString().setNum(pkt->dataSequence)));
        ui->packetDataEdit->appendPlainText(QString("Sample Count: ").append(QString().setNum(pkt->sampleCount)));
        ui->packetDataEdit->appendPlainText(QString("Raw Power Value: ").append(QString().setNum(pkt->powerData)));
        ui->packetDataEdit->appendPlainText(QString("Raw Line Period: ").append(QString().setNum(pkt->linePeriod)));
        ui->packetDataEdit->appendPlainText(QString("Raw Voltage Value: ").append(QString().setNum(pkt->squaredVoltage)));
        ui->packetDataEdit->appendPlainText(QString("Raw Current Value: ").append(QString().setNum(pkt->squaredCurrent)));

        ui->packetDataEdit->appendPlainText("");
        ui->packetDataEdit->appendPlainText(QString("Power: ").append(QString().setNum((((double)pkt->powerData)/pkt->sampleCount)*ui->voltage1ScalingSpinbox->value()*ui->current1ScalingSpinbox->value())));
        ui->packetDataEdit->appendPlainText(QString("Voltage (Vrms): ").append(QString().setNum(sqrt((((double)pkt->squaredVoltage)/pkt->sampleCount))*ui->voltage1ScalingSpinbox->value())));
        ui->packetDataEdit->appendPlainText(QString("Current (Arms): ").append(QString().setNum(sqrt((((double)pkt->squaredCurrent)/pkt->sampleCount))*ui->current1ScalingSpinbox->value())));
        ui->packetDataEdit->appendPlainText(QString("Line Frequency (Hz): ").append(QString().setNum(1.0/(((double)pkt->linePeriod) * ui->linePeriodScalingSpinbox->value()))));

        ui->dataAckSeqNumber->setValue(pkt->dataSequence);

    }
    else if ((selData[0] == dataAck) && (selData.size() == sizeof(dataAckPacket_t))) {
        ui->packetDataEdit->appendPlainText("Data Ack Packet (0x05)");
        ui->packetDataEdit->appendPlainText(QString("Data Sequence Number: ").append(QString().setNum(selData[1])));
    }
}

void Widget::on_sendBaconPacket_clicked()
{
    baconPacket_t pkt;
    pkt.type = bacon;
    pkt.PAN_ID = ui->panLineEdit->text().toInt();
    QByteArray name = ui->networkNameEdit->text().toUtf8();
    while (name.length() < 16)
    {
        name.append(' ');
    }
    memcpy(pkt.name, name.data(), 16);
    transmitRawData(&pkt, sizeof(pkt), 0xFFFF, true);
}

void Widget::transmitRawData(void* data, uint8_t size, int32_t address, bool panBroadcast)
{
    QString outData;
    for (int i = 0; i < size; i++)
        outData.append(QString("%1 ").arg(((uint8_t*)data)[i],2,16,QChar('0')));

    if ((address >= 0) && (address <= 0xFFFF))
        ui->addressEdit->setText(QString().setNum(address,16).toUpper());
    ui->panBroadcastCheckbox->setChecked(panBroadcast);
    ui->dataEdit->setPlainText(outData);
    ui->hexRadioButton->setChecked(true);
    ui->transmitButton->clicked();
}

void Widget::on_sendConnectionAck_clicked()
{
    connectionAckPacket_t pkt;
    pkt.type = connectionAck;
    transmitRawData(&pkt, sizeof(pkt), -1, false);
}

void Widget::on_sendDataRrequest_clicked()
{
    dataRequestPacket_t pkt;
    pkt.type = dataRequest;
    pkt.requestSequence = ui->dataReqSeqNumber->value();
    if (pkt.requestSequence != 255)
        ui->dataReqSeqNumber->setValue(pkt.requestSequence+1);
    else
        ui->dataReqSeqNumber->setValue(0);
    transmitRawData(&pkt, sizeof(pkt), -1, false);
}

void Widget::on_sendDataAck_clicked()
{
    dataAckPacket_t pkt;
    pkt.type = dataAck;
    pkt.dataSequence = ui->dataAckSeqNumber->value();
    transmitRawData(&pkt, sizeof(pkt), -1, false);
}

void Widget::on_saveDataButton_clicked()
{
    QString filename = QFileDialog::getSaveFileName(this,"Save Data",QDir::homePath());
    tableModel.writeDataFile(filename);
}
