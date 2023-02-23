/*
 *
Copyright (C) 2016  Gabriele Salvato

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*
*/

#ifndef NETSERVER_H
#define NETSERVER_H

#include <QObject>
#include <QUrl>
#include <QWebSocketServer>
#include <QFile>

class NetServer : public QObject
{
    Q_OBJECT
public:
    explicit NetServer(QString myServerName, QFile* myLogFile = nullptr, QObject *parent = nullptr);

    bool prepareServer(quint16 serverPort);
    void closeServer();

signals:
    void newConnection(QWebSocket *);
    void netServerError(QWebSocketProtocol::CloseCode);

private slots:
    void onNewServerConnection();
    void onServerError(QWebSocketProtocol::CloseCode closeCode);

protected:
    QString           sServerName;
    QFile            *logFile;
    QWebSocketServer *pServerSocket;

private:
    QDateTime         dateTime;
};

#endif // NETSERVER_H
