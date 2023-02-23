#include <QSettings>
#include <QGridLayout>
#include <QMessageBox>
#include <QResizeEvent>
#include <QApplication>
#include <QDir>
#include <QStandardPaths>

#include "volleycontroller.h"
#include "generalsetupdialog.h"
#include "edit.h"
#include "button.h"


VolleyController::VolleyController()
    : ScoreController()
    , pSettings( new QSettings("Gabriele Salvato", "Volley Controller"))
    , bFontBuilt(false)
{
    // QWidget propagates explicit palette roles from parent to child.
    // If you assign a brush or color to a specific role on a palette and
    // assign that palette to a widget, that role will propagate to all
    // the widget's children, overriding any system defaults for that role.
    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
    panelGradient.setColorAt(0, QColor(0, 0, 16));
    panelGradient.setColorAt(1, QColor(0, 0, 96));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active,   QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);
    panelPalette.setColor(QPalette::WindowText,    Qt::yellow);
    panelPalette.setColor(QPalette::Base,          Qt::black);
    panelPalette.setColor(QPalette::AlternateBase, Qt::blue);
    panelPalette.setColor(QPalette::Text,          Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,    Qt::white);
    setPalette(panelPalette);

    GetSettings();

    prepareDirectories();
    prepareServices();
    pSlideUpdaterServer->setDir(sSlideDir, "*.jpg *.jpeg *.png *.JPG *.JPEG *.PNG");
    emit startSlideServer();
    pSpotUpdaterServer->setDir(sSpotDir, "*.mp4 *.MP4");
    emit startSpotServer();

    buildControls();
    setWindowLayout();

    setEventHandlers();

    service[iServizio ? 1 : 0]->setChecked(true);
    service[iServizio ? 0 : 1]->setChecked(false);
}


void
VolleyController::closeEvent(QCloseEvent *event) {
    SaveSettings();
    event->accept();
}


void
VolleyController::resizeEvent(QResizeEvent *event) {
    if(!bFontBuilt) {
        bFontBuilt = true;
        buildFontSizes();
        event->setAccepted(true);
    }
}


void
VolleyController::GetGeneralSetup() {
    GeneralSetupDialog* pGeneralSetupDialog = new GeneralSetupDialog(&generalSetupArguments);
    int iResult = pGeneralSetupDialog->exec();
    if(iResult == QDialog::Accepted) {
        sSlideDir = generalSetupArguments.sSlideDir;
        if(!sSlideDir.endsWith(QString("/"))) sSlideDir+= QString("/");
        QDir slideDir(sSlideDir);
        if(sSlideDir != QString() && slideDir.exists()) {
            QStringList filter(QStringList() << "*.jpg" << "*.jpeg" << "*.png" << "*.JPG" << "*.JPEG" << "*.PNG");
            slideDir.setNameFilters(filter);
            slideList = slideDir.entryInfoList();
        }
        else {
            sSlideDir = QStandardPaths::displayName(QStandardPaths::GenericDataLocation);
            slideList = QFileInfoList();
        }
#ifdef LOG_MESSAGE
        logMessage(logFile,
                   Q_FUNC_INFO,
                   QString("Found %1 slides").arg(slideList.count()));
#endif
        sSpotDir = generalSetupArguments.sSpotDir;
        if(!sSpotDir.endsWith(QString("/"))) sSpotDir+= QString("/");
        QDir spotDir(sSpotDir);
        if(sSpotDir != QString() && spotDir.exists()) {
            QStringList nameFilter(QStringList() << "*.mp4" << "*.MP4");
            spotDir.setNameFilters(nameFilter);
            spotDir.setFilter(QDir::Files);
            spotList = spotDir.entryInfoList();
        }
        else {
            sSpotDir = QStandardPaths::displayName(QStandardPaths::GenericDataLocation);
            spotList = QFileInfoList();
        }
#ifdef LOG_MESSAGE
        logMessage(logFile,
                   Q_FUNC_INFO,
                   QString("Found %1 spots")
                   .arg(spotList.count()));
#endif
        SaveSettings();
    }
}


