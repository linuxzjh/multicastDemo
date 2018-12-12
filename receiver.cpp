/****************************************************************************
**
** Copyright (C) 2016 The Qt Company Ltd.
** Contact: https://www.qt.io/licensing/
**
** This file is part of the examples of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:BSD$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and The Qt Company. For licensing terms
** and conditions see https://www.qt.io/terms-conditions. For further
** information use the contact form at https://www.qt.io/contact-us.
**
** BSD License Usage
** Alternatively, you may use this file under the terms of the BSD license
** as follows:
**
** "Redistribution and use in source and binary forms, with or without
** modification, are permitted provided that the following conditions are
** met:
**   * Redistributions of source code must retain the above copyright
**     notice, this list of conditions and the following disclaimer.
**   * Redistributions in binary form must reproduce the above copyright
**     notice, this list of conditions and the following disclaimer in
**     the documentation and/or other materials provided with the
**     distribution.
**   * Neither the name of The Qt Company Ltd nor the names of its
**     contributors may be used to endorse or promote products derived
**     from this software without specific prior written permission.
**
**
** THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
** "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
** LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
** A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
** OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
** SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
** LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
** DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
** THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
** (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
** OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE."
**
** $QT_END_LICENSE$
**
****************************************************************************/

#include <QtWidgets>
#include <QtNetwork>
#include <QHeaderView>

#include "receiver.h"

Receiver::Receiver(QWidget *parent)
    : QDialog(parent)
{
   initSocket();
   initUI();
}

void Receiver::initUI()
{

    typeSelectLabel = new QLabel(tr("type:"), this);
    typeSelectComboBox = new QComboBox(this);

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch(1);
    hlayout->addWidget(typeSelectLabel);
    hlayout->addWidget(typeSelectComboBox);
    hlayout->addStretch(1);

    QStringList strList;
    strList << tr("Host Service") <<tr("Slave Service");
    typeSelectComboBox->addItems(strList);

    connect(typeSelectComboBox, SIGNAL(currentIndexChanged(int)), this, SLOT(onCurrentTypeChanged(int)));

    stackedWidget = new QStackedWidget(this);
    QWidget *hostServicePage = addHostServicePage();
    QWidget *slaveServicePage = addSlaveServicePage();

    stackedWidget->addWidget(hostServicePage);
    stackedWidget->addWidget(slaveServicePage);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addLayout(hlayout);
    mainLayout->addWidget(stackedWidget);
    setLayout(mainLayout);

     setWindowTitle(tr("Multicast Receiver"));
}

QWidget *Receiver::addHostServicePage()
{
    QWidget *hostServicePage = new QWidget();

    tableWidget = new QTableWidget(hostServicePage);
    quitButton = new QPushButton(tr("&Quit"), hostServicePage);
    connectButton = new QPushButton(tr("&Connect"), hostServicePage);

     connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
     connect(connectButton, SIGNAL(clicked()), this, SLOT(connectClicked()));

     QHBoxLayout *buttonLayout = new QHBoxLayout;
     buttonLayout->addStretch(1);
     buttonLayout->addWidget(connectButton);
     buttonLayout->addSpacing(20);
     buttonLayout->addWidget(quitButton);
     buttonLayout->addStretch(1);

     QVBoxLayout *mainLayout = new QVBoxLayout;
     mainLayout->addWidget(tableWidget);
     mainLayout->addLayout(buttonLayout);
     hostServicePage->setLayout(mainLayout);

     {
         //初始化TableWidget
         QStringList hHeaderStrList;
         hHeaderStrList << tr("select") << tr("IP") << tr("HostName");
         tableWidget->setHorizontalHeaderLabels(hHeaderStrList);
         tableWidget->setColumnCount(hHeaderStrList.count());
         QFont font = tableWidget->horizontalHeader()->font();
         font.setBold(true);

         tableWidget->horizontalHeader()->resizeSection(0,20);
         tableWidget->horizontalHeader()->setFixedHeight(25);
         tableWidget->horizontalHeader()->setStretchLastSection(true); //设置充满表宽度
         tableWidget->horizontalHeader()->setStyleSheet("QHeaderView::section{background:skyblue;}"); //设置表头背景色

         tableWidget->verticalHeader()->setSectionResizeMode(QHeaderView::ResizeToContents);
         tableWidget->verticalHeader()->setDefaultSectionSize(30); //设置行高
         tableWidget->verticalHeader()->setVisible(false); //设置垂直头不可见

         tableWidget->setFrameShape(QFrame::NoFrame); //设置无边框
         tableWidget->setStyleSheet("selection-background-color:lightblue;"); //设置选中背景色
         tableWidget->setShowGrid(false);
         tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
     }

    return hostServicePage;
}

