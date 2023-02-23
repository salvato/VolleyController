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

#include "paneldirection.h"
#include "panelconfigurator.h"
#include "connection.h"

#include <QObject>
#include <QDialog>
#include <QListWidget>

QT_FORWARD_DECLARE_CLASS(QGroupBox)
QT_FORWARD_DECLARE_CLASS(QListWidgetItem)
QT_FORWARD_DECLARE_CLASS(QSlider)
QT_FORWARD_DECLARE_CLASS(QComboBox)

class ClientListDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ClientListDialog(QList<Connection> connectionList, QWidget *parent=nullptr);
    int exec();
    void remotePanTiltReceived(int newPan, int newTilt);
    void remoteDirectionReceived(PanelDirection currentDirection);
    void remoteScoreOnlyValueReceived(bool bScoreOnly);

public slots:
    void onSetNewPan(int newPan);
    void onSetNewTilt(int newTilt);
    void onStartCamera();
    void onCloseCamera(int);
    void onCloseCamera();
    void onChangePanelDirection(PanelDirection newDirection);
    void onChangeScoreOnly(bool bScoreOnly);

signals:
    void enableVideo(QString sIpAdress);
    void disableVideo();
    void setPanTilt(int newPan, int newTilt);
    void newPanValue(QString sClientIp, int newPan);
    void newTiltValue(QString sClientIp, int newTilt);
    void getDirection(QString sIpAdress);
    void getScoreOnly(QString sIpAdress);
    void changeDirection(QString sIpAdress, PanelDirection newDirection);
    void changeScoreOnly(QString sIpAdress, bool bScoreOnly);

private slots:
    void onClientSelected(QListWidgetItem* selectedClient);

private:
    QGroupBox* createClientListBox();
    QGroupBox* createPanTiltBox();
    QGroupBox* createDirectionBox();

private:
    QWidget           *pMyParent;
    QListWidget       clientListWidget;
    QPushButton       *closeButton{};
    QString            sSelectedClient;
    PanelConfigurator *pConfigurator;

public:
//    void addItem(const QString& sAddress);

};
