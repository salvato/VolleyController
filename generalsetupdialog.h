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
#include <QLineEdit>
#include <QPushButton>
#include <QLabel>

#include "generalsetuparguments.h"


QT_FORWARD_DECLARE_CLASS(QDialogButtonBox)


class GeneralSetupDialog : public QDialog
{
    Q_OBJECT
public:
    explicit GeneralSetupDialog(GeneralSetupArguments* pArguments);

public slots:
    void onSelectSlideDir();
    void onSelectSpotDir();
    void onOk();
    void onCancel();

protected:
    void setSlideDir(const QString& sDir);
    void setSpotDir(const QString& sDir);

private:
    QPalette    panelPalette;
    QGradient   panelGradient;
    QBrush      panelBrush;
    QLineEdit   numTimeoutEdit;
    QLineEdit   maxSetEdit;
    QLineEdit   timeoutDurationEdit;

    QLineEdit   slidesDirEdit;
    QLineEdit   spotsDirEdit;
    QPushButton buttonSelectSlidesDir;
    QPushButton buttonSelectSpotsDir;
    QPushButton buttonOk;
    QPushButton buttonCancel;

    GeneralSetupArguments* pTempArguments;
};

