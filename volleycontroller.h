#pragma once

#include <scorecontroller.h>


QT_FORWARD_DECLARE_CLASS(QSettings)
QT_FORWARD_DECLARE_CLASS(Edit)
QT_FORWARD_DECLARE_CLASS(myLabel)
QT_FORWARD_DECLARE_CLASS(Button)
QT_FORWARD_DECLARE_CLASS(QLabel)
QT_FORWARD_DECLARE_CLASS(QGridLayout)


class VolleyController : public ScoreController
{
    Q_OBJECT
public:
    VolleyController();
    void resizeEvent(QResizeEvent *event);

protected:
    void          GetSettings();
    void          SaveSettings();
    void          SaveStatus();
    void          GetGeneralSetup();
    void          setWindowLayout();
    QGridLayout*  CreateGamePanel();
    QHBoxLayout*  CreateGameButtons();
    void          buildFontSizes();

private slots:
    void closeEvent(QCloseEvent*);
    void onTimeOutIncrement(int iTeam);
    void onTimeOutDecrement(int iTeam);
    void onSetIncrement(int iTeam);
    void onSetDecrement(int iTeam);
    void onScoreIncrement(int iTeam);
    void onScoreDecrement(int iTeam);
    void onServiceClicked(int iTeam);
    void onTeamTextChanged(QString sText, int iTeam);
    void onButtonChangeFieldClicked();
    void onButtonNewSetClicked();
    void onButtonNewGameClicked();

private:
    void          buildControls();
    void          setEventHandlers();
    QString       FormatStatusMsg();

protected:
    QSettings    *pSettings;

private:
    QString       sTeam[2];
    int           iTimeout[2]{};
    int           iSet[2]{};
    int           iScore[2]{};
    int           iServizio{};
    int           lastService{};
    Edit         *teamName[2]{};
    Edit         *timeoutEdit[2]{};
    Edit         *setsEdit[2]{};
    Edit         *scoreEdit[2]{};
    Button       *timeoutIncrement[2]{};
    Button       *timeoutDecrement[2]{};
    Button       *setsIncrement[2]{};
    Button       *setsDecrement[2]{};
    Button       *scoreIncrement[2]{};
    Button       *scoreDecrement[2]{};
    Button       *service[2]{};
    QLabel       *timeoutLabel{};
    QLabel       *setsLabel{};
    QLabel       *serviceLabel{};
    QLabel       *scoreLabel{};

    QPushButton  *newSetButton{};
    QPushButton  *newGameButton{};
    QPushButton  *changeFieldButton{};
    bool          bFontBuilt;
    QPalette           panelPalette;
    QLinearGradient    panelGradient;
    QBrush             panelBrush;
};

