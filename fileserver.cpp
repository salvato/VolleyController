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
#include "fileserver.h"
#include "utility.h"

#include <QFile>
#include <QDir>
#include <QThread>
#include <QWebSocket>
#include <QImage>
#include <QBuffer>
#include <utility>


/*!
 * \brief FileServer::FileServer It implements a Server for Slides or Spots file transfer
 * \param sName A string distinguishing this server (used for logging)
 * \param myLogFile The File for message logging (if any)
 * \param parent
 */
FileServer::FileServer(const QString& sName, QFile* myLogFile, QObject *parent)
    : NetServer(sName, myLogFile, parent)
    , serverName(sName)
{
    port      = 0;
    sFileDir  = QString();
    fileList  = QList<QFileInfo>();
    connections.clear();
}


/*!
 * \brief FileServer::setServerPort
 * \param myPort
 */
void
FileServer::setServerPort(quint16 myPort) {
    port = myPort;
}


/*!
 * \brief FileServer::setDir To set the destination directory
 * \param sDirectory The selected directory
 * \param sExtensions The file extensions it manipulate
 * \return true if the directory can be used
 */
bool
FileServer::setDir(QString sDirectory, const QString& sExtensions) {
    sFileDir = std::move(sDirectory);
    if(!sFileDir.endsWith(QString("/")))  sFileDir+= QString("/");

    QDir sDir(sFileDir);
    if(sDir.exists()) {
        QStringList nameFilter(sExtensions.split(" "));
        sDir.setNameFilters(nameFilter);
        sDir.setFilter(QDir::Files);
        fileList = sDir.entryInfoList();
    }
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               serverName +
               QString(" Found %1 files in %2")
               .arg(fileList.count())
               .arg(sFileDir));
#endif
    return true;
}


/*!
 * \brief FileServer::onStartServer Invoked to start listening for connections
 */
void
FileServer::onStartServer() {
    if(port == 0) {
        logMessage(logFile,
                   Q_FUNC_INFO,
                   serverName +
                   QString(" Error! Server port not set."));
        emit fileServerDone(true);// Close with errors
        return;
    }

    prepareServer(port);

    connect(this, SIGNAL(newConnection(QWebSocket*)),
            this, SLOT(onNewConnection(QWebSocket*)));
    connect(this, SIGNAL(netServerError(QWebSocketProtocol::CloseCode)),
            this, SLOT(onFileServerError(QWebSocketProtocol::CloseCode)));
}


/*!
 * \brief FileServer::onFileServerError
 */
void
FileServer::onFileServerError(QWebSocketProtocol::CloseCode code) {
    Q_UNUSED(code)
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               QString("code=%1").arg(code));
#endif
    for(int i=0; i<connections.count(); i++) {
        if(connections.at(i)) {
            if(connections.at(i)->isValid())
                connections.at(i)->close();
            connections.at(i)->disconnect();
            delete connections.at(i);
        }
    }
    connections.clear();
    emit fileServerDone(true);// Close File Server with errors !
}


/*!
 * \brief FileServer::onNewConnection
 * Invoked upon a new connection has been detected
 * \param pClient The websocket of the connected client
 */
void
FileServer::onNewConnection(QWebSocket *pClient) {
    int nConnections = connections.count();
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               serverName +
               QString("Connection requests from %1")
               .arg(pClient->peerAddress().toString()));
#endif
    for(int i=nConnections-1; i>=0; i--) {
        if(connections.at(i)->peerAddress() == pClient->peerAddress()) {
            logMessage(logFile,
                       Q_FUNC_INFO,
                       serverName +
                       QString("Duplicate requests from %1")
                       .arg(pClient->peerAddress().toString()));
            if(connections.at(i)->isValid() && pClient->isValid()) {
                logMessage(logFile,
                           Q_FUNC_INFO,
                           serverName +
                           QString(" Both sockets are valid! Removing the old connection"));
                connections.at(i)->disconnect();
                connections.at(i)->abort();
                delete connections.at(i);
                connections.removeAt(i);
                break;
            }
            if(pClient->isValid()) {
                logMessage(logFile,
                           Q_FUNC_INFO,
                           serverName +
                           QString(" Only present socket is valid. Removing the old one"));
                connections.at(i)->disconnect();
                connections.at(i)->abort();
                delete connections.at(i);
                connections.removeAt(i);
            }
            else {
                logMessage(logFile,
                           Q_FUNC_INFO,
                           serverName +
                           QString(" Present socket is not valid."));
                pClient->disconnect();
                pClient->close(QWebSocketProtocol::CloseCodeNormal,
                               QString("Duplicated request"));
                delete pClient;
                pClient = nullptr;
                return;
            }
            break;
        }// if(connections.at(i)->peerAddress() == pClient->peerAddress())
    }// for(int i=nConnections-1; i>=0; i--)
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               serverName +
               QString(" Client connected: %1")
               .arg(pClient->peerAddress().toString()));