void
VolleyController::buildFontSizes() {
    QFont font;
    int iFontSize;
    int hMargin, vMargin;
    QMargins margins;

    font = teamName[0]->font();
    font.setCapitalization(QFont::Capitalize);
    margins = teamName[0]->contentsMargins();
    vMargin = margins.bottom() + margins.top();
    hMargin = margins.left() + margins.right();
    iFontSize = qMin((teamName[0]->width()/teamName[0]->maxLength())-2*hMargin,
                     teamName[0]->height()-vMargin);

    font.setPixelSize(iFontSize);

    font.setPixelSize(iFontSize*0.75);
    timeoutLabel->setFont(font);
    setsLabel->setFont(font);
    serviceLabel->setFont(font);
    scoreLabel->setFont(font);

    font.setWeight(QFont::Black);

    font.setPixelSize(iFontSize);
    teamName[0]->setFont(font);
    teamName[1]->setFont(font);
    setsEdit[0]->setFont(font);
    setsEdit[1]->setFont(font);
    timeoutEdit[0]->setFont(font);
    timeoutEdit[1]->setFont(font);

    font.setPixelSize(2*iFontSize);
    scoreEdit[0]->setFont(font);
    scoreEdit[1]->setFont(font);
}



void
VolleyController::setWindowLayout() {
    QWidget *widget = new QWidget();
    auto *mainLayout = new QGridLayout();

    int gamePanelWidth  = 15;
    int gamePanelHeigth =  8;

    mainLayout->addLayout(CreateGamePanel(),
                          0,
                          0,
                          gamePanelHeigth,
                          gamePanelWidth);

    mainLayout->addLayout(CreateGameButtons(),
                          gamePanelHeigth,
                          0,
                          2,
                          5);

    mainLayout->addLayout(pSpotButtonsLayout,
                          gamePanelHeigth,
                          5,
                          2,
                          gamePanelWidth-5);

    widget->setLayout(mainLayout);
    setCentralWidget(widget);
}


