#include <QStandardPaths>
#include <QDir>
#include <QFileInfo>
#include <QMessageBox>
#include <QThread>
#include <QNetworkInterface>
#include <QUdpSocket>
#include <QWebSocket>
#include <QHBoxLayout>


#include "scorecontroller.h"
#include "utility.h"
#include "clientlistdialog.h"

#define DISCOVERY_PORT      45453
#define SERVER_SOCKET_PORT  45454
#define SPOT_UPDATER_PORT   45455
#define SLIDE_UPDATER_PORT  45456


ScoreController::ScoreController(QWidget *parent)
    : QMainWindow(parent)
    , pClientListDialog(nullptr)
    , pButtonClick(nullptr)
    , pSettings(nullptr)
    , discoveryPort(DISCOVERY_PORT)
    , discoveryAddress(QHostAddress("224.0.0.1"))
    , pSlideUpdaterServer(nullptr)
    , pSpotUpdaterServer(nullptr)
    , serverPort(SERVER_SOCKET_PORT)
    , spotUpdaterPort(SPOT_UPDATER_PORT)
    , pSpotServerThread(nullptr)
    , pSlideServerThread(nullptr)
    , slideUpdaterPort(SLIDE_UPDATER_PORT)
{
    // For Message Logging...
    pLogFile = nullptr;

     // Logged messages (if enabled) will be written in the following folder
    sLogDir = QStandardPaths::writableLocation(QStandardPaths::GenericDataLocation);
    if(!sLogDir.endsWith(QString("/"))) sLogDir+= QString("/");

    logFileName = QString("%1score_controller.txt").arg(sLogDir);
    prepareLogFile();

    // The click sound for button press.
    // To have an acoustic feedback on touch screen tablets.
    pButtonClick = new QSoundEffect(this);
    pButtonClick->setSource(QUrl::fromLocalFile(":/key.wav"));

    // Block until a network connection is available
    if(WaitForNetworkReady() != QMessageBox::Ok) {
        exit(0);
    }

    pSpotButtonsLayout = CreateSpotButtons();

    // A List of IP Addresses of the connected Score Panels
    sIpAddresses = QStringList();

    // The default Directories to look for the slides and spots
    sSlideDir   = QStandardPaths::writableLocation(QStandardPaths::PicturesLocation);
    if(!sSlideDir.endsWith(QString("/"))) sSlideDir+= QString("/");
    sSpotDir    = QStandardPaths::writableLocation(QStandardPaths::MoviesLocation);
    if(!sSpotDir.endsWith(QString("/"))) sSpotDir+= QString("/");

    slideList     = QFileInfoList();
    spotList      = QFileInfoList();
    iCurrentSlide = 0;
    iCurrentSpot  = 0;

    connectButtonSignals();

    myStatus = showPanel;
}


ScoreController::~ScoreController() {
}


bool
ScoreController::prepareLogFile() {
#if defined(LOG_MESG)
    QFileInfo checkFile(logFileName);
    if(checkFile.exists() && checkFile.isFile()) {
        QDir renamed;
        renamed.remove(logFileName+QString(".bkp"));
        renamed.rename(logFileName, logFileName+QString(".bkp"));
    }
    pLogFile = new QFile(logFileName);
    if (!pLogFile->open(QIODevice::WriteOnly)) {
        QMessageBox::information(this, tr("Volley Controller"),
                                 tr("Impossibile aprire il file %1: %2.")
                                 .arg(logFileName, pLogFile->errorString()));
        delete pLogFile;
        pLogFile = nullptr;
    }
#endif
    return true;
}


