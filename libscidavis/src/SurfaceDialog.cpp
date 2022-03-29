/***************************************************************************
    File                 : SurfaceDialog.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Define surface plot dialog

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
#include "SurfaceDialog.h"
#include "MyParser.h"
#include "ApplicationWindow.h"

#include <QMessageBox>
#include <QLayout>
#include <QGroupBox>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QComboBox>

SurfaceDialog::SurfaceDialog(QWidget *parent, Qt::WindowFlags fl) : QDialog(parent, fl)
{
    setWindowTitle(tr("Define surface plot"));
    setSizeGripEnabled(true);

    boxFunction = new QComboBox();
    boxFunction->setEditable(true);

    QBoxLayout *bl1 = new QBoxLayout(QBoxLayout::LeftToRight);
    bl1->addWidget(new QLabel(tr("f(x,y)=")), 1);
    bl1->addWidget(boxFunction, 10);

    QGroupBox *gb1 = new QGroupBox(tr("X - axis"));

    boxXFrom = new QLineEdit();
    boxXFrom->setText(tr("-1"));

    boxXTo = new QLineEdit();
    boxXTo->setText(tr("1"));

    QGridLayout *gl1 = new QGridLayout();
    gl1->addWidget(new QLabel(tr("From")), 0, 0);
    gl1->addWidget(boxXFrom, 0, 1);
    gl1->addWidget(new QLabel(tr("To")), 1, 0);
    gl1->addWidget(boxXTo, 1, 1);
    gl1->setRowStretch(2, 1);
    gb1->setLayout(gl1);

    QGroupBox *gb2 = new QGroupBox(tr("Y - axis"));
    boxYFrom = new QLineEdit();
    boxYFrom->setText(tr("-1"));

    boxYTo = new QLineEdit();
    boxYTo->setText(tr("1"));

    QGridLayout *gl2 = new QGridLayout();
    gl2->addWidget(new QLabel(tr("From")), 0, 0);
    gl2->addWidget(boxYFrom, 0, 1);
    gl2->addWidget(new QLabel(tr("To")), 1, 0);
    gl2->addWidget(boxYTo, 1, 1);
    gl2->setRowStretch(2, 1);
    gb2->setLayout(gl2);

    QGroupBox *gb3 = new QGroupBox(tr("Z - axis"));
    boxZFrom = new QLineEdit();
    boxZFrom->setText(tr("-1"));

    boxZTo = new QLineEdit();
    boxZTo->setText(tr("1"));

    QGridLayout *gl3 = new QGridLayout();
    gl3->addWidget(new QLabel(tr("From")), 0, 0);
    gl3->addWidget(boxZFrom, 0, 1);
    gl3->addWidget(new QLabel(tr("To")), 1, 0);
    gl3->addWidget(boxZTo, 1, 1);
    gl3->setRowStretch(2, 1);
    gb3->setLayout(gl3);

    QBoxLayout *bl3 = new QBoxLayout(QBoxLayout::LeftToRight);
    bl3->addWidget(gb1);
    bl3->addWidget(gb2);
    bl3->addWidget(gb3);

    buttonClear = new QPushButton(tr("Clear &list"));
    buttonOk = new QPushButton(tr("&OK"));
    buttonOk->setDefault(true);
    buttonCancel = new QPushButton(tr("&Close"));

    QBoxLayout *bl2 = new QBoxLayout(QBoxLayout::LeftToRight);
    bl2->addStretch();
    bl2->addWidget(buttonOk);
    bl2->addWidget(buttonClear);
    bl2->addWidget(buttonCancel);
    bl2->addStretch();

    QVBoxLayout *vl = new QVBoxLayout(this);
    vl->addLayout(bl1);
    vl->addLayout(bl3);
    vl->addLayout(bl2);

    resize(vl->minimumSize());
    setFocusProxy(boxFunction);

    connect(buttonClear, SIGNAL(clicked()), this, SLOT(clearList()));
    connect(buttonOk, SIGNAL(clicked()), this, SLOT(accept()));
    connect(buttonCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void SurfaceDialog::clearList()
{
    boxFunction->clear();
    emit clearFunctionsList();
}

void SurfaceDialog::setFunction(const QString &s)
{
    boxFunction->setItemText(boxFunction->currentIndex(), s);
}

void SurfaceDialog::setLimits(double xs, double xe, double ys, double ye, double zs, double ze)
{
    boxXFrom->setText(QString::number(xs));
    boxXTo->setText(QString::number(xe));
    boxYFrom->setText(QString::number(ys));
    boxYTo->setText(QString::number(ye));
    boxZFrom->setText(QString::number(zs));
    boxZTo->setText(QString::number(ze));
}

void SurfaceDialog::accept()
{
    QString Xfrom = boxXFrom->text().toLower();
    QString Xto = boxXTo->text().toLower();
    QString Yfrom = boxYFrom->text().toLower();
    QString Yto = boxYTo->text().toLower();
    QString Zfrom = boxZFrom->text().toLower();
    QString Zto = boxZTo->text().toLower();

    double fromX, toX, fromY, toY, fromZ, toZ;
    try {
        MyParser parser;
        parser.SetExpr(Xfrom);
        fromX = parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(0, tr("X Start limit error"), QStringFromString(e.GetMsg()));
        boxXFrom->setFocus();
        return;
    }
    try {
        MyParser parser;
        parser.SetExpr(Xto);
        toX = parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(0, tr("X End limit error"), QStringFromString(e.GetMsg()));
        boxXTo->setFocus();
        return;
    }

    try {
        MyParser parser;
        parser.SetExpr(Yfrom);
        fromY = parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(0, tr("Y Start limit error"), QStringFromString(e.GetMsg()));
        boxYFrom->setFocus();
        return;
    }
    try {
        MyParser parser;
        parser.SetExpr(Yto);
        toY = parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(0, tr("Y End limit error"), QStringFromString(e.GetMsg()));
        boxYTo->setFocus();
        return;
    }
    try {
        MyParser parser;
        parser.SetExpr(Zfrom);
        fromZ = parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(0, tr("Z Start limit error"), QStringFromString(e.GetMsg()));
        boxZFrom->setFocus();
        return;
    }
    try {
        MyParser parser;
        parser.SetExpr(Zto);
        toZ = parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(0, tr("Z End limit error"), QStringFromString(e.GetMsg()));
        boxZTo->setFocus();
        return;
    }

    if (fromX >= toX || fromY >= toY || fromZ >= toZ) {
        QMessageBox::critical(0, tr("Input error"),
                              tr("Please enter limits that satisfy: from < end!"));
        boxXTo->setFocus();
        return;
    }

    double x, y;
    QString formula = boxFunction->currentText();
    bool error = false;
    try {
        MyParser parser;
        parser.DefineVar(_T("x"), &x);
        parser.DefineVar(_T("y"), &y);
        parser.SetExpr(formula);

        x = fromX;
        y = fromY;
        parser.Eval();
        x = toX;
        y = toY;
        parser.Eval();
    } catch (mu::ParserError &e) {
        QMessageBox::critical(0, tr("Input function error"), QStringFromString(e.GetMsg()));
        boxFunction->setFocus();
        error = true;
    }

    if (!error) {
        emit options(boxFunction->currentText(), fromX, toX, fromY, toY, fromZ, toZ);
        emit custom3DToolBar();

        ApplicationWindow *app = (ApplicationWindow *)this->parent();
        app->updateSurfaceFuncList(boxFunction->currentText());
        close();
    }
}

void SurfaceDialog::insertFunctionsList(const QStringList &list)
{
    boxFunction->insertItems(1, list);
}

SurfaceDialog::~SurfaceDialog() { }