QGridLayout*
VolleyController::CreateGamePanel() {
    auto* gamePanel = new QGridLayout();
    // Team
    int iRow;
    for(int iTeam=0; iTeam<2; iTeam++) {
        // Matrice x righe e 8 colonne
        iRow = 0;
        gamePanel->addWidget(teamName[iTeam], iRow, iTeam*4, 1, 4);
        int iCol = iTeam*5;
        iRow += 1;
        gamePanel->addWidget(timeoutDecrement[iTeam], iRow, iCol,   1, 1, Qt::AlignRight);
        gamePanel->addWidget(timeoutEdit[iTeam],      iRow, iCol+1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(timeoutIncrement[iTeam], iRow, iCol+2, 1, 1, Qt::AlignLeft);
        iRow += 1;
        gamePanel->addWidget(setsDecrement[iTeam], iRow, iCol,   1, 1, Qt::AlignRight);
        gamePanel->addWidget(setsEdit[iTeam],      iRow, iCol+1, 1, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(setsIncrement[iTeam], iRow, iCol+2, 1, 1, Qt::AlignLeft);
        iRow += 1;
        gamePanel->addWidget(service[iTeam],   iRow, iCol, 1, 4, Qt::AlignHCenter|Qt::AlignVCenter);
        iRow += 1;
        gamePanel->addWidget(scoreDecrement[iTeam], iRow, iCol,   2, 1, Qt::AlignRight);
        gamePanel->addWidget(scoreEdit[iTeam],      iRow, iCol+1, 2, 1, Qt::AlignHCenter|Qt::AlignVCenter);
        gamePanel->addWidget(scoreIncrement[iTeam], iRow, iCol+2, 2, 1, Qt::AlignLeft);
        iRow++;
    }

    gamePanel->addWidget(timeoutLabel, 1, 3, 1, 2);
    gamePanel->addWidget(setsLabel, 2, 3, 1, 2);
    gamePanel->addWidget(serviceLabel, 3, 3, 1, 2);
    gamePanel->addWidget(scoreLabel, 4, 3, 2, 2);

    return gamePanel;
}


QHBoxLayout*
VolleyController::CreateGameButtons() {
    auto* gameButtonLayout = new QHBoxLayout();

    QPixmap pixmap(":/buttonIcons/ExchangeVolleyField.png");
    QIcon ButtonIcon(pixmap);
    changeFieldButton = new QPushButton(ButtonIcon, "");
    changeFieldButton->setIconSize(pixmap.rect().size());
    changeFieldButton->setFlat(true);
    changeFieldButton->setToolTip("Inverti Campo");

    pixmap.load(":/buttonIcons/New-Game-Volley.png");
    ButtonIcon.addPixmap(pixmap);
    newGameButton = new QPushButton(ButtonIcon, "");
    newGameButton->setIconSize(pixmap.rect().size());
    newGameButton->setFlat(true);
    newGameButton->setToolTip("Nuova Partita");

    pixmap.load(":/buttonIcons/New-Set-Volley.png");
    ButtonIcon.addPixmap(pixmap);
    newSetButton  = new QPushButton(ButtonIcon, "");
    newSetButton->setIconSize(pixmap.rect().size());
    newSetButton->setFlat(true);
    newSetButton->setToolTip("Nuovo Set");

    gameButtonLayout->addWidget(newGameButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(newSetButton);
    gameButtonLayout->addStretch();
    gameButtonLayout->addWidget(changeFieldButton);
    gameButtonLayout->addStretch();
    return gameButtonLayout;
}


void
VolleyController::GetSettings() {
    generalSetupArguments.maxTimeout       = pSettings->value("volley/maxTimeout", 2).toInt();
    generalSetupArguments.maxSet           = pSettings->value("volley/maxSet", 3).toInt();
    generalSetupArguments.iTimeoutDuration = pSettings->value("volley/TimeoutDuration", 30).toInt();
    generalSetupArguments.sSlideDir        = pSettings->value("directories/slides", sSlideDir).toString();
    generalSetupArguments.sSpotDir         = pSettings->value("directories/spots",  sSpotDir).toString();

    sTeam[0]    = pSettings->value("team1/name", QString(tr("Locali"))).toString();
    sTeam[1]    = pSettings->value("team2/name", QString(tr("Ospiti"))).toString();
    iTimeout[0] = pSettings->value("team1/timeouts", 0).toInt();
    iTimeout[1] = pSettings->value("team2/timeouts", 0).toInt();
    iSet[0]     = pSettings->value("team1/sets", 0).toInt();
    iSet[1]     = pSettings->value("team2/sets", 0).toInt();
    iScore[0]   = pSettings->value("team1/score", 0).toInt();
    iScore[1]   = pSettings->value("team2/score", 0).toInt();
    iServizio   = pSettings->value("set/service", 0).toInt();
    lastService = pSettings->value("set/lastservice", 0).toInt();

    sSlideDir   = generalSetupArguments.sSlideDir;
    sSpotDir    = generalSetupArguments.sSpotDir;

    // Check Stored Values vs Maximum Values
    for(int i=0; i<2; i++) {
        if(iTimeout[i] > generalSetupArguments.maxTimeout)
            iTimeout[i] = generalSetupArguments.maxTimeout;
        if(iSet[i] > generalSetupArguments.maxSet)
            iSet[i] = generalSetupArguments.maxSet;
    }
}


void
VolleyController::SaveStatus() {
    // Save Present Game Values
    pSettings->setValue("team1/name", sTeam[0]);
    pSettings->setValue("team2/name", sTeam[1]);
    pSettings->setValue("team1/timeouts", iTimeout[0]);
    pSettings->setValue("team2/timeouts", iTimeout[1]);
    pSettings->setValue("team1/sets", iSet[0]);
    pSettings->setValue("team2/sets", iSet[1]);
    pSettings->setValue("team1/score", iScore[0]);
    pSettings->setValue("team2/score", iScore[1]);
    pSettings->setValue("set/service", iServizio);
    pSettings->setValue("set/lastservice", lastService);
}


void
VolleyController::SaveSettings() { // Save General Setup Values
    pSettings->setValue("directories/slides",     generalSetupArguments.sSlideDir);
    pSettings->setValue("directories/spots",      generalSetupArguments.sSpotDir);
    pSettings->setValue("volley/maxTimeout",      generalSetupArguments.maxTimeout);
    pSettings->setValue("volley/maxSet",          generalSetupArguments.maxSet);
    pSettings->setValue("volley/TimeoutDuration", generalSetupArguments.iTimeoutDuration);

}


void
VolleyController::buildControls() {
    QString sString;

    QPixmap plusPixmap, minusPixmap;
    QIcon plusButtonIcon, minusButtonIcon;
    plusPixmap.load(":/buttonIcons/Plus.png");
    plusButtonIcon.addPixmap(plusPixmap);
    minusPixmap.load(":/buttonIcons/Minus.png");
    minusButtonIcon.addPixmap(minusPixmap);

    QPalette pal = panelPalette;
    pal.setColor(QPalette::Text, Qt::white);
    for(int iTeam=0; iTeam<2; iTeam++){
        // Teams
        teamName[iTeam] = new Edit(sTeam[iTeam], iTeam);
        teamName[iTeam]->setAlignment(Qt::AlignHCenter);
        teamName[iTeam]->setMaxLength(15);
        pal.setColor(QPalette::Text, Qt::white);
        teamName[iTeam]->setPalette(pal);
        // Timeout
        sString = QString("%1").arg(iTimeout[iTeam], 1);
        timeoutEdit[iTeam] = new Edit(sString, iTeam);
        timeoutEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        timeoutEdit[iTeam]->setMaxLength(1);
        pal.setColor(QPalette::Text, Qt::yellow);
        timeoutEdit[iTeam]->setPalette(pal);
        timeoutEdit[iTeam]->setReadOnly(true);
        // Timeout buttons
        timeoutIncrement[iTeam] = new Button("", iTeam);
        timeoutIncrement[iTeam]->setIcon(plusButtonIcon);
        timeoutIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        timeoutDecrement[iTeam] = new Button("", iTeam);
        timeoutDecrement[iTeam]->setIcon(minusButtonIcon);
        timeoutDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iTimeout[iTeam] == 0)
            timeoutDecrement[iTeam]->setEnabled(false);
        if(iTimeout[iTeam] == generalSetupArguments.maxTimeout) {
            timeoutIncrement[iTeam]->setEnabled(false);
            timeoutEdit[iTeam]->setStyleSheet("background:red;color:white;");
        }
        // Sets
        sString = QString("%1").arg(iSet[iTeam], 1);
        setsEdit[iTeam] = new Edit(sString, iTeam);
        setsEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        setsEdit[iTeam]->setMaxLength(1);
        setsEdit[iTeam]->setPalette(pal);
        setsEdit[iTeam]->setReadOnly(true);
        // Set buttons
        setsIncrement[iTeam] = new Button("", iTeam);
        setsIncrement[iTeam]->setIcon(plusButtonIcon);
        setsIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        setsDecrement[iTeam] = new Button("", iTeam);
        setsDecrement[iTeam]->setIcon(minusButtonIcon);
        setsDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iSet[iTeam] == 0)
            setsDecrement[iTeam]->setEnabled(false);
        if(iSet[iTeam] == generalSetupArguments.maxSet)
            setsIncrement[iTeam]->setEnabled(false);
        // Service
        QPixmap pixmap(":/buttonIcons/ball.png");
        QIcon ButtonIcon(pixmap);
        service[iTeam] = new Button("", iTeam);
        service[iTeam]->setIcon(ButtonIcon);
        auto const rec = QApplication::primaryScreen()->availableSize();
        auto const height = rec.height();
        service[iTeam]->setIconSize(QSize(height/16,height/16));
        service[iTeam]->setCheckable(true);
        service[iTeam]->setStyleSheet("QPushButton:checked { background-color: rgb(0, 0, 160) }");
        // Score
        scoreLabel = new QLabel(tr("Punti"));
        scoreLabel->setAlignment(Qt::AlignRight|Qt::AlignHCenter);
        sString = QString("%1").arg(iScore[iTeam], 2);
        scoreEdit[iTeam] = new Edit(sString, iTeam);
        scoreEdit[iTeam]->setAlignment(Qt::AlignHCenter);
        scoreEdit[iTeam]->setMaxLength(2);
        scoreEdit[iTeam]->setPalette(pal);
        scoreEdit[iTeam]->setReadOnly(true);
        // Score buttons
        scoreIncrement[iTeam] = new Button("", iTeam);
        scoreIncrement[iTeam]->setIcon(plusButtonIcon);
        scoreIncrement[iTeam]->setIconSize(plusPixmap.rect().size());
        scoreDecrement[iTeam] = new Button("", iTeam);
        scoreDecrement[iTeam]->setIcon(minusButtonIcon);
        scoreDecrement[iTeam]->setIconSize(minusPixmap.rect().size());
        if(iScore[iTeam] == 0)
            scoreDecrement[iTeam]->setEnabled(false);
    }
    // Timeout
    timeoutLabel = new QLabel(tr("Timeout"));
    timeoutLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Set
    setsLabel = new QLabel(tr("Set"));
    setsLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Service
    serviceLabel = new QLabel(tr("Servizio"));
    serviceLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
    // Score
    scoreLabel = new QLabel(tr("Punti"));
    scoreLabel->setAlignment(Qt::AlignHCenter|Qt::AlignVCenter);
}


QString
VolleyController::FormatStatusMsg() {
    QString sMessage = tr("");
    QString sTemp;
    for(int i=0; i<2; i++) {
        sTemp = QString("<team%1>%2</team%3>").arg(i,1).arg(sTeam[i].toLocal8Bit().data()).arg(i,1);
        sMessage += sTemp;
        sTemp = QString("<timeout%1>%2</timeout%3>").arg(i,1).arg(iTimeout[i]).arg(i,1);
        sMessage += sTemp;
        sTemp = QString("<set%1>%2</set%3>").arg(i,1).arg(iSet[i]).arg(i,1);
        sMessage += sTemp;
        sTemp = QString("<score%1>%2</score%3>").arg(i,1).arg(iScore[i], 2).arg(i,1);
        sMessage += sTemp;
    }
    sTemp = QString("<servizio>%1</servizio>").arg(iServizio, 1);
    sMessage += sTemp;
    if(myStatus == showSlides)
        sMessage += QString("<slideshow>1</slideshow>");
    else if(myStatus == showCamera)
        sMessage += QString("<live>1</live>");
    else if(myStatus == showSpots)
        sMessage += QString("<spotloop>1</spotloop>");

//>>>>>>>>>>>>    sMessage += QString("<language>%1</language>").arg(sLanguage);
    sMessage += QString("<language>%1</language>").arg("Italiano");
    return sMessage;
}


void
VolleyController::setEventHandlers() {
    for(int iTeam=0; iTeam <2; iTeam++) {
        connect(teamName[iTeam], SIGNAL(teamTextChanged(QString,int)),
                this, SLOT(onTeamTextChanged(QString,int)));
        connect(timeoutIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutIncrement(int)));
        connect(timeoutIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(timeoutDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onTimeOutDecrement(int)));
        connect(timeoutDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(setsIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onSetIncrement(int)));
        connect(setsIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(setsDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onSetDecrement(int)));
        connect(setsDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(service[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onServiceClicked(int)));
        connect(service[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(scoreIncrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreIncrement(int)));
        connect(scoreIncrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
        connect(scoreDecrement[iTeam], SIGNAL(buttonClicked(int)),
                this, SLOT(onScoreDecrement(int)));
        connect(scoreDecrement[iTeam], SIGNAL(clicked()),
                pButtonClick, SLOT(play()));
    }
    // New Set
    connect(newSetButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewSetClicked()));
    connect(newSetButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
    // New Game
    connect(newGameButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonNewGameClicked()));
    connect(newGameButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
    // Exchange Field Position
    connect(changeFieldButton, SIGNAL(clicked(bool)),
            this, SLOT(onButtonChangeFieldClicked()));
    connect(changeFieldButton, SIGNAL(clicked()),
            pButtonClick, SLOT(play()));
}


// =========================
// Event management routines
// =========================

void
VolleyController::onTimeOutIncrement(int iTeam) {
    QString sMessage;
    iTimeout[iTeam]++;
    if(iTimeout[iTeam] >= generalSetupArguments.maxTimeout) {
        timeoutIncrement[iTeam]->setEnabled(false);
        timeoutEdit[iTeam]->setStyleSheet("background:red;color:white;");
    }
    timeoutDecrement[iTeam]->setEnabled(true);
    sMessage = QString("<timeout%1>%2</timeout%3>")
              .arg(iTeam, 1)
              .arg(iTimeout[iTeam]).arg(iTeam, 1);
    SendToAll(sMessage);
    sMessage = QString("<startTimeout>%1</startTimeout>")
               .arg(generalSetupArguments.iTimeoutDuration);
    SendToAll(sMessage);
    QString sText;
    sText = QString("%1").arg(iTimeout[iTeam]);
    timeoutEdit[iTeam]->setText(sText);
    sText = QString("team%1/timeouts").arg(iTeam+1, 1);
    pSettings->setValue(sText, iTimeout[iTeam]);
}


void
VolleyController::onTimeOutDecrement(int iTeam) {
    QString sMessage;
    iTimeout[iTeam]--;
    if(iTimeout[iTeam] == 0) {
        timeoutDecrement[iTeam]->setEnabled(false);
    }
    timeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow;");
    timeoutIncrement[iTeam]->setEnabled(true);
    sMessage = QString("<timeout%1>%2</timeout%3>").arg(iTeam, 1).arg(iTimeout[iTeam]).arg(iTeam, 1);
    SendToAll(sMessage);
    sMessage = QString("<stopTimeout>1</stopTimeout>");
    SendToAll(sMessage);
    QString sText;
    sText = QString("%1").arg(iTimeout[iTeam], 1);
    timeoutEdit[iTeam]->setText(sText);
    sText = QString("team%1/timeouts").arg(iTeam+1, 1);
    pSettings->setValue(sText, iTimeout[iTeam]);
}


void
VolleyController::onSetIncrement(int iTeam) {
    QString sMessage;
    iSet[iTeam]++;
    setsDecrement[iTeam]->setEnabled(true);
    if(iSet[iTeam] == generalSetupArguments.maxSet) {
        setsIncrement[iTeam]->setEnabled(false);
    }
    sMessage = QString("<set%1>%2</set%3>").arg(iTeam, 1).arg(iSet[iTeam]).arg(iTeam, 1);
    SendToAll(sMessage);
    QString sText;
    sText = QString("%1").arg(iSet[iTeam], 1);
    setsEdit[iTeam]->setText(sText);
    sText = QString("team%1/sets").arg(iTeam+1, 1);
    pSettings->setValue(sText, iSet[iTeam]);
}


void
VolleyController::onSetDecrement(int iTeam) {
    QString sMessage;
    iSet[iTeam]--;
    setsIncrement[iTeam]->setEnabled(true);
    if(iSet[iTeam] == 0) {
       setsDecrement[iTeam]->setEnabled(false);
    }
    sMessage= QString("<set%1>%2</set%3>").arg(iTeam, 1).arg(iSet[iTeam]).arg(iTeam, 1);
    SendToAll(sMessage);
    QString sText;
    sText = QString("%1").arg(iSet[iTeam], 1);
    setsEdit[iTeam]->setText(sText);
    sText = QString("team%1/sets").arg(iTeam+1, 1);
    pSettings->setValue(sText, iSet[iTeam]);
}


void
VolleyController::onServiceClicked(int iTeam) {
    QString sMessage;
    iServizio = iTeam;
    lastService = iServizio;
    service[iServizio ? 1 : 0]->setChecked(true);
    service[iServizio ? 0 : 1]->setChecked(false);
    sMessage = QString("<servizio>%1</servizio>").arg(iServizio);
    SendToAll(sMessage);
    pSettings->setValue("set/service", iServizio);
    pSettings->setValue("set/lastservice", lastService);
}


void
VolleyController::onScoreIncrement(int iTeam) {
    QString sMessage;
    iScore[iTeam]++;
    scoreDecrement[iTeam]->setEnabled(true);
    if(iScore[iTeam] > 98) {
      scoreIncrement[iTeam]->setEnabled(false);
    }
    lastService = iServizio;
    iServizio = iTeam;
    service[iServizio ? 1 : 0]->setChecked(true);
    service[iServizio ? 0 : 1]->setChecked(false);
    sMessage = QString("<score%1>%2</score%3><servizio>%4</servizio>")
               .arg(iTeam, 1)
               .arg(iScore[iTeam], 2)
               .arg(iTeam, 1)
               .arg(iServizio, 1);
    SendToAll(sMessage);
    QString sText;
    sText = QString("%1").arg(iScore[iTeam], 2);
    scoreEdit[iTeam]->setText(sText);
    sText = QString("team%1/score").arg(iTeam+1, 1);
    pSettings->setValue(sText, iScore[iTeam]);
}


void
VolleyController::onScoreDecrement(int iTeam) {
    QString sMessage;
    iScore[iTeam]--;
    scoreIncrement[iTeam]->setEnabled(true);
    if(iScore[iTeam] == 0) {
      scoreDecrement[iTeam]->setEnabled(false);
    }
    iServizio = lastService;
    service[iServizio ? 1 : 0]->setChecked(true);
    service[iServizio ? 0 : 1]->setChecked(false);
    sMessage = QString("<score%1>%2</score%3><servizio>%4</servizio>")
               .arg(iTeam, 1)
               .arg(iScore[iTeam], 2)
               .arg(iTeam, 1)
               .arg(iServizio, 1);
    SendToAll(sMessage);
    QString sText;
    sText = QString("%1").arg(iScore[iTeam], 2);
    scoreEdit[iTeam]->setText(sText);
    sText = QString("team%1/score").arg(iTeam+1, 1);
    pSettings->setValue(sText, iScore[iTeam]);
}


void
VolleyController::onTeamTextChanged(QString sText, int iTeam) {
    QString sMessage;
    sTeam[iTeam] = sText;
    if(sText=="")// C'è un problema con la stringa vuota...
        sMessage = QString("<team%1>-</team%1>").arg(iTeam, 1).arg(iTeam, 1);
    else
        sMessage = QString("<team%1>%2</team%3>").arg(iTeam, 1).arg(sTeam[iTeam].toLocal8Bit().data()).arg(iTeam, 1);
    SendToAll(sMessage);
    sText = QString("team%1/name").arg(iTeam+1, 1);
    pSettings->setValue(sText, sTeam[iTeam]);
}


void
VolleyController::onButtonChangeFieldClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Scambiare il campo delle squadre ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    QString sText = sTeam[0];
    sTeam[0] = sTeam[1];
    sTeam[1] = sText;
    teamName[0]->setText(sTeam[0]);
    teamName[1]->setText(sTeam[1]);

    int iVal = iSet[0];
    iSet[0] = iSet[1];
    iSet[1] = iVal;
    sText = QString("%1").arg(iSet[0], 1);
    setsEdit[0]->setText(sText);
    sText = QString("%1").arg(iSet[1], 1);
    setsEdit[1]->setText(sText);

    iVal = iScore[0];
    iScore[0] = iScore[1];
    iScore[1] = iVal;
    sText = QString("%1").arg(iScore[0], 2);
    scoreEdit[0]->setText(sText);
    sText = QString("%1").arg(iScore[1], 2);
    scoreEdit[1]->setText(sText);

    iVal = iTimeout[0];
    iTimeout[0] = iTimeout[1];
    iTimeout[1] = iVal;
    sText = QString("%1").arg(iTimeout[0]);
    timeoutEdit[0]->setText(sText);
    sText = QString("%1").arg(iTimeout[1]);
    timeoutEdit[1]->setText(sText);

    iServizio = 1 - iServizio;
    lastService = 1 -lastService;

    service[iServizio ? 1 : 0]->setChecked(true);
    service[iServizio ? 0 : 1]->setChecked(false);

    for(int iTeam=0; iTeam<2; iTeam++) {
        scoreDecrement[iTeam]->setEnabled(true);
        scoreIncrement[iTeam]->setEnabled(true);
        if(iScore[iTeam] == 0) {
          scoreDecrement[iTeam]->setEnabled(false);
        }
        if(iScore[iTeam] > 98) {
          scoreIncrement[iTeam]->setEnabled(false);
        }

        setsDecrement[iTeam]->setEnabled(true);
        setsIncrement[iTeam]->setEnabled(true);
        if(iSet[iTeam] == 0) {
            setsDecrement[iTeam]->setEnabled(false);
        }
        if(iSet[iTeam] == generalSetupArguments.maxSet) {
            setsIncrement[iTeam]->setEnabled(false);
        }

        timeoutIncrement[iTeam]->setEnabled(true);
        timeoutDecrement[iTeam]->setEnabled(true);
        timeoutEdit[iTeam]->setStyleSheet("background-color: rgba(0, 0, 0, 0);color:yellow;");
        if(iTimeout[iTeam] == generalSetupArguments.maxTimeout) {
            timeoutIncrement[iTeam]->setEnabled(false);
            timeoutEdit[iTeam]->setStyleSheet("background:red;color:white;");
        }
        if(iTimeout[iTeam] == 0) {
            timeoutDecrement[iTeam]->setEnabled(false);
        }
    }
    SendToAll(FormatStatusMsg());
    SaveStatus();
}


void
VolleyController::onButtonNewSetClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Vuoi davvero iniziare un nuovo Set ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;

    // Exchange team's order in the field
    QString sText = sTeam[0];
    sTeam[0] = sTeam[1];
    sTeam[1] = sText;
    teamName[0]->setText(sTeam[0]);
    teamName[1]->setText(sTeam[1]);
    int iVal = iSet[0];
    iSet[0] = iSet[1];
    iSet[1] = iVal;
    sText = QString("%1").arg(iSet[0], 1);
    setsEdit[0]->setText(sText);
    sText = QString("%1").arg(iSet[1], 1);
    setsEdit[1]->setText(sText);
    for(int iTeam=0; iTeam<2; iTeam++) {
        iTimeout[iTeam] = 0;
        sText = QString("%1").arg(iTimeout[iTeam], 1);
        timeoutEdit[iTeam]->setText(sText);
        timeoutEdit[iTeam]->setStyleSheet(styleSheet());
        iScore[iTeam]   = 0;
        sText = QString("%1").arg(iScore[iTeam], 2);
        scoreEdit[iTeam]->setText(sText);
        timeoutDecrement[iTeam]->setEnabled(false);
        timeoutIncrement[iTeam]->setEnabled(true);
        setsDecrement[iTeam]->setEnabled(iSet[iTeam] != 0);
        setsIncrement[iTeam]->setEnabled(true);
        scoreDecrement[iTeam]->setEnabled(false);
        scoreIncrement[iTeam]->setEnabled(true);
    }
    iServizio   = 0;
    lastService = 0;
    service[iServizio ? 1 : 0]->setChecked(true);
    service[iServizio ? 0 : 1]->setChecked(false);
    SendToAll(FormatStatusMsg());
    SaveStatus();
}


void
VolleyController::onButtonNewGameClicked() {
    int iRes = QMessageBox::question(this, tr("Volley_Controller"),
                                     tr("Iniziare una Nuova Partita ?"),
                                     QMessageBox::Yes | QMessageBox::No,
                                     QMessageBox::No);
    if(iRes != QMessageBox::Yes) return;
    sTeam[0]    = tr("Locali");
    sTeam[1]    = tr("Ospiti");
    QString sText;
    for(int iTeam=0; iTeam<2; iTeam++) {
        teamName[iTeam]->setText(sTeam[iTeam]);
        iTimeout[iTeam] = 0;
        sText = QString("%1").arg(iTimeout[iTeam], 1);
        timeoutEdit[iTeam]->setText(sText);
        timeoutEdit[iTeam]->setStyleSheet(styleSheet());
        iSet[iTeam]   = 0;
        sText = QString("%1").arg(iSet[iTeam], 1);
        setsEdit[iTeam]->setText(sText);
        iScore[iTeam]   = 0;
        sText = QString("%1").arg(iScore[iTeam], 2);
        scoreEdit[iTeam]->setText(sText);
        timeoutDecrement[iTeam]->setEnabled(false);
        timeoutIncrement[iTeam]->setEnabled(true);
        setsDecrement[iTeam]->setEnabled(false);
        setsIncrement[iTeam]->setEnabled(true);
        scoreDecrement[iTeam]->setEnabled(false);
        scoreIncrement[iTeam]->setEnabled(true);
    }
    iServizio   = 0;
    lastService = 0;
    service[iServizio ? 1 : 0]->setChecked(true);
    service[iServizio ? 0 : 1]->setChecked(false);
    SendToAll(FormatStatusMsg());
    SaveStatus();
}