QHBoxLayout*
ScoreController::CreateSpotButtons() {
    auto* spotButtonLayout = new QHBoxLayout();

    QPixmap pixmap(":/buttonIcons/PlaySpots.png");
    QIcon ButtonIcon(pixmap);
    startStopLoopSpotButton = new QPushButton(ButtonIcon, "");
    startStopLoopSpotButton->setIconSize(pixmap.rect().size());
    startStopLoopSpotButton->setFlat(true);
    startStopLoopSpotButton->setToolTip("Start/Stop Spot Loop");

    pixmap.load(":/buttonIcons/PlaySlides.png");
    ButtonIcon.addPixmap(pixmap);
    startStopSlideShowButton = new QPushButton(ButtonIcon, "");
    startStopSlideShowButton->setIconSize(pixmap.rect().size());
    startStopSlideShowButton->setFlat(true);
    startStopSlideShowButton->setToolTip("Start/Stop Slide Show");

    pixmap.load(":/buttonIcons/Camera.png");
    ButtonIcon.addPixmap(pixmap);
    startStopLiveCameraButton = new QPushButton(ButtonIcon, "");
    startStopLiveCameraButton->setIconSize(pixmap.rect().size());
    startStopLiveCameraButton->setFlat(true);
    startStopLiveCameraButton->setToolTip("Start/Stop Live Camera");

    pixmap.load(":/buttonIcons/PanelSetup.png");
    ButtonIcon.addPixmap(pixmap);
    panelControlButton = new QPushButton(ButtonIcon, "");
    panelControlButton->setIconSize(pixmap.rect().size());
    panelControlButton->setFlat(true);
    panelControlButton->setToolTip("Panel Setup");

    pixmap.load(":/buttonIcons/GeneralSetup.png");
    ButtonIcon.addPixmap(pixmap);
    generalSetupButton = new QPushButton(ButtonIcon, "");
    generalSetupButton->setIconSize(pixmap.rect().size());
    generalSetupButton->setFlat(true);
    generalSetupButton->setToolTip("General Setup");

    pixmap.load(":/buttonIcons/video-display.png");
    ButtonIcon.addPixmap(pixmap);
    shutdownButton = new QPushButton(ButtonIcon, "");
    shutdownButton->setIconSize(pixmap.rect().size());
    shutdownButton->setFlat(true);
    shutdownButton->setToolTip("Shutdown System");

    startStopLoopSpotButton->setDisabled(true);
    startStopSlideShowButton->setDisabled(true);
    startStopLiveCameraButton->setDisabled(true);

    panelControlButton->setDisabled(true);
    generalSetupButton->setEnabled(true);
    shutdownButton->setDisabled(true);

    spotButtonLayout->addWidget(startStopLoopSpotButton);

    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(startStopSlideShowButton);

    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(startStopLiveCameraButton);

    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(panelControlButton);

    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(generalSetupButton);

    spotButtonLayout->addStretch();
    spotButtonLayout->addWidget(shutdownButton);

    return spotButtonLayout;
}


void
ScoreController::connectButtonSignals() {
        connect(panelControlButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(panelControlButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonPanelControlClicked()));

        connect(startStopLoopSpotButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonStartStopSpotLoopClicked()));
        connect(startStopLoopSpotButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));

        connect(startStopSlideShowButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonStartStopSlideShowClicked()));
        connect(startStopSlideShowButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));

        connect(startStopLiveCameraButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonStartStopLiveCameraClicked()));
        connect(startStopLiveCameraButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));

        connect(generalSetupButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonSetupClicked()));
        connect(generalSetupButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));

        connect(shutdownButton, SIGNAL(clicked(bool)),
                this, SLOT(onButtonShutdownClicked()));
        connect(shutdownButton, SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
}

int
ScoreController::WaitForNetworkReady() {
    int iResponse;
    while(!isConnectedToNetwork()) {
        iResponse = QMessageBox::critical(this,
                                          tr("Connessione Assente"),
                                          tr("Connettiti alla rete e ritenta"),
                                          QMessageBox::Retry,
                                          QMessageBox::Abort);

        if(iResponse == QMessageBox::Abort) {
            return iResponse;
        }
        QThread::sleep(1);
    }
    return QMessageBox::Ok;
}

bool
ScoreController::isConnectedToNetwork() {
    QList<QNetworkInterface> ifaces = QNetworkInterface::allInterfaces();
    bool result = false;

    for(int i=0; i<ifaces.count(); i++) {
        const QNetworkInterface& iface = ifaces.at(i);
        if(iface.flags().testFlag(QNetworkInterface::IsUp) &&
           iface.flags().testFlag(QNetworkInterface::IsRunning) &&
           iface.flags().testFlag(QNetworkInterface::CanBroadcast) &&
          !iface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            for(int j=0; j<iface.addressEntries().count(); j++) {
                if(!result) result = true;
            }
        }
    }
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               result ? QString("true") : QString("false"));
#endif
    return result;
}

