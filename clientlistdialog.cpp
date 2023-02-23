/*
 *
Copyright (C) 2023  Gabriele Salvato

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

#include <QGridLayout>
#include <QVBoxLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QListWidget>
#include <QListWidgetItem>
#include <QComboBox>
#include <QMessageBox>
#include <QAbstractItemView>

#include "clientlistdialog.h"


ClientListDialog::ClientListDialog(QList<Connection> _connectionList, QWidget* parent)
    : QDialog(parent)
    , pMyParent(parent)
{
    for(int i=0; i<_connectionList.count(); i++) {
        clientListWidget.addItem(_connectionList.at(i).pClientSocket->peerAddress().toString());
    }
    clientListWidget.setFont(QFont("Arial", 24));
    clientListWidget.setSelectionMode(QAbstractItemView::SingleSelection);
    connect(&clientListWidget, SIGNAL(itemClicked(QListWidgetItem*)),
            this, SLOT(onClientSelected(QListWidgetItem*)));

    auto*  mainLayout = new QGridLayout();
    mainLayout->addWidget(createClientListBox(),  0,  0, 10, 10);
    setLayout(mainLayout);
    connect(this, SIGNAL(finished(int)),
            this, SLOT(onCloseCamera(int)));

    pConfigurator = new PanelConfigurator(this);

    // CameraTab forwarded signals
    connect(pConfigurator, SIGNAL(newPanValue(int)),
            this, SLOT(onSetNewPan(int)));
    connect(pConfigurator, SIGNAL(startCamera()),
            this, SLOT(onStartCamera()));
    connect(pConfigurator, SIGNAL(stopCamera()),
            this, SLOT(onCloseCamera()));
    connect(pConfigurator, SIGNAL(newTiltValue(int)),
            this, SLOT(onSetNewTilt(int)));
    // PanelTab forwarded signals
    connect(pConfigurator, SIGNAL(changeDirection(PanelDirection)),
            this, SLOT(onChangePanelDirection(PanelDirection)));
    connect(pConfigurator, SIGNAL(changeScoreOnly(bool)),
            this, SLOT(onChangeScoreOnly(bool)));

}


QGroupBox*
ClientListDialog::createClientListBox() {
    auto* clientListBox = new QGroupBox();
    auto* clientListLayout = new QGridLayout();
    closeButton = new QPushButton(tr("Chiudi"));
    connect(closeButton, SIGNAL(clicked(bool)), this, SLOT(accept()));

    clientListBox->setTitle(tr("Pannelli Connessi"));

    clientListLayout->addWidget(&clientListWidget, 0, 0, 6, 3);
    clientListLayout->addWidget(closeButton, 6, 1, 1, 1);
    clientListBox->setLayout(clientListLayout);
    return clientListBox;
}


void
ClientListDialog::onStartCamera() {
    emit enableVideo(sSelectedClient);
}


void
ClientListDialog::onCloseCamera(int) {
    emit disableVideo();
}


void
ClientListDialog::onCloseCamera() {
    emit disableVideo();
}


void
ClientListDialog::onClientSelected(QListWidgetItem* selectedClient) {
    emit disableVideo();
    sSelectedClient = selectedClient->text();

    pConfigurator->setClient(sSelectedClient);
    pConfigurator->show();
    emit getDirection(sSelectedClient);
    emit getScoreOnly(sSelectedClient);
}


void
ClientListDialog::onSetNewPan(int newPan) {
    emit newPanValue(sSelectedClient, newPan);
}


void
ClientListDialog::onSetNewTilt(int newTilt) {
    emit newTiltValue(sSelectedClient, newTilt);
}


void
ClientListDialog::remotePanTiltReceived(int newPan, int newTilt) {
    pConfigurator->SetCurrentPanTilt(newPan, newTilt);
}


void
ClientListDialog::remoteDirectionReceived(PanelDirection currentDirection) {
    pConfigurator->SetCurrrentOrientaton(currentDirection);
}


void
ClientListDialog::onChangePanelDirection(PanelDirection newDirection) {
    emit changeDirection(sSelectedClient, newDirection);
}


void
ClientListDialog::remoteScoreOnlyValueReceived(bool bScoreOnly) {
    pConfigurator->SetIsScoreOnly(bScoreOnly);
}


void
ClientListDialog::onChangeScoreOnly(bool bScoreOnly) {
    #ifdef LOG_VERBOSE
        logMessage(nullptr,
                   Q_FUNC_INFO,
                   QString("ScoreOnly: %2")
                   .arg(bScoreOnly));
    #endif
    emit changeScoreOnly(sSelectedClient, bScoreOnly);
}


int
ClientListDialog::exec() {
  clientListWidget.clearSelection();
  return QDialog::exec();
}
