/*
Copyright (C) 2020 popkc(popkcer at gmail dot com)
This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/
#ifndef DIALOG_H
#define DIALOG_H

#include <QDialog>
#include <QDir>
#include <QSettings>
#include <QStringList>

QT_BEGIN_NAMESPACE
namespace Ui {
class Dialog;
}
QT_END_NAMESPACE

class Dialog : public QDialog
{
    Q_OBJECT

public:
    QSettings settings;

    Dialog(QWidget* parent = nullptr);
    ~Dialog();

private slots:
    void on_pushButtonDir_clicked();

    void on_pushButtonRenew_clicked();

    void on_pushButtonDefault_clicked();

private:
    Ui::Dialog* ui;
    size_t totalCount, renewCount;
    QStringList suffixes;

    void renewDir(const QDir& dir);
    bool renewFile(const QString& filePath);
    bool writeFile(QFile& file, QByteArray& content, bool utf8Bom);
};
#endif // DIALOG_H