void
ScoreController::prepareDirectories() {
    QDir slideDir(sSlideDir);
    QDir spotDir(sSpotDir);

    if(!slideDir.exists() || !spotDir.exists()) {
        onButtonSetupClicked();
        slideDir.setPath(sSlideDir);
        if(!slideDir.exists())
            sSlideDir = QStandardPaths::displayName(QStandardPaths::GenericDataLocation);
        if(!sSlideDir.endsWith(QString("/"))) sSlideDir+= QString("/");
        spotDir.setPath(sSpotDir);
        if(!spotDir.exists())
            sSpotDir = QStandardPaths::displayName(QStandardPaths::GenericDataLocation);
        if(!sSpotDir.endsWith(QString("/"))) sSpotDir+= QString("/");
        pSettings->setValue("directories/slides", sSlideDir);
        pSettings->setValue("directories/spots", sSpotDir);
    }
    else {
        QStringList filter(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.JPG" << "*.JPEG" << "*.PNG");
        slideDir.setNameFilters(filter);
        slideList = slideDir.entryInfoList();
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Slides directory: %1 Found %2 Slides")
                   .arg(sSlideDir)
                   .arg(slideList.count()));
#endif
        QStringList nameFilter(QStringList() << "*.mp4"<< "*.MP4");
        spotDir.setNameFilters(nameFilter);
        spotDir.setFilter(QDir::Files);
        spotList = spotDir.entryInfoList();
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Spot directory: %1 Found %2 Spots")
                   .arg(sSpotDir)
                   .arg(spotList.count()));
#endif
    }
}


void
ScoreController::onButtonSetupClicked() {
    GetGeneralSetup();
}


void
ScoreController::GetGeneralSetup() {

}


void
ScoreController::SaveStatus() {
}


void
ScoreController::prepareServices() {
    // Start listening to the discovery port
    if(!prepareDiscovery()) {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("!prepareDiscovery()"));
        close();
    }
    // Prepare the Server port for the Panels to connect to
    else if(!prepareServer()) {
        close();
    }
    else {
        prepareSpotUpdateService();
        prepareSlideUpdateService();
    }
}


bool
ScoreController::prepareDiscovery() {
    bool bSuccess = false;
    sIpAddresses = QStringList();
    QList<QNetworkInterface> interfaceList = QNetworkInterface::allInterfaces();
    for(int i=0; i<interfaceList.count(); i++) {
        const QNetworkInterface& interface = interfaceList.at(i);
        if(interface.flags().testFlag(QNetworkInterface::IsUp) &&
           interface.flags().testFlag(QNetworkInterface::IsRunning) &&
           interface.flags().testFlag(QNetworkInterface::CanMulticast) &&
          !interface.flags().testFlag(QNetworkInterface::IsLoopBack))
        {
            QList<QNetworkAddressEntry> list = interface.addressEntries();
            for(int j=0; j<list.count(); j++) {
                auto* pDiscoverySocket = new QUdpSocket(this);
                if(list[j].ip().protocol() == QAbstractSocket::IPv4Protocol) {
                    if(pDiscoverySocket->bind(QHostAddress::AnyIPv4, discoveryPort, QUdpSocket::ShareAddress)) {
                        pDiscoverySocket->joinMulticastGroup(discoveryAddress);
                        sIpAddresses.append(list[j].ip().toString());
                        discoverySocketArray.append(pDiscoverySocket);
                        connect(pDiscoverySocket, SIGNAL(readyRead()),
                                this, SLOT(onProcessConnectionRequest()));
                        bSuccess = true;
#ifdef LOG_VERBOSE
                        logMessage(pLogFile,
                                   Q_FUNC_INFO,
                                   QString("Listening for connections at address: %1 port:%2")
                                   .arg(discoveryAddress.toString())
                                   .arg(discoveryPort));
#endif
                    }
                    else {
                        logMessage(pLogFile,
                                   Q_FUNC_INFO,
                                   QString("%1 bind() failed")
                                   .arg(discoveryAddress.toString()));
                        delete pDiscoverySocket;
                    }
                }
                else {
                    delete pDiscoverySocket;
                }
            }// for(int j=0; j<list.count(); j++)
        }
    }// for(int i=0; i<interfaceList.count(); i++)
    return bSuccess;
}

bool
ScoreController::prepareServer() {
    pPanelServer = new NetServer(QString("PanelServer"), pLogFile, this);
    if(!pPanelServer->prepareServer(serverPort)) {
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("prepareServer() Failed !"));
#endif
        delete pPanelServer;
        pPanelServer = nullptr;
        return false;
    }
    connect(pPanelServer, SIGNAL(newConnection(QWebSocket*)),
            this, SLOT(onNewConnection(QWebSocket*)));
    return true;
}

