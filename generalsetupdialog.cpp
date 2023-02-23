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
#include "generalsetupdialog.h"
#include <QTabWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QDir>
#include <QApplication>
#include <QLabel>
#include <QFrame>
#include <QFileDialog>
#include <QStandardPaths>


/*!
 * \brief GeneralSetupDialog::GeneralSetupDialog
 * \param parent
 */
GeneralSetupDialog::GeneralSetupDialog(GeneralSetupArguments* pArguments)
    : QDialog()
    , pTempArguments(pArguments)
{
    panelPalette = QWidget::palette();
    panelGradient = QLinearGradient(0.0, 0.0, 0.0, height());
    panelGradient.setColorAt(0, QColor(0, 0, 16));
    panelGradient.setColorAt(1, QColor(0, 0, 96));
    panelBrush = QBrush(panelGradient);
    panelPalette.setBrush(QPalette::Active, QPalette::Window, panelBrush);
    panelPalette.setBrush(QPalette::Inactive, QPalette::Window, panelBrush);
    panelPalette.setColor(QPalette::WindowText,    Qt::yellow);
    panelPalette.setColor(QPalette::Base,          Qt::black);
    panelPalette.setColor(QPalette::AlternateBase, Qt::blue);
    panelPalette.setColor(QPalette::Text,          Qt::yellow);
    panelPalette.setColor(QPalette::BrightText,    Qt::white);
    setPalette(panelPalette);

    buttonOk.setText("OK");
    buttonCancel.setText("Cancel");
    connect(&buttonOk,     SIGNAL(clicked()), this, SLOT(onOk()));
    connect(&buttonCancel, SIGNAL(clicked()), this, SLOT(onCancel()));

    slidesDirEdit.setReadOnly(true);
    spotsDirEdit.setReadOnly(true);
    slidesDirEdit.setStyleSheet("background:red;color:white;");
    spotsDirEdit.setStyleSheet("background:red;color:white;");
    setSlideDir(pArguments->sSlideDir);
    setSpotDir(pArguments->sSpotDir);

    QLabel* pSlidesPathLabel = new QLabel(tr("Slides folder:"));
    QLabel* pSpotsPathLabel  = new QLabel(tr("Movies folder:"));

    buttonSelectSlidesDir.setText("Change");
    buttonSelectSpotsDir.setText("Change");
    connect(&buttonSelectSlidesDir, SIGNAL(clicked()),
            this, SLOT(onSelectSlideDir()));
    connect(&buttonSelectSpotsDir, SIGNAL(clicked()),
            this, SLOT(onSelectSpotDir()));

    QLabel* pNumTimeoutLabel      = new QLabel(tr("Max Timeouts:"));
    QLabel* pMaxSetLabel          = new QLabel(tr("Max Sets:"));
    QLabel* pTimeoutDurationLabel = new QLabel(tr("Timeout sec:"));

    numTimeoutEdit.setMaxLength(1);
    maxSetEdit.setMaxLength(1);
    timeoutDurationEdit.setMaxLength(2);

    numTimeoutEdit.setStyleSheet("background:white;color:black;");
    maxSetEdit.setStyleSheet("background:white;color:black;");
    timeoutDurationEdit.setStyleSheet("background:white;color:black;");

    numTimeoutEdit.setText(QString("%1").arg(pArguments->maxTimeout));
    maxSetEdit.setText(QString("%1").arg(pArguments->maxSet));
    timeoutDurationEdit.setText(QString("%1").arg(pArguments->iTimeoutDuration));

    QGridLayout* pMainLayout = new QGridLayout;

    pMainLayout->addWidget(pSlidesPathLabel,        0, 0, 1, 1);
    pMainLayout->addWidget(&slidesDirEdit,          0, 1, 1, 6);
    pMainLayout->addWidget(&buttonSelectSlidesDir,  0, 7, 1, 1);

    pMainLayout->addWidget(pSpotsPathLabel,         1, 0, 1, 1);
    pMainLayout->addWidget(&spotsDirEdit,           1, 1, 1, 6);
    pMainLayout->addWidget(&buttonSelectSpotsDir,   1, 7, 1, 1);

    pMainLayout->addWidget(pNumTimeoutLabel,        2, 0, 1, 3);
    pMainLayout->addWidget(&numTimeoutEdit,         2, 3, 1, 1);

    pMainLayout->addWidget(pMaxSetLabel,            3, 0, 1, 3);
    pMainLayout->addWidget(&maxSetEdit,             3, 3, 1, 1);

    pMainLayout->addWidget(pTimeoutDurationLabel,   4, 0, 1, 3);
    pMainLayout->addWidget(&timeoutDurationEdit,    4, 3, 1, 1);

    pMainLayout->addWidget(&buttonCancel,           5, 6, 1, 1);
    pMainLayout->addWidget(&buttonOk,               5, 7, 1, 1);

    setLayout(pMainLayout);

#ifdef Q_OS_ANDROID
    setWindowFlags(Qt::Window);
    setAttribute(Qt::WA_DeleteOnClose,true);
#endif
}


