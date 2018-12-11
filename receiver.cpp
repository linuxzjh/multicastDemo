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
    groupAddress = QHostAddress("238.255.43.21");

    udpSocket = new QUdpSocket(this);
    udpSocket->bind(QHostAddress::AnyIPv4, 45454, QUdpSocket::ShareAddress);
    udpSocket->joinMulticastGroup(groupAddress);
    setNetworkInterface();

   connect(udpSocket, SIGNAL(readyRead()),
            this, SLOT(processPendingDatagrams()));

   initUI();
}

void Receiver::initUI()
{
    tableWidget = new QTableWidget(this);
    quitButton = new QPushButton(tr("&Quit"));
    okButton = new QPushButton(tr("&Ok"));

     connect(quitButton, SIGNAL(clicked()), this, SLOT(close()));
     connect(okButton, SIGNAL(clicked()), this, SLOT(okClicked()));

     QHBoxLayout *buttonLayout = new QHBoxLayout;
     buttonLayout->addStretch(1);
     buttonLayout->addWidget(okButton);
     buttonLayout->addSpacing(20);
     buttonLayout->addWidget(quitButton);
     buttonLayout->addStretch(1);

     QVBoxLayout *mainLayout = new QVBoxLayout;
     mainLayout->addWidget(tableWidget);
     mainLayout->addLayout(buttonLayout);
     setLayout(mainLayout);

     setWindowTitle(tr("Multicast Receiver"));

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

void Receiver::processPendingDatagrams()
{
    while (udpSocket->hasPendingDatagrams()) {
        QByteArray datagram;
        datagram.resize(udpSocket->pendingDatagramSize());
        udpSocket->readDatagram(datagram.data(), datagram.size());
        parseData(datagram);
    }
}

void Receiver::okClicked()
{
    for (int row = 0; row < tableWidget->rowCount(); ++row)
    {
        int checkeState = tableWidget->item(row, 0)->checkState();
        if (Qt::Checked == checkeState)
        {
            QString ip = tableWidget->item(row, 1)->text();

            QString senderInfo = "1";
            QByteArray datagram = senderInfo.toLocal8Bit();
            udpSocket->writeDatagram(datagram.data(), datagram.size(),
                                     QHostAddress(ip), 50000);
        }
    }
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
                udpSocket->setMulticastInterface(netWorkInterface);
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