void
ScoreController::prepareSpotUpdateService() {
    pSpotUpdaterServer = new FileServer(QString("SpotUpdater"), pLogFile, nullptr);
    connect(pSpotUpdaterServer, SIGNAL(fileServerDone(bool)),
            this, SLOT(onSpotServerDone(bool)));
    pSpotUpdaterServer->setServerPort(spotUpdaterPort);
    pSpotServerThread = new QThread();
    pSpotUpdaterServer->moveToThread(pSpotServerThread);
    connect(this, SIGNAL(startSpotServer()),
            pSpotUpdaterServer, SLOT(onStartServer()));
    connect(this, SIGNAL(closeSpotServer()),
            pSpotUpdaterServer, SLOT(onCloseServer()));
    pSpotServerThread->start(QThread::LowestPriority);
}

void
ScoreController::prepareSlideUpdateService() {
    pSlideUpdaterServer = new FileServer(QString("SlideUpdater"), pLogFile, nullptr);
    connect(pSlideUpdaterServer, SIGNAL(fileServerDone(bool)),
            this, SLOT(onSlideServerDone(bool)));
    pSlideUpdaterServer->setServerPort(slideUpdaterPort);
    pSlideServerThread = new QThread();
    pSlideUpdaterServer->moveToThread(pSlideServerThread);
    connect(this, SIGNAL(startSlideServer()),
            pSlideUpdaterServer, SLOT(onStartServer()));
    connect(this, SIGNAL(closeSlideServer()),
            pSlideUpdaterServer, SLOT(onCloseServer()));
    pSlideServerThread->start(QThread::LowestPriority);
}


void
ScoreController::onProcessConnectionRequest() {
    QByteArray datagram, request;
    QString sToken;
    auto* pDiscoverySocket = qobject_cast<QUdpSocket*>(sender());
    QString sNoData = QString("NoData");
    QString sMessage;
    Q_UNUSED(sMessage)
    QHostAddress hostAddress;
    quint16 port=0;

    while(pDiscoverySocket->hasPendingDatagrams()) {
        datagram.resize(int(pDiscoverySocket->pendingDatagramSize()));
        pDiscoverySocket->readDatagram(datagram.data(), datagram.size(), &hostAddress, &port);
        request.append(datagram.data());
/*!
 * \todo Do we have to limit the maximum amount of data that can be received ???
 */
    }
    sToken = XML_Parse(request.data(), "getServer");
    if(sToken != sNoData) {
        sendAcceptConnection(pDiscoverySocket, hostAddress, port);
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Connection request from: %1 at Address %2:%3")
                   .arg(sToken, hostAddress.toString())
                   .arg(port));
#endif
        // If a Client with the same address asked for a Server it means that
        // the connections has dropped (at least it think so). Then remove it
        // from the connected clients list
        RemoveClient(hostAddress);
#ifdef LOG_VERBOSE
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Sent: %1")
                   .arg(sMessage));
#endif
        UpdateUI();// To disable some buttons if this was the last client
    }
}

void
ScoreController::sendAcceptConnection(QUdpSocket* pDiscoverySocket, const QHostAddress& hostAddress, quint16 port) {
    QString sString = QString("%1,0").arg(sIpAddresses.at(0));
    for(int i=1; i<sIpAddresses.count(); i++) {
        sString += QString(";%1,0").arg(sIpAddresses.at(i));
    }
    QString sMessage = "<serverIP>" + sString + "</serverIP>";
    QByteArray datagram = sMessage.toUtf8();
    qint64 bytesWritten = pDiscoverySocket->writeDatagram(datagram.data(), datagram.size(), hostAddress, port);
    Q_UNUSED(bytesWritten)
    if(bytesWritten != datagram.size()) {
        logMessage(pLogFile,
                 Q_FUNC_INFO,
                 QString("Unable to send data !"));
    }
}

void
ScoreController::RemoveClient(const QHostAddress& hAddress) {
    for(int i=connectionList.count()-1; i>=0; i--) {
        QWebSocket* pClient = connectionList.at(i).pClientSocket;
        if(pClient->peerAddress().toIPv4Address() == hAddress.toIPv4Address())
        {
            pClient->disconnect(); // No more events from this socket
            if(pClient->isValid())
                pClient->close(QWebSocketProtocol::CloseCodeNormal,
                               tr("Socket disconnection"));
            delete pClient;
            pClient = nullptr;
            connectionList.removeAt(i);
#ifdef LOG_VERBOSE
            logMessage(pLogFile,
                       Q_FUNC_INFO,
                       QString("%1")
                       .arg(hAddress.toString()));
#endif
        }
    }
}


