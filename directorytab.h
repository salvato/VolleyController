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
#ifndef DIRECTORYTAB_H
#define DIRECTORYTAB_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>

class DirectoryTab : public QWidget
{
    Q_OBJECT
public:
    explicit DirectoryTab(QWidget *parent = nullptr);
    void setSlideDir(const QString& sDir);
    void setSpotDir(const QString& sDir);
    QString getSlideDir();
    QString getSpotDir();

signals:

public slots:
    void onSelectSlideDir();
    void onSelectSpotDir();

private:
    QLineEdit slidesDirEdit;
    QLineEdit spotsDirEdit;
    QPushButton buttonSelectSlidesDir;
    QPushButton buttonSelectSpotsDir;
};

#endif // DIRECTORYTAB_H