QWidget *Receiver::addSlaveServicePage()
{
    QWidget *slaveServicePage = new QWidget();

    senderLabel = new QLabel(tr("Clike Start Button, Sender Service Infomation!"));
    startPushButton = new QPushButton(tr("Start"));
    quitPushButton = new QPushButton(tr("Quit"));

    connect(startPushButton, SIGNAL(clicked()), this, SLOT(onStartBtnClicked()));
    connect(quitPushButton, SIGNAL(clicked()), this, SLOT(onQuitCliked()));

    QHBoxLayout *hlayout = new QHBoxLayout;
    hlayout->addStretch(1);
    hlayout->addWidget(startPushButton);
    hlayout->addWidget(quitPushButton);
    hlayout->addStretch(1);

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(senderLabel);
    mainLayout->addLayout(hlayout);

    slaveServicePage->setLayout(mainLayout);

    return slaveServicePage;
}

void Receiver::initSocket()
{
    addHostServiceSocket();
    addSlaveServiceSocket();
}

void Receiver::addHostServiceSocket()
{
    groupAddress = QHostAddress("238.255.43.21");

    hostReceiverUdpSocket = new QUdpSocket(this);
    hostReceiverUdpSocket->bind(QHostAddress::AnyIPv4, 45454, QUdpSocket::ShareAddress);
    hostReceiverUdpSocket->joinMulticastGroup(groupAddress);
    setNetworkInterface();

   connect(hostReceiverUdpSocket, SIGNAL(readyRead()),
            this, SLOT(hostReceiveProcessPendingDatagrams()));

   hostSenderUdpSocket = new QUdpSocket(this);
}

void Receiver::addSlaveServiceSocket()
{
    timer = new QTimer(this);
    connect(timer, SIGNAL(timeout()), this, SLOT(onTimeOut()));

    slaveReceiverUdpSocket = new QUdpSocket(this);
    slaveReceiverUdpSocket->bind(QHostAddress::AnyIPv4, 50000, QUdpSocket::ShareAddress);
    connect(slaveReceiverUdpSocket, SIGNAL(readyRead()),
             this, SLOT(slaveReceiverProcessPendingDatagrams()));

    slaveSenderUdpSocket = new QUdpSocket(this);
}

void Receiver::resetUI()
{
    tableWidget->clear();
    timer->stop();
    startPushButton->setEnabled(true);
}

void Receiver::hostReceiveProcessPendingDatagrams()
{
    while (hostReceiverUdpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(hostReceiverUdpSocket->pendingDatagramSize());
        hostReceiverUdpSocket->readDatagram(datagram.data(), datagram.size());
        parseData(datagram);
    }
}

void Receiver::connectClicked()
{
    for (int row = 0; row < tableWidget->rowCount(); ++row)
    {
        int checkeState = tableWidget->item(row, 0)->checkState();
        if (Qt::Checked == checkeState)
        {
            QString ip = tableWidget->item(row, 1)->text();

            QString senderInfo = "1";
            QByteArray datagram = senderInfo.toLocal8Bit();
            hostSenderUdpSocket->writeDatagram(datagram.data(), datagram.size(),
                                     QHostAddress(ip), 50000);
        }
    }
}

void Receiver::onCurrentTypeChanged(int index)
{
    resetUI();

    stackedWidget->setCurrentIndex(index);

    switch (index) {
    case 0:
    {
        //主设备
        hostReceiverUdpSocket->joinMulticastGroup(groupAddress);
        connect(hostReceiverUdpSocket, SIGNAL(readyRead()),
                 this, SLOT(hostReceiveProcessPendingDatagrams()));
        disconnect(slaveReceiverUdpSocket, SIGNAL(readyRead()),
                 this, SLOT(slaveReceiverProcessPendingDatagrams()));
        break;
    }
    case 1:
    {
        //从设备
        hostReceiverUdpSocket->leaveMulticastGroup(groupAddress);
        disconnect(hostReceiverUdpSocket, SIGNAL(readyRead()),
                 this, SLOT(hostReceiveProcessPendingDatagrams()));
        connect(slaveReceiverUdpSocket, SIGNAL(readyRead()),
                 this, SLOT(slaveReceiverProcessPendingDatagrams()));
        break;
    }
    default:
        break;
    }
}