#endif
    connections.append(pClient);

    connect(pClient, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onProcessTextMessage(QString)));
    connect(pClient, SIGNAL(binaryMessageReceived(QByteArray)),
            this, SLOT(onProcessBinaryMessage(QByteArray)));
    connect(pClient, SIGNAL(disconnected()),
            this, SLOT(onClientDisconnected()));
    connect(pClient, SIGNAL(error(QAbstractSocket::SocketError)),
            this, SLOT(onClientSocketError(QAbstractSocket::SocketError)));
}


/*!
 * \brief FileServer::onClientSocketError
 * \param error
 */
void
FileServer::onClientSocketError(QAbstractSocket::SocketError error) {
    Q_UNUSED(error)
    auto *pClient = qobject_cast<QWebSocket *>(sender());
    logMessage(logFile,
               Q_FUNC_INFO,
               serverName +
               QString(" %1 Error %2 %3")
               .arg(pClient->peerAddress().toString())
               .arg(error)
               .arg(pClient->errorString()));
    pClient->disconnect();
    pClient->abort();
    if(!connections.removeOne(pClient)) {
        logMessage(logFile,
                   Q_FUNC_INFO,
                   serverName +
                   QString(" Unable to remove %1 from list !")
                   .arg(pClient->peerAddress().toString()));
    }
    delete pClient;
    pClient = nullptr;
}


/*!
 * \brief FileServer::onProcessTextMessage
 * Process the requests from the clients
 * \param sMessage
 */
void
FileServer::onProcessTextMessage(QString sMessage) {
    QString sNoData = QString("NoData");
    QString sToken;

    // The pointer is valid only during the execution of the slot
    // that calls this function from this object's thread context.
    auto *pClient = qobject_cast<QWebSocket *>(sender());

    sToken = XML_Parse(sMessage, "get");
    if(sToken != sNoData) {
        QStringList argumentList = sToken.split(",");
        if(argumentList.count() < 3) {
            logMessage(logFile,
                       Q_FUNC_INFO,
                       QString("Bad formatted requests: %1")
                       .arg(sToken));
            return;
        }
        const QString& sFileName = argumentList.at(0);
        qint64 startPos = argumentList.at(1).toInt();
        qint64 length   = argumentList.at(2).toInt();
#ifdef LOG_VERBOSE
        logMessage(logFile,
                   Q_FUNC_INFO,
                   QString("%1 asked for: %2 %3 bytes, starting form %4 ")
                   .arg(pClient->peerAddress().toString(), sFileName)
                   .arg(length)
                   .arg(startPos));
#endif
        QFile file;
        QString sFilePath = sFileDir + sFileName;
        file.setFileName(sFilePath);
        if(file.exists()) {
            qint64 filesize = file.size();
            if(filesize <= startPos) {
                logMessage(logFile,
                           Q_FUNC_INFO,
                           QString("File size %1 is less than requested start position: %2")
                           .arg(filesize)
                           .arg(startPos));
                return;
            }
            QByteArray ba;
            if(startPos == 0) {
                ba.append(sFileName.toLocal8Bit());
                ba.append(QString(",%1").arg(filesize).toLocal8Bit());
                ba.append(QString(1024-ba.length(), '\0').toLocal8Bit());
            }
            if(file.open(QIODevice::ReadOnly)) {
                if(file.seek(startPos)) {
                    ba.append(file.read(length));
                    if(ba.size() == 1024) {// Read error !
                        file.close();
                        logMessage(logFile,
                                   Q_FUNC_INFO,
                                   QString("Error reading %1")
                                   .arg(sFileName));
                        return;
                    }
                    if(pClient->isValid()) {
                        auto bytesSent = int(pClient->sendBinaryMessage(ba));
                        if(bytesSent != ba.size()) {
                            logMessage(logFile,
                                       Q_FUNC_INFO,
                                       QString("Unable to send the file %1")
                                       .arg(sFileName));
                        }
#ifdef LOG_VERBOSE
                        else {
                            logMessage(logFile,
                                       Q_FUNC_INFO,
                                       QString("File %1 correctly sent")
                                       .arg(sFileName));
                        }
#endif
                    }
                    else { // Client disconnected
                        file.close();
                        logMessage(logFile,
                                   Q_FUNC_INFO,
                                   QString("Client disconnected while sending %1")
                                   .arg(sFileName));
                        return;
                    }
                    return;
                }
                // Unsuccesful seek
                    file.close();
                    logMessage(logFile,
                               Q_FUNC_INFO,
                               QString("Error seeking %1 to %2")
                               .arg(sFileName)
                               .arg(startPos));
                    return;

            }
            // file.open failed !
            logMessage(logFile,
                       Q_FUNC_INFO,
                       QString("Unable to open the file %1")
                       .arg(sFileName));
            return;
            
        }
        sMessage = QString("<missingFile>%1</missingFile>").arg(sToken);
        if(pClient->isValid()) {
            pClient->sendTextMessage(sMessage);
        }
        logMessage(logFile,
                   Q_FUNC_INFO,
                   QString("Missing File: %1")
                   .arg(sFileName));
        
    }

    sToken = XML_Parse(sMessage, "send_file_list");
    if(sToken != sNoData) {
        if(fileList.isEmpty()) {
            sMessage = QString("<file_list>0/file_list>");
            SendToOne(pClient, sMessage);
            return;
        }
        if(pClient->isValid()) {
            sMessage = QString("<file_list>");
            for(int i=0; i<fileList.count()-1; i++) {
                sMessage += fileList.at(i).fileName();
                sMessage += QString(";%1,").arg(fileList.at(i).size());
            }
            int i = fileList.count()-1;
            sMessage += fileList.at(i).fileName();
            sMessage += QString(";%1</file_list>").arg(fileList.at(i).size());
            SendToOne(pClient, sMessage);
        }
    }// send_spot_list
}


