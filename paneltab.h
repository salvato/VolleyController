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
#ifndef PANELTAB_H
#define PANELTAB_H

#include "paneldirection.h"

#include <QWidget>
#include <QLabel>
#include <QComboBox>
#include <QCheckBox>

class PanelTab : public QWidget
{
    Q_OBJECT
public:
    explicit PanelTab(QWidget *parent = nullptr);
    void setDirection(PanelDirection newDirection);
    void setScoreOnly(bool bScoreOnly);

signals:
    void getDirection(QString sIpAdress);
    void getScoreOnly(QString sIpAdress);
    void changeDirection(PanelDirection newDirection);
    void changeScoreOnly(bool bScoreOnly);

public slots:
    void onChangePanelDirection(int iDirection);
    void onChangeScoreOnly(int newState);

private:
    QLabel labelDirection;
    QComboBox directionCombo;
    QCheckBox scoreOnlyCheckBox;
};

#endif // PANELTAB_H
