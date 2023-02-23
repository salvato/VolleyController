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
#pragma once

#include <QDialog>

#include "paneldirection.h"
#include "paneltab.h"
#include "cameratab.h"


QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)
QT_FORWARD_DECLARE_CLASS(QTabWidget)

class PanelConfigurator : public QDialog
{
    Q_OBJECT

public:
    explicit PanelConfigurator(QWidget *parent = nullptr);
    void setClient(QString sClient);
    void SetCurrentPanTilt(int newPan, int newTilt);
    void SetCurrrentOrientaton(PanelDirection newDirection);
    void SetIsScoreOnly(bool bScoreOnly);

signals:
    void changeDirection(PanelDirection newDirection);
    void changeScoreOnly(bool bScoreOnly);
    void newPanValue(int newValue);
    void newTiltValue(int newValue);
    void startCamera();
    void stopCamera();

private:

public slots:
    // tabWidget event
    void onChangedTab(int iTabIndex);
    // PanelTab events
    void onChangeDirection(PanelDirection newDirection);
    void onChangeScoreOnly(bool bScoreOnly);
    // CameraTab events
    void onChangeTiltValue(int);
    void onChangePanValue(int);

private slots:

private:
    QTabWidget       *pTabWidget;
    QDialogButtonBox *pButtonBox;
    PanelTab         *pPanelTab;
    CameraTab        *pCameraTab;
    QString           sCurrentClient;
    int               iCameraTab;
};

