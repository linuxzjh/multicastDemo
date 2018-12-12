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

#ifndef RECEIVER_H
#define RECEIVER_H

#include <QDialog>
#include <QHostAddress>

QT_BEGIN_NAMESPACE
class QTableWidget;
class QStackedWidget;
class QPushButton;
class QLabel;
class QComboBox;
class QUdpSocket;
class QTimer;
class QFile;
class QTextStream;
QT_END_NAMESPACE

typedef struct _senderInfo_stru
{
    int type;
    QString ip;
    QString hostName;
} senderInfo_stru;

class Receiver : public QDialog
{
    Q_OBJECT

public:
    Receiver(QWidget *parent = 0);
    ~Receiver();
private slots:
    //主设备
    void connectClicked();
    void hostReceiveProcessPendingDatagrams();

    //从设备
    void onStartBtnClicked();
    void onQuitCliked();
    void onTimeOut();
    void slaveReceiverProcessPendingDatagrams();

    //其他
    void onCurrentTypeChanged(int);
private:
    void initLogFile();
    void initUI();
    QWidget* addHostServicePage();
    QWidget* addSlaveServicePage();

    void initSocket();
    void addHostServiceSocket();
    void addSlaveServiceSocket();

    void resetUI();

    //主设备socket相关函数
    void setNetworkInterface();
    void parseData(const QByteArray& datagram);

    //从设备socket相关函数
    QString getIPAddress();
    QString getHostName();
    void    slaveServiceSendDatagram(const senderInfo_stru &info);

    //主窗口
    QLabel *typeSelectLabel;
    QComboBox *typeSelectComboBox;
    QStackedWidget *stackedWidget;

    //主设备页面
    QTableWidget *tableWidget;
    QPushButton *quitButton;
    QPushButton *connectButton;

    //从设备操作页面
    QLabel *senderLabel;
    QPushButton *startPushButton;
    QPushButton *quitPushButton;
    QTimer *timer;

    QHostAddress groupAddress;

    QUdpSocket *hostReceiverUdpSocket;
    QUdpSocket *hostSenderUdpSocket;

    QUdpSocket *slaveReceiverUdpSocket;
    QUdpSocket *slaveSenderUdpSocket;


    QFile *logFile;
    QTextStream out;
};

#endif
