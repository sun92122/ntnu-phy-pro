/***************************************************************************
    File                 : Convolution.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2007 by Ion Vasilief
    Email (use @ for *)  : ion_vasilief*yahoo.fr
    Description          : Numerical convolution/deconvolution of data sets

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
#include "Convolution.h"
#include "MultiLayer.h"
#include "Plot.h"
#include "PlotCurve.h"
#include "ColorButton.h"
#include "core/column/Column.h"

#include <QMessageBox>
#include <QLocale>
#include <gsl/gsl_fft_halfcomplex.h>

Convolution::Convolution(ApplicationWindow *parent, Table *t, const QString &signalColName,
                         const QString &responseColName)
    : Filter(parent, t)
{
    setObjectName(tr("Convolution"));
    setDataFromTable(t, signalColName, responseColName);
}

void Convolution::setDataFromTable(Table *t, const QString &signalColName,
                                   const QString &responseColName)
{
    if (t && d_table != t)
        d_table = t;

    int signal_col = d_table->colIndex(signalColName);
    int response_col = d_table->colIndex(responseColName);

    if (signal_col < 0) {
        QMessageBox::warning((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
                             tr("The signal data set %1 does not exist!").arg(signalColName));
        d_init_err = true;
        return;
    } else if (response_col < 0) {
        QMessageBox::warning((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
                             tr("The response data set %1 does not exist!").arg(responseColName));
        d_init_err = true;
        return;
    }

    size_t d_n_response = 0;
    int rows = d_table->numRows();
    for (int i = 0; i < rows; i++) {
        if (!d_table->text(i, response_col).isEmpty())
            d_n_response++;
    }
    if (d_n_response >= rows / 2) {
        QMessageBox::warning((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
                             tr("The response dataset '%1' must be less then half the size of the "
                                "signal dataset '%2'!")
                                     .arg(responseColName)
                                     .arg(signalColName));
        d_init_err = true;
        return;
    } else if (d_n_response % 2 == 0) {
        QMessageBox::warning((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
                             tr("The response dataset '%1' must contain an odd number of points!")
                                     .arg(responseColName));
        d_init_err = true;
        return;
    }

    size_t d_n_signal = 16; // tmp number of points
    while (unsigned(d_n_signal) < rows + d_n_response / 2)
        d_n_signal *= 2;

    try {
        d_x.resize(d_n_signal);
        d_y.resize(d_n_response);
    } catch (const std::bad_alloc &e) {
        QMessageBox::critical((ApplicationWindow *)parent(), tr("SciDAVis") + " - " + tr("Error"),
                              tr("Could not allocate memory, operation aborted!\n")
                                      + tr("Allocator returned: ") + e.what());
        d_init_err = true;
        return;
    }

    for (unsigned i = 0; i < rows; i++)
        d_x[i] = d_table->cell(i, signal_col);
    std::fill(d_x.begin()+rows,d_x.end(),0.0);
    for (int i = 0u; i < d_n_response; i++)
        d_y[i] = d_table->cell(i, response_col);
}

void Convolution::output()
{
    convlv(d_x.data(), d_x.size(), d_y.data(), d_y.size(), 1);
    addResultCurve();
}

void Convolution::addResultCurve()
{
    ApplicationWindow *app = (ApplicationWindow *)parent();
    if (!app)
        return;

    int cols = d_table->numCols();
    int cols2 = cols + 1;

    d_table->addCol();
    d_table->addCol();
    std::vector<double> x_temp(d_x.size());
    for (unsigned i = 0; i < d_x.size(); i++) {
        double x = i + 1;
        x_temp[i] = x;

        d_table->column(cols)->setValueAt(i, x);
        d_table->column(cols2)->setValueAt(i, d_x[i]);
    }

    QStringList l = d_table->colNames().filter(tr("Index"));
    QString id = QString::number((int)l.size() + 1);
    QString label = objectName() + id;

    d_table->setColName(cols, tr("Index") + id);
    d_table->setColName(cols2, label);
    d_table->setColPlotDesignation(cols, SciDAVis::X);

    MultiLayer *ml = app->newGraph(objectName() + tr("Plot"));
    if (!ml)
        return;

    DataCurve *c = new DataCurve(d_table, d_table->colName(cols), d_table->colName(cols2));
    c->setData(x_temp.data(), d_x.data(), d_x.size());
    c->setPen(QPen(d_curveColor, 1));
    ml->activeGraph()->insertPlotItem(c, Graph::Line);
    ml->activeGraph()->updatePlot();
}

void Convolution::convlv(double *sig, int n, double *dres, int m, int sign)
{
    double *res = new double[n];
    memset(res, 0, n * sizeof(double));
    int i, m2 = m / 2;
    for (i = 0; i < m2; i++) { // store the response in wrap around order, see Numerical Recipes doc
        res[i] = dres[m2 + i];
        res[n - m2 + i] = dres[i];
    }

    if (m2 % 2 == 1)
        res[m2] = dres[m - 1];

    // calculate ffts
    gsl_fft_real_radix2_transform(res, 1, n);
    gsl_fft_real_radix2_transform(sig, 1, n);

    double re, im, size;
    for (i = 0; i < n / 2; i++) { // multiply/divide both ffts
        if (i == 0 || i == n / 2 - 1) {
            if (sign == 1)
                sig[i] = res[i] * sig[i];
            else
                sig[i] = sig[i] / res[i];
        } else {
            int ni = n - i;
            if (sign == 1) {
                re = res[i] * sig[i] - res[ni] * sig[ni];
                im = res[i] * sig[ni] + res[ni] * sig[i];
            } else {
                size = res[i] * res[i] + res[ni] * res[ni];
                re = res[i] * sig[i] + res[ni] * sig[ni];
                im = res[i] * sig[ni] - res[ni] * sig[i];
                re /= size;
                im /= size;
            }

            sig[i] = re;
            sig[ni] = im;
        }
    }
    delete[] res;
    gsl_fft_halfcomplex_radix2_inverse(sig, 1, n); // inverse fft
}
/**************************************************************************
 *             Class Deconvolution                                         *
 ***************************************************************************/

Deconvolution::Deconvolution(ApplicationWindow *parent, Table *t, const QString &signalColName,
                             const QString &responseColName)
    : Convolution(parent, t, signalColName, responseColName)
{
    setObjectName(tr("Deconvolution"));
    setDataFromTable(t, signalColName, responseColName);
}

void Deconvolution::output()
{
    convlv(d_x.data(), d_x.size(), d_y.data(), d_y.size(), -1);
    addResultCurve();
}
