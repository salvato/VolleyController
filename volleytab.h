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
#ifndef VOLLEYTAB_H
#define VOLLEYTAB_H

#include <QWidget>
#include <QLineEdit>

QT_FORWARD_DECLARE_CLASS(QSettings)


class VolleyTab : public QWidget
{
    Q_OBJECT
public:
    explicit VolleyTab(QWidget *parent = nullptr);
    void GetSettings();
    void StoreSettings();
    int getNumTimeout();
    int getNumSet();
    int getTimeoutDuration();

signals:

public slots:

private:
    QSettings *pSettings;
    QLineEdit  numTimeoutEdit;
    QLineEdit  maxSetEdit;
    QLineEdit  timeoutDurationEdit;

    int        maxTimeout;
    int        maxSet;
    int        iTimeoutDuration;

};

#endif // VOLLEYTAB_H