void
ScoreController::UpdateUI() {
    if(connectionList.count() == 1) {
        startStopLoopSpotButton->setEnabled(true);
        startStopSlideShowButton->setEnabled(true);
        startStopLiveCameraButton->setEnabled(true);
        panelControlButton->setEnabled(true);
        shutdownButton->setEnabled(true);
    }
    else if(connectionList.count() == 0) {
        startStopLoopSpotButton->setDisabled(true);
        QPixmap pixmap(":/buttonIcons/PlaySpots.png");
        QIcon ButtonIcon(pixmap);
        startStopLoopSpotButton->setIcon(ButtonIcon);
        startStopLoopSpotButton->setIconSize(pixmap.rect().size());

        startStopSlideShowButton->setDisabled(true);
        pixmap.load(":/buttonIcons/PlaySlides.png");
        ButtonIcon.addPixmap(pixmap);
        startStopSlideShowButton->setIcon(ButtonIcon);
        startStopSlideShowButton->setIconSize(pixmap.rect().size());

        startStopLiveCameraButton->setDisabled(true);
        pixmap.load(":buttonIcons/Camera.png");
        ButtonIcon.addPixmap(pixmap);
        startStopLiveCameraButton->setIcon(ButtonIcon);
        startStopLiveCameraButton->setIconSize(pixmap.rect().size());

        panelControlButton->setDisabled(true);
        generalSetupButton->setEnabled(true);
        shutdownButton->setDisabled(true);
        myStatus = showPanel;
    }
}


QString
ScoreController::FormatStatusMsg() {
    QString sMessage = QString();
    return sMessage;
}

int
ScoreController::SendToOne(QWebSocket* pClient, const QString& sMessage) {
    if (pClient->isValid()) {
        for(int i=0; i< connectionList.count(); i++) {
            QWebSocket* pConnected = connectionList.at(i).pClientSocket;
           if(pConnected->peerAddress().toIPv4Address() ==
              pClient->peerAddress().toIPv4Address())
           {
                qint64 written = pClient->sendTextMessage(sMessage);
                Q_UNUSED(written)
                if(written != sMessage.length()) {
                    logMessage(pLogFile,
                               Q_FUNC_INFO,
                               QString("Error writing %1").arg(sMessage));
                }
#ifdef LOG_VERBOSE
                else {
                    logMessage(pLogFile,
                               Q_FUNC_INFO,
                               QString("Sent %1 to: %2")
                               .arg(sMessage, pClient->peerAddress().toString()));
                }
#endif
                break;
            }
        }
    }
    else {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Client socket is invalid !"));
        RemoveClient(pClient->peerAddress());
        UpdateUI();
    }
    return 0;
}


int
ScoreController::SendToAll(const QString& sMessage) {
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               sMessage);
#endif
    for(int i=0; i< connectionList.count(); i++) {
        SendToOne(connectionList.at(i).pClientSocket, sMessage);
    }
    return 0;
}


void
ScoreController::onNewConnection(QWebSocket *pClient) {
    QHostAddress address = pClient->peerAddress();

    connect(pClient, SIGNAL(textMessageReceived(QString)),
            this, SLOT(onProcessTextMessage(QString)));
    connect(pClient, SIGNAL(binaryMessageReceived(QByteArray)),
            this, SLOT(onProcessBinaryMessage(QByteArray)));
    connect(pClient, SIGNAL(disconnected()),
            this, SLOT(onClientDisconnected()));

    RemoveClient(address);

    Connection newConnection(pClient);
    connectionList.append(newConnection);
    UpdateUI();
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Client connected: %1")
               .arg(pClient->peerAddress().toString()));
#endif
}


void
ScoreController::onSlideServerDone(bool bError) {
    Q_UNUSED(bError)
#ifdef LOG_VERBOSE
    // Log a Message just to inform
    if(bError) {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Slide server stopped with errors"));
    }
    else {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Slide server stopped without errors"));
    }
#endif
}


void
ScoreController::onSpotServerDone(bool bError) {
    Q_UNUSED(bError)
#ifdef LOG_VERBOSE
    // Log a Message just to inform
    if(bError) {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Spot server stopped with errors"));
    }
    else {
        logMessage(pLogFile,
                   Q_FUNC_INFO,
                   QString("Spot server stopped without errors"));
    }
#endif
}

