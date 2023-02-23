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
#ifndef CAMERATAB_H
#define CAMERATAB_H

#include <QWidget>
#include <QPushButton>

class CameraTab : public QWidget
{
    Q_OBJECT
public:
    explicit CameraTab(QWidget *parent = nullptr);
    void SetCurrentPanTilt(int newPan, int newTilt);

protected:
    void setupButtons();
    void setupButtonEvents();

signals:
    void newTiltValue(int);
    void newPanValue(int);

public slots:
    void onUpButton_pressed();
    void onDownButton_pressed();
    void onLeftButton_pressed();
    void onRightButton_pressed();

private:
    QPushButton downButton;
    QPushButton upButton;
    QPushButton leftButton;
    QPushButton rightButton;

    int iTilt;
    int iPan;
    int panMin;
    int panMax;
    int tiltMin;
    int tiltMax;
};

#endif // CAMERATAB_H