/*!
 * \brief FileServer::onFileTransferDone
 * Invoked when a transfer is done (with or without errors)
 * \param bSuccess
 */
void
FileServer::onFileTransferDone(bool bSuccess) {
    Q_UNUSED(bSuccess)
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               serverName +
               QString(" File Transfer terminated with code %1")
               .arg(bSuccess));
#endif
}


/*!
 * \brief FileServer::SendToOne
 * \param pClient
 * \param sMessage
 * \return
 */
int
FileServer::SendToOne(QWebSocket* pClient, const QString& sMessage) {
    if (pClient->isValid()) {
        qint64 written = pClient->sendTextMessage(sMessage);
        if(written != sMessage.length()) {
            logMessage(logFile,
                       serverName +
                       Q_FUNC_INFO,
                       QString(" Error writing %1").arg(sMessage));
        }
#ifdef LOG_VERBOSE
        else {
            logMessage(logFile,
                       Q_FUNC_INFO,
                       serverName +
                       QString(" Sent %1 to: %2")
                       .arg(sMessage, pClient->peerAddress().toString()));
        }
#endif
    }
    else {
        logMessage(logFile,
                   Q_FUNC_INFO,
                   serverName +
                   QString(" Client socket is invalid !"));
    }
    return 0;
}


/*!
 * \brief FileServer::onProcessBinaryMessage
 * \param message
 */
void
FileServer::onProcessBinaryMessage(QByteArray message) {
    Q_UNUSED(message)
    logMessage(logFile,
               Q_FUNC_INFO,
               QString("Unexpected binary message received !"));
}


void
FileServer::onClientDisconnected() {
    auto* pClient = qobject_cast<QWebSocket *>(sender());
    QString sDiconnectedAddress = pClient->peerAddress().toString();
#ifdef LOG_VERBOSE
    logMessage(logFile,
               Q_FUNC_INFO,
               serverName +
               QString(" %1 disconnected because %2. Close code: %3")
               .arg(sDiconnectedAddress, pClient->closeReason())
               .arg(pClient->closeCode()));
#endif
    if(!connections.removeOne(pClient)) {
        logMessage(logFile,
                   Q_FUNC_INFO,
                   serverName +
                   QString(" Unable to remove %1 from list !")
                   .arg(sDiconnectedAddress));
    }
}

/*!
 * \brief FileServer::onCloseServer
 */
void
FileServer::onCloseServer() {
    for(int i=0; i<connections.count(); i++) {
        connections.at(i)->disconnect();
        if(connections.at(i)->isValid())
            connections.at(i)->close();
        delete connections.at(i);
    }
    connections.clear();
    for(int i=0; i<senderThreads.count(); i++) {
        senderThreads.at(i)->requestInterruption();
        if(senderThreads.at(i)->wait(3000)) {
            logMessage(logFile,
                       Q_FUNC_INFO,
                       serverName +
                       QString(" File Server Thread %1 regularly closed")
                       .arg(i));
        }
        else {
            logMessage(logFile,
                       Q_FUNC_INFO,
                       serverName +
                       QString(" File Server Thread %1 forced to close")
                       .arg(i));
            senderThreads.at(i)->terminate();
        }
    }
    // NetServer::closeServer() calls
    // thread()->quit()
    // to quit the processing thread
    NetServer::closeServer();
}