void
ScoreController::onProcessTextMessage(QString sMessage) {
    QString sToken;
    QString sNoData = QString("NoData");

    // The Panel is asking for the Status
    sToken = XML_Parse(sMessage, "getStatus");
    if(sToken != sNoData) {
        auto *pClient = qobject_cast<QWebSocket *>(sender());
        SendToOne(pClient, FormatStatusMsg());
    }// getStatus

    // The Panel communicates the local Pan and Tilt values
    sToken = XML_Parse(sMessage, "pan_tilt");
    if(sToken != sNoData) {
        QStringList values = QStringList(sToken.split(",",Qt::SkipEmptyParts));
//        qDebug() << "pan_tilt" << values;
        if(pClientListDialog)
            pClientListDialog->remotePanTiltReceived(values.at(0).toInt(), values.at(1).toInt());
    }// pan_tilt

    // The Panel communicates its orientation
    sToken = XML_Parse(sMessage, "orientation");
    if(sToken != sNoData) {
        bool ok;
        int iDirection = sToken.toInt(&ok);
        if(!ok) {
            logMessage(pLogFile,
                       Q_FUNC_INFO,
                       QString("Illegal Direction received: %1")
                       .arg(sToken));
            return;
        }
        auto direction = static_cast<PanelDirection>(iDirection);
//        qDebug() << "orientation" << ((direction==PanelDirection::Normal)? "Normal" : "Reflected");
        if(pClientListDialog)
            pClientListDialog->remoteDirectionReceived(direction);
    }// orientation

    // The Panel communicates if it shows only the score
    sToken = XML_Parse(sMessage, "isScoreOnly");
    if(sToken != sNoData) {
        bool ok;
        auto isScoreOnly = bool(sToken.toInt(&ok));
        if(!ok) {
            logMessage(pLogFile,
                       Q_FUNC_INFO,
                       QString("Illegal Score Only value received: %1")
                       .arg(sToken));
            return;
        }
//        qDebug() << "isScoreOnly" << isScoreOnly;
        if(pClientListDialog)
            pClientListDialog->remoteScoreOnlyValueReceived(isScoreOnly);
    }// isScoreOnly
}

void
ScoreController::onProcessBinaryMessage(QByteArray message) {
    Q_UNUSED(message)
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Unexpected binary message received !"));
}

void
ScoreController::onClientDisconnected() {
    auto* pClient = qobject_cast<QWebSocket *>(sender());
#ifdef LOG_VERBOSE
    QString sDiconnectedAddress = pClient->peerAddress().toString();
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("%1 disconnected because %2. Close code: %3")
               .arg(sDiconnectedAddress, pClient->closeReason())
               .arg(pClient->closeCode()));
#endif
    RemoveClient(pClient->peerAddress());
    UpdateUI();
}

void
ScoreController::onButtonStartStopSpotLoopClicked() {
    QString sMessage;
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(connectionList.count() == 0) {
        pixmap.load(":/buttonIcons/PlaySpots.png");
        ButtonIcon.addPixmap(pixmap);
        startStopLoopSpotButton->setIcon(ButtonIcon);
        startStopLoopSpotButton->setIconSize(pixmap.rect().size());
        startStopLoopSpotButton->setDisabled(true);
        myStatus = showPanel;
        return;
    }
    if(myStatus == showPanel) {
        sMessage = QString("<spotloop>1</spotloop>");
        SendToAll(sMessage);
        pixmap.load(":/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        startStopLoopSpotButton->setIcon(ButtonIcon);
        startStopLoopSpotButton->setIconSize(pixmap.rect().size());
        startStopSlideShowButton->setDisabled(true);
        startStopLiveCameraButton->setDisabled(true);
        panelControlButton->setDisabled(true);
        generalSetupButton->setDisabled(true);
        myStatus = showSpots;
    }
    else {
        sMessage = "<endspotloop>1</endspotloop>";
        SendToAll(sMessage);
        pixmap.load(":/buttonIcons/PlaySpots.png");
        ButtonIcon.addPixmap(pixmap);
        startStopLoopSpotButton->setIcon(ButtonIcon);
        startStopLoopSpotButton->setIconSize(pixmap.rect().size());
        startStopSlideShowButton->setEnabled(true);
        startStopLiveCameraButton->setEnabled(true);
        panelControlButton->setEnabled(true);
        generalSetupButton->setEnabled(true);
        myStatus = showPanel;
    }
}

void
ScoreController::onGetPanelDirection(const QString& sClientIp) {
    QHostAddress hostAddress(sClientIp);
    for(int i=0; i<connectionList.count(); i++) {
        QWebSocket* pClient = connectionList.at(i).pClientSocket;
        if(pClient->peerAddress().toIPv4Address() == hostAddress.toIPv4Address()) {
            QString sMessage = "<getOrientation>1</getOrientation>";
            SendToOne(pClient, sMessage);
            return;
        }
    }
}

void
ScoreController::onChangePanelDirection(const QString& sClientIp, PanelDirection direction) {
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Client %1 Direction %2")
               .arg(sClientIp)
               .arg(static_cast<int>(direction)));
#endif
    QHostAddress hostAddress(sClientIp);
    for(int i=0; i<connectionList.count(); i++) {
        QWebSocket* pClient = connectionList.at(i).pClientSocket;
        if(pClient->peerAddress().toIPv4Address() == hostAddress.toIPv4Address()) {
            QString sMessage = QString("<setOrientation>%1</setOrientation>")
                                       .arg(static_cast<int>(direction));
            SendToOne(pClient, sMessage);
            return;
        }
    }
}


