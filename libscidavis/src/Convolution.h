/***************************************************************************
    File                 : Convolution.h
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
#ifndef CONVOLUTION_H
#define CONVOLUTION_H

#include "Filter.h"

class Convolution : public Filter
{
    Q_OBJECT

public:
    Convolution(ApplicationWindow *parent, Table *t, const QString &signalColName,
                const QString &responseColName);

    void setDataFromTable(Table *t, const QString &signalColName, const QString &responseColName);

protected:
    //! Handles the graphical output
    void addResultCurve();
    //! Performes the convolution of the two data sets and stores the result in the signal data set
    void convlv(double *sig, int n, double *dres, int m, int sign);

private:
    virtual void output();
};

class Deconvolution : public Convolution
{
    Q_OBJECT

public:
    Deconvolution(ApplicationWindow *parent, Table *t, const QString &realColName,
                  const QString &imagColName = QString());

private:
    void output();
};

#endif