void
GeneralSetupDialog::setSlideDir(const QString& sDir) {
    slidesDirEdit.setText(sDir);
    QDir slideDir(sDir);
    if(slideDir.exists()) {
        slidesDirEdit.setStyleSheet("background:white;color:black;");
    }
    else {
        slidesDirEdit.setStyleSheet("background:red;color:white;");
    }
}


void
GeneralSetupDialog::setSpotDir(const QString& sDir){
    spotsDirEdit.setText(sDir);
    QDir spotDir(sDir);
    if(spotDir.exists()) {
        spotsDirEdit.setStyleSheet("background:white;color:black;");
    }
    else {
        spotsDirEdit.setStyleSheet("background:red;color:white;");
    }
}


void
GeneralSetupDialog::onSelectSlideDir() {
    QString sSlideDir = slidesDirEdit.text();
    QDir slideDir = QDir(sSlideDir);
    if(slideDir.exists()) {
        sSlideDir = QFileDialog::getExistingDirectory(this,
                                                      "Slide Directory",
                                                      sSlideDir);
    }
    else {
        sSlideDir = QFileDialog::getExistingDirectory(this,
                                                      "Slide Directory",
                                                      QStandardPaths::displayName(QStandardPaths::GenericDataLocation));
    }
    if(sSlideDir == QString()) return; // "Cancel" has been pressed...
    if(!sSlideDir.endsWith(QString("/"))) sSlideDir+= QString("/");
    slidesDirEdit.setText(sSlideDir);
    slideDir.setPath(sSlideDir);
    setSlideDir(sSlideDir);
}


void
GeneralSetupDialog::onSelectSpotDir() {
    QString sSpotDir = spotsDirEdit.text();
    QDir spotDir = QDir(sSpotDir);
    if(spotDir.exists()) {
        sSpotDir = QFileDialog::getExistingDirectory(this,
                                                     "Spot Directory",
                                                     sSpotDir);
    }
    else {
        sSpotDir = QFileDialog::getExistingDirectory(this,
                                                     "Spot Directory",
                                                     QStandardPaths::displayName(QStandardPaths::GenericDataLocation));
    }
    if(sSpotDir == QString()) return; // "Cancel" has been pressed...
    if(!sSpotDir.endsWith(QString("/"))) sSpotDir+= QString("/");
    spotsDirEdit.setText(sSpotDir);
    spotDir.setPath(sSpotDir);
    setSpotDir(sSpotDir);
}


void
GeneralSetupDialog::onOk() {
    pTempArguments->iTimeoutDuration = timeoutDurationEdit.text().toInt();
    pTempArguments->maxTimeout       = numTimeoutEdit.text().toInt();
    pTempArguments->maxSet           = maxSetEdit.text().toInt();
    pTempArguments->sSlideDir        = slidesDirEdit.text();
    pTempArguments->sSpotDir         = spotsDirEdit.text();
    accept();
}


void
GeneralSetupDialog::onCancel() {
    reject();
}