void
ScoreController::onGetIsPanelScoreOnly(const QString& sClientIp) {
    QHostAddress hostAddress(sClientIp);
    for(int i=0; i<connectionList.count(); i++) {
        QWebSocket* pClient = connectionList.at(i).pClientSocket;
        if(pClient->peerAddress().toIPv4Address() == hostAddress.toIPv4Address()) {
            QString sMessage = "<getScoreOnly>1</getScoreOnly>";
            SendToOne(connectionList.at(i).pClientSocket, sMessage);
            return;
        }
    }
}


void
ScoreController::onButtonStartStopLiveCameraClicked() {
    QString sMessage;
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(connectionList.count() == 0) {
        pixmap.load(":/buttonIcons/Camera.png");
        ButtonIcon.addPixmap(pixmap);
        startStopLiveCameraButton = new QPushButton(ButtonIcon, "");
        startStopLiveCameraButton->setIconSize(pixmap.rect().size());
        startStopLiveCameraButton->setDisabled(true);
        myStatus = showPanel;
        return;
    }
    if(myStatus == showPanel) {
        sMessage = QString("<live>1</live>");
        SendToAll(sMessage);
        pixmap.load(":/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        startStopLiveCameraButton->setIcon(ButtonIcon);
        startStopLiveCameraButton->setIconSize(pixmap.rect().size());
        startStopLoopSpotButton->setDisabled(true);
        startStopSlideShowButton->setDisabled(true);
        panelControlButton->setDisabled(true);
        generalSetupButton->setDisabled(true);
        myStatus = showCamera;
    }
    else {
        sMessage = "<endlive>1</endlive>";
        SendToAll(sMessage);
        pixmap.load(":/buttonIcons/Camera.png");
        ButtonIcon.addPixmap(pixmap);
        startStopLiveCameraButton->setIcon(ButtonIcon);
        startStopLiveCameraButton->setIconSize(pixmap.rect().size());
        startStopLoopSpotButton->setEnabled(true);
        startStopSlideShowButton->setEnabled(true);
        panelControlButton->setEnabled(true);
        generalSetupButton->setEnabled(true);
        myStatus = showPanel;
    }
}


void
ScoreController::onButtonStartStopSlideShowClicked() {
    QString sMessage;
    QPixmap pixmap;
    QIcon ButtonIcon;
    if(connectionList.count() == 0) {
        pixmap.load(":/buttonIcons/PlaySlides.png");
        ButtonIcon.addPixmap(pixmap);
        startStopSlideShowButton->setIcon(ButtonIcon);
        startStopSlideShowButton->setIconSize(pixmap.rect().size());
        startStopSlideShowButton->setDisabled(true);
        myStatus = showPanel;
        return;
    }
    if(myStatus == showPanel) {
        sMessage = "<slideshow>1</slideshow>";
        SendToAll(sMessage);
        startStopLoopSpotButton->setDisabled(true);
        startStopLiveCameraButton->setDisabled(true);
        panelControlButton->setDisabled(true);
        generalSetupButton->setDisabled(true);
        pixmap.load(":/buttonIcons/sign_stop.png");
        ButtonIcon.addPixmap(pixmap);
        startStopSlideShowButton->setIcon(ButtonIcon);
        startStopSlideShowButton->setIconSize(pixmap.rect().size());
        myStatus = showSlides;
    }
    else {
        sMessage = "<endslideshow>1</endslideshow>";
        SendToAll(sMessage);
        startStopLoopSpotButton->setEnabled(true);
        startStopLiveCameraButton->setEnabled(true);
        panelControlButton->setEnabled(true);
        generalSetupButton->setEnabled(true);
        pixmap.load(":/buttonIcons/PlaySlides.png");
        ButtonIcon.addPixmap(pixmap);
        startStopSlideShowButton->setIcon(ButtonIcon);
        startStopSlideShowButton->setIconSize(pixmap.rect().size());
        myStatus = showPanel;
    }
}


void
ScoreController::onButtonShutdownClicked() {
    QMessageBox msgBox;
    msgBox.setText("Sei Sicuro di Volere Spegnere");
    msgBox.setInformativeText("i Tabelloni ?");
    msgBox.setStandardButtons(QMessageBox::Yes | QMessageBox::No);
    msgBox.setDefaultButton(QMessageBox::No);
    int answer = msgBox.exec();
    if(answer != QMessageBox::Yes) return;
    QString sMessage = "<kill>1</kill>";
    SendToAll(sMessage);
}


void
ScoreController::onButtonPanelControlClicked() {
    pClientListDialog = new ClientListDialog(connectionList, this);
    // ClientListDialog Signals Management...
    // Pan-Tilt Camera management
    connect(pClientListDialog, SIGNAL(disableVideo()),
            this, SLOT(onStopCamera()));
    connect(pClientListDialog, SIGNAL(enableVideo(QString)),
            this, SLOT(onStartCamera(QString)));
    connect(pClientListDialog, SIGNAL(newPanValue(QString,int)),
            this, SLOT(onSetNewPanValue(QString,int)));
    connect(pClientListDialog, SIGNAL(newTiltValue(QString,int)),
            this, SLOT(onSetNewTiltValue(QString,int)));
    // Panel orientation management
    connect(pClientListDialog, SIGNAL(getDirection(QString)),
            this, SLOT(onGetPanelDirection(QString)));
    connect(pClientListDialog, SIGNAL(changeDirection(QString,PanelDirection)),
            this, SLOT(onChangePanelDirection(QString,PanelDirection)));
    // Score Only Panel management
    connect(pClientListDialog, SIGNAL(getScoreOnly(QString)),
            this, SLOT(onGetIsPanelScoreOnly(QString)));
    connect(pClientListDialog, SIGNAL(changeScoreOnly(QString,bool)),
            this, SLOT(onSetScoreOnly(QString,bool)));
    pClientListDialog->exec();
    delete pClientListDialog;
}


void
ScoreController::onStartCamera(const QString& sClientIp) {
    QHostAddress hostAddress(sClientIp);
    for(int i=0; i<connectionList.count(); i++) {
        QWebSocket* pClient = connectionList.at(i).pClientSocket;
        if(pClient->peerAddress().toIPv4Address() == hostAddress.toIPv4Address()) {
            QString sMessage = QString("<live>1</live>");
            SendToOne(connectionList.at(i).pClientSocket, sMessage);
            sMessage = QString("<getPanTilt>1</getPanTilt>");
            SendToOne(pClient, sMessage);
            return;
        }
        myStatus = showCamera;
    }
}


void
ScoreController::onStopCamera() {
    QString sMessage = QString("<endlive>1</endlive>");
    SendToAll(sMessage);
    myStatus = showPanel;
}


void
ScoreController::onSetNewPanValue(const QString& sClientIp, int newPan) {
  QHostAddress hostAddress(sClientIp);
  for(int i=0; i<connectionList.count(); i++) {
      QWebSocket* pClient = connectionList.at(i).pClientSocket;
      if(pClient->peerAddress().toIPv4Address() == hostAddress.toIPv4Address()) {
          QString sMessage = QString("<pan>%1</pan>").arg(newPan);
          SendToOne(pClient, sMessage);
          return;
      }
  }
}


void
ScoreController::onSetNewTiltValue(const QString& sClientIp, int newTilt) {
  QHostAddress hostAddress(sClientIp);
  for(int i=0; i<connectionList.count(); i++) {
      QWebSocket* pClient = connectionList.at(i).pClientSocket;
      if(pClient->peerAddress().toIPv4Address() == hostAddress.toIPv4Address()) {
          QString sMessage = QString("<tilt>%1</tilt>").arg(newTilt);
          SendToOne(pClient, sMessage);
          return;
      }
  }
}


void
ScoreController::onSetScoreOnly(const QString& sClientIp, bool bScoreOnly) {
#ifdef LOG_VERBOSE
    logMessage(pLogFile,
               Q_FUNC_INFO,
               QString("Client %1 ScoreOnly: %2")
               .arg(sClientIp)
               .arg(bScoreOnly));
#endif
    QHostAddress hostAddress(sClientIp);
    for(int i=0; i<connectionList.count(); i++) {
        QWebSocket* pClient = connectionList.at(i).pClientSocket;
        if(pClient->peerAddress().toIPv4Address() == hostAddress.toIPv4Address()) {
            QString sMessage = QString("<setScoreOnly>%1</setScoreOnly>").arg(bScoreOnly);
            SendToOne(pClient, sMessage);
            return;
        }
    }
}


