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
#include "cameratab.h"

#include <QGridLayout>
#include <QStyle>


CameraTab::CameraTab(QWidget *parent)
    : QWidget(parent)
{
    panMin  = -30;
    panMax  =  30;
    tiltMin = -30;
    tiltMax =  30;

    setupButtons();
    setupButtonEvents();

    auto *mainLayout = new QGridLayout();
    mainLayout->addWidget(&upButton,    0, 1, 1, 1);
    mainLayout->addWidget(&leftButton,  1, 0, 1, 1);
    mainLayout->addWidget(&rightButton, 1, 2, 1, 1);
    mainLayout->addWidget(&downButton,  2, 1, 1, 1);

    setLayout(mainLayout);
}


void
CameraTab::SetCurrentPanTilt(int newPan, int newTilt) {
#ifdef LOG_VERBOSE
    logMessage(nullptr,
               Q_FUNC_INFO,
               QString("Pan: %1 - Tilt: %2")
               .arg(newPan)
               .arg(newTilt));
#endif
    iPan  = newPan;
    iTilt = newTilt;
    leftButton.setDisabled(iPan <= panMin);
    rightButton.setDisabled(iPan >= panMax);
    upButton.setDisabled(iTilt >= tiltMax);
    downButton.setDisabled(iTilt <= tiltMin);
}


void
CameraTab::setupButtons() {
    int iDelay  = 300;
    int iRepeat = 500;

    upButton.setIcon(style()->standardIcon(QStyle::SP_ArrowUp));
    upButton.setAutoRepeat(true);
    upButton.setAutoRepeatDelay(iDelay);
    upButton.setAutoRepeatInterval(iRepeat);

    downButton.setIcon(style()->standardIcon(QStyle::SP_ArrowDown));
    downButton.setAutoRepeat(true);
    downButton.setAutoRepeatDelay(iDelay);
    downButton.setAutoRepeatInterval(iRepeat);

    leftButton.setIcon(style()->standardIcon(QStyle::SP_ArrowLeft));
    leftButton.setAutoRepeat(true);
    leftButton.setAutoRepeatDelay(iDelay);
    leftButton.setAutoRepeatInterval(iRepeat);

    rightButton.setIcon(style()->standardIcon(QStyle::SP_ArrowRight));
    rightButton.setAutoRepeat(true);
    rightButton.setAutoRepeatDelay(iDelay);
    rightButton.setAutoRepeatInterval(iRepeat);

    upButton.setDisabled(true);
    downButton.setDisabled(true);
    leftButton.setDisabled(true);
    rightButton.setDisabled(true);
}


void
CameraTab::setupButtonEvents() {
    connect(&upButton, SIGNAL(clicked()),
            this, SLOT(onUpButton_pressed()));
    connect(&downButton, SIGNAL(clicked()),
            this, SLOT(onDownButton_pressed()));
    connect(&leftButton, SIGNAL(clicked()),
            this, SLOT(onLeftButton_pressed()));
    connect(&rightButton, SIGNAL(clicked()),
            this, SLOT(onRightButton_pressed()));
}


void
CameraTab::onUpButton_pressed() {
    iTilt++;
    downButton.setDisabled(false);
    if(iTilt <= tiltMax)
        emit newTiltValue(iTilt);
    else {
        iTilt = tiltMax;
        upButton.setDisabled(true);
    }
}


void
CameraTab::onDownButton_pressed() {
    iTilt--;
    upButton.setDisabled(false);
    if(iTilt >= tiltMin)
        emit newTiltValue(iTilt);
    else {
        iTilt = tiltMin;
        downButton.setDisabled(true);
    }
}


void
CameraTab::onLeftButton_pressed() {
    iPan--;
    rightButton.setDisabled(false);
    if(iPan >= panMin)
        emit newPanValue(iPan);
    else {
        iPan = panMin;
        leftButton.setDisabled(true);
    }
}


void
CameraTab::onRightButton_pressed() {
    iPan++;
    leftButton.setDisabled(false);
    if(iPan <= panMax)
        emit newPanValue(iPan);
    else {
        iPan = panMax;
        rightButton.setDisabled(true);
    }
}

