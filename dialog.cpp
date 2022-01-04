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
#include "dialog.h"
#include "ui_dialog.h"
#include <QDate>
#include <QFile>
#include <QFileDialog>
#include <QMessageBox>
#include <QTextCodec>
#include <QTextStream>

#define DEFAULT_LICENSE "This program is free software: you can redistribute it and/or modify"   \
                        "\nit under the terms of the GNU General Public License as published by" \
                        "\nthe Free Software Foundation, either version 3 of the License, or"    \
                        "\n(at your option) any later version.\n"                                \
                        "\nThis program is distributed in the hope that it will be useful,"      \
                        "\nbut WITHOUT ANY WARRANTY; without even the implied warranty of"       \
                        "\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the"        \
                        "\nGNU General Public License for more details.\n"                       \
                        "\nYou should have received a copy of the GNU General Public License"    \
                        "\nalong with this program.  If not, see <https://www.gnu.org/licenses/>."
#define DEFAULT_SUFFIX "*.c;*.cpp;*.h;*.hpp"

Dialog::Dialog(QWidget* parent)
    : QDialog(parent)
    , settings("popkc", "addlicense")
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);

    ui->lineEditDir->setText(settings.value("dir").toString());
    int year = QDate::currentDate().year();
    QString s = QString::number(year);
    ui->lineEditYear->setText(settings.value("year", s).toString());
    ui->lineEditSuffix->setText(settings.value("suffix", DEFAULT_SUFFIX).toString());
    ui->lineEditCopyright->setText(settings.value("copyright", "popkc(popkc at 163.com)").toString());
    ui->plainTextEditLicense->setPlainText(settings.value("license", DEFAULT_LICENSE).toString());
}

Dialog::~Dialog()
{
    delete ui;
}

void Dialog::on_pushButtonDir_clicked()
{
    QString dir = QFileDialog::getExistingDirectory(this, "选择源码文件夹");
    if (!dir.isEmpty()) {
        ui->lineEditDir->setText(dir);
    }
}

void Dialog::on_pushButtonRenew_clicked()
{
    settings.setValue("dir", ui->lineEditDir->text());
    settings.setValue("year", ui->lineEditYear->text());
    settings.setValue("suffix", ui->lineEditSuffix->text());
    settings.setValue("copyright", ui->lineEditCopyright->text());
    settings.setValue("license", ui->plainTextEditLicense->toPlainText());

    QDir dir(ui->lineEditDir->text());
    if (!dir.exists()) {
        QMessageBox::warning(this, "错误", "源码文件夹不存在");
        return;
    }

    suffixes = ui->lineEditSuffix->text().split(';', QString::SkipEmptyParts);
    if (suffixes.empty()) {
        QMessageBox::warning(this, "错误", "后缀名不能为空");
        return;
    }

    totalCount = 0;
    renewCount = 0;
    renewDir(dir);
    QString s;
    QTextStream ts(&s);
    ts << QString("共找到") << totalCount << QString("个符合要求的文件，更新了其中的") << renewCount << QString("个文件。");
    QMessageBox::about(this, "更新完成", s);
}

void Dialog::renewDir(const QDir& dir)
{
    auto fel = dir.entryInfoList(suffixes, QDir::NoDotAndDotDot | QDir::AllDirs | QDir::Files | QDir::Readable | QDir::Writable);
    for (auto& fe : fel) {
        if (fe.isDir()) {
            renewDir(fe.filePath());
        }
        else if (fe.isFile()) {
            totalCount++;
            if (renewFile(fe.filePath()))
                renewCount++;
        }
    }
}

bool Dialog::renewFile(const QString& filePath)
{
    QFile file(filePath);
    if (!file.open(QIODevice::ReadOnly))
        return false;
    bool utf8Bom = false;
    size_t startPos = 0;
    QByteArray content;
    if (file.size() >= 3) {
        auto ba = file.read(3);
        if (ba.startsWith("\xef\xbb\xbf")) {
            utf8Bom = true;
            startPos = 3;
        }
        else
            file.seek(0);
        ba = file.readLine();
        if (ba != "/*\n" && ba != "/*\r\n") {
            content = ba;
        }
        else {
            ba = file.readLine();
            if (ba.startsWith("Copyright (C)")
                && (ui->checkBoxForceRenew->isChecked() || ba.contains(ui->lineEditCopyright->text().toUtf8()))) {
                int k;
                while (!ba.isEmpty() && (k = ba.indexOf("*/")) == -1) {
                    ba = file.readLine();
                }
                if (ba.isEmpty())
                    return false;
                content = ba.mid(k + 2);
                if (content.size() == 1)
                    content.clear();
            }
            else
                return false;
        }
    }
    content.append(file.readAll());
    file.close();
    return writeFile(file, content, utf8Bom);
}

bool Dialog::writeFile(QFile& file, QByteArray& content, bool utf8Bom)
{
    if (!file.open(QIODevice::WriteOnly))
        return false;
    if (utf8Bom)
        file.write("\xef\xbb\xbf");
    file.write("/*\nCopyright (C) ");
    file.write(ui->lineEditYear->text().toUtf8());
    file.write(" ");
    file.write(ui->lineEditCopyright->text().toUtf8());
    file.write("\n");
    file.write(ui->plainTextEditLicense->toPlainText().toUtf8());
    file.write("\n*/\n");
    file.write(content);
    file.close();
    return true;
}

void Dialog::on_pushButtonDefault_clicked()
{
    ui->plainTextEditLicense->setPlainText(DEFAULT_LICENSE);
    ui->lineEditSuffix->setText(DEFAULT_SUFFIX);
    int year = QDate::currentDate().year();
    QString s = QString::number(year);
    ui->lineEditYear->setText(s);
}