void Receiver::onStartBtnClicked()
{
    startPushButton->setEnabled(false);
    timer->start(1000);
}

void Receiver::onQuitCliked()
{
    senderInfo_stru info;
    info.type = 0;
    info.hostName = getHostName();
    info.ip = getIPAddress();

    slaveServiceSendDatagram(info);

    startPushButton->setEnabled(true);
    timer->stop();
    senderLabel->setText(tr("Clike Start Button, Sender Service Infomation!"));
}

void Receiver::onTimeOut()
{
    senderInfo_stru info;
    info.type = 1;
    info.hostName = getHostName();
    info.ip = getIPAddress();

    slaveServiceSendDatagram(info);
}

void Receiver::slaveReceiverProcessPendingDatagrams()
{
    while (slaveReceiverUdpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(slaveReceiverUdpSocket->pendingDatagramSize());
        slaveReceiverUdpSocket->readDatagram(datagram.data(), datagram.size());
        senderLabel->setText("selected!!!!");
    }
}

void Receiver::slaveServiceSendDatagram(const senderInfo_stru &info)
{
    QString printInfo;
    printInfo = QString("type:%1\n hostName:%2\n IP:%3\n").arg(info.type ? "normal" : "close").arg(info.hostName).arg(info.ip);
    senderLabel->setText(printInfo);

    QStringList senderInfo;
    char split = ';';
    senderInfo << QString::number(info.type) << info.hostName << info.ip << "$$$";

    QByteArray datagram = senderInfo.join(split).toLocal8Bit();
    slaveSenderUdpSocket->writeDatagram(datagram.data(), datagram.size(),
                             groupAddress, 45454);
}

void Receiver::setNetworkInterface()
{
    QList<QNetworkInterface> networkInterfaces = QNetworkInterface::allInterfaces();
    for (QNetworkInterface netWorkInterface : networkInterfaces)
    {
        QList<QHostAddress> ipAddressesList = netWorkInterface.allAddresses();
        for (int i = 0; i < ipAddressesList.size(); ++i)
        {
            if (ipAddressesList.at(i) == QHostAddress::LocalHost &&  ipAddressesList.at(i).toIPv4Address())
            {
                hostReceiverUdpSocket->setMulticastInterface(netWorkInterface);
                break;
            }
        }
    }
}

void Receiver::parseData(const QByteArray &datagram)
{
    QString data = QString::fromLocal8Bit(datagram);
    QStringList strList = data.split(';');
    if (strList.count() == 4 && strList.at(3) == "$$$")
    {
        QString typeStr = strList.at(0);
        switch (typeStr.toInt()) {
        case 0:
        {
            //QString hostName = strList.at(1);
            QString ip = strList.at(2);
            QList<QTableWidgetItem *> items = tableWidget->findItems(ip, Qt::MatchFixedString);
            if (items.count() > 0)
            {
                Q_ASSERT(items.count() == 1);
                tableWidget->removeRow(items[0]->row());
            }
            break;
        }
        case 1:
        {
            QString hostName = strList.at(1);
            QString ip = strList.at(2);
            QList<QTableWidgetItem *> items = tableWidget->findItems(ip, Qt::MatchFixedString);
            if (0 == items.count())
            {
                int rowCount = tableWidget->rowCount();
                tableWidget->insertRow(rowCount);

                QTableWidgetItem *checkedItem = new QTableWidgetItem();
                QTableWidgetItem *ipItem = new QTableWidgetItem(ip);
                QTableWidgetItem *hostNameItem = new QTableWidgetItem(hostName);

                tableWidget->setItem(rowCount, 0, checkedItem);
                tableWidget->setItem(rowCount, 1, ipItem);
                tableWidget->setItem(rowCount, 2, hostNameItem);

                checkedItem->setCheckState(Qt::Checked);
            }
            break;
        }
        default:
            break;
        }
    }
}

QString Receiver::getIPAddress()
{
    QString ipAddress;
    QList<QHostAddress> ipAddressesList = QNetworkInterface::allAddresses();
    for (int i = 0; i < ipAddressesList.size(); ++i)
    {
        if (ipAddressesList.at(i) != QHostAddress::LocalHost &&  ipAddressesList.at(i).toIPv4Address())
        {
            ipAddress = ipAddressesList.at(i).toString();
            break;
        }
    }
    if (ipAddress.isEmpty())
        ipAddress = QHostAddress(QHostAddress::LocalHost).toString();
    return ipAddress;
}

QString Receiver::getHostName()
{
    return QHostInfo::localHostName();
}
