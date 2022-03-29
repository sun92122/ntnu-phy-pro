/***************************************************************************
    File                 : Note.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief,
                           Tilman Benkert,
                                          Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Notes window class

 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *  This program is free software; you can redistribute it and/or modify   *
 *  it under the terms of the GNU General Public License as published by   *
 *  the Free Software Foundation; either version 2 of the License, or      *
 *  (at your option) any later version.                                    *
 *                                                                         *
 *  This program is distributed in the hope that it will be useful,        *
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of         *
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the          *
 *  GNU General Public License for more details.                           *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the Free Software           *
 *   Foundation, Inc., 51 Franklin Street, Fifth Floor,                    *
 *   Boston, MA  02110-1301  USA                                           *
 *                                                                         *
 ***************************************************************************/
#include "Note.h"
#include "ScriptEdit.h"

#include <QDateTime>
#include <QLayout>
#include <QApplication>
#include <QPrinter>
#include <QPainter>
#include <QPaintDevice>
#include <QVBoxLayout>
#include <QPrintDialog>

Note::Note(ScriptingEnv *env, const QString &label, QWidget *parent, const char *name,
           Qt::WindowFlags f)
    : MyWidget(label, parent, name, f)
{
    init(env);
}

void Note::init(ScriptingEnv *env)
{
    autoExec = false;
    QDateTime dt = QDateTime::currentDateTime();
    setBirthDate(QLocale().toString(dt));

    te = new ScriptEdit(env, this, name());
    te->setContext(this);
    this->setWidget(te);

    setGeometry(0, 0, 500, 200);
    connect(te, SIGNAL(textChanged()), this, SLOT(modifiedNote()));
}

void Note::modifiedNote()
{
    emit modifiedWindow(this);
}

QString Note::saveToString(const QString &info)
{
    QString s = "<note>\n";
    s += QString(name()) + "\t" + birthDate() + "\n";
    s += info;
    s += "WindowLabel\t" + windowLabel() + "\t" + QString::number(captionPolicy()) + "\n";
    s += "AutoExec\t" + QString(autoExec ? "1" : "0") + "\n";
    s += "<content>\n" + te->toPlainText().trimmed() + "\n</content>";
    s += "\n</note>\n";
    return s;
}

void Note::restore(const QStringList &data)
{
    QStringList::ConstIterator line = data.begin();
    QStringList fields;

    fields = (*line).split("\t");
    if (fields[0] == "AutoExec") {
        setAutoexec(fields[1] == "1");
        line++;
    }

    if (*line == "<content>")
        line++;
    while (line != data.end() && *line != "</content>")
        te->insertPlainText((*line++) + "\n");
}

void Note::setAutoexec(bool exec)
{
    autoExec = exec;
    QPalette palette;
    if (autoExec)
        palette.setColor(te->backgroundRole(), QColor(255, 239, 185));
    te->setPalette(palette);
}
