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

#include "netServer.h"
#include "utility.h"

#include <QtNetwork>
#include <QWebSocket>
#include <utility>
#include <utility>


/*!
 * \brief NetServer::NetServer The bas class for the Slides and spots File Servers
 * \param _serverName
 * \param _logFile
 * \param parent
 */
NetServer::NetServer(QString _serverName, QFile* _logFile, QObject *parent)
    : QObject(parent)
    , sServerName(std::move(_serverName))
    , logFile(_logFile)
{
    pServerSocket = nullptr;
}


/*!
 * \brief NetServer::prepareServer
 * This is the Base Class of all running Servers
 *
 * It creates the Server Socket and connects its SIGNALS to the rigth SLOTS
 * \param serverPort The port this Server listen for connections
 * \return
 */
bool
NetServer::prepareServer(quint16 serverPort) {
    pServerSocket = new QWebSocketServer(QStringLiteral("Server"),
                                         QWebSocketServer::NonSecureMode,
                                         this);
    connect(pServerSocket, SIGNAL(newConnection()),
            this, SLOT(onNewServerConnection()));
    connect(pServerSocket, SIGNAL(serverError(QWebSocketProtocol::CloseCode)),
            this, SLOT(onServerError(QWebSocketProtocol::CloseCode)));
    if (!pServerSocket->listen(QHostAddress::Any, serverPort)) {
        logMessage(logFile,
                   Q_FUNC_INFO,
                   QString("%1 - Impossibile to listen port %2 !")
                   .arg(sServerName)
                   .arg(serverPort));
        return false;
    }
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               QString("%1 - listening on port:%2")
               .arg(sServerName)
               .arg(serverPort));
#endif
    return true;
}


/*!
 * \brief NetServer::onServerError
 * \param closeCode
 */
void
NetServer::onServerError(QWebSocketProtocol::CloseCode closeCode){
    logMessage(logFile,
               Q_FUNC_INFO,
               QString("%1 - %2 Close code: %3")
               .arg(sServerName, pServerSocket->serverAddress().toString())
               .arg(closeCode));
    emit netServerError(closeCode);
}


/*!
 * \brief NetServer::onNewServerConnection
 */
void
NetServer::onNewServerConnection() {
    QWebSocket *pClient = pServerSocket->nextPendingConnection();
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               QString("%1 - Client %2 connected")
               .arg(sServerName, pClient->peerAddress().toString()));
#endif
    emit newConnection(pClient);
}


/*!
 * \brief NetServer::closeServer
 */
void
NetServer::closeServer() {
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               QString("Entering"));
#endif
    pServerSocket->disconnect();
    if(pServerSocket->isListening())
        pServerSocket->close();
    delete pServerSocket;
    pServerSocket = nullptr;
    thread()->quit();
}
