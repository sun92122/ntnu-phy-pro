/***************************************************************************
    File                 : DataPickerTool.cpp
    Project              : SciDAVis
    --------------------------------------------------------------------
    Copyright            : (C) 2006,2007 by Ion Vasilief,
                           Tilman Benkert, Knut Franke
    Email (use @ for *)  : ion_vasilief*yahoo.fr, thzs*gmx.net,
                           knut.franke*gmx.de
    Description          : Plot tool for selecting points on curves.

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
#include "DataPickerTool.h"
#include "Graph.h"
#include "Plot.h"
#include "FunctionCurve.h"
#include "PlotCurve.h"
#include "QwtErrorPlotCurve.h"
#include "ApplicationWindow.h"
#include "core/column/Column.h"

#include <qwt_symbol.h>
#include <qwt_plot_picker.h>
#include <qwt_plot_curve.h>
#include <qwt_scale_draw.h>
#include <QMessageBox>
#include <QLocale>
#include <QKeyEvent>
#include <QMouseEvent>

DataPickerTool::DataPickerTool(Graph *graph, ApplicationWindow *app, Mode mode,
                               const QObject *status_target, const char *status_slot)
    : QwtPlotPicker(graph->plotWidget()->canvas()),
      PlotToolInterface(graph),
      d_app(app),
      d_mode(mode),
      d_move_mode(Free)
{
    d_selected_curve = NULL;

    d_selection_marker.setSymbol(QwtSymbol(QwtSymbol::Ellipse, QBrush(QColor(255, 255, 0, 128)),
                                           QPen(Qt::black, 2), QSize(20, 20)));
    d_selection_marker.setLineStyle(QwtPlotMarker::Cross);
    d_selection_marker.setLinePen(QPen(Qt::red, 1));

    setTrackerMode(QwtPicker::AlwaysOn);
    if (d_mode == Move) {
        setSelectionFlags(QwtPicker::PointSelection | QwtPicker::DragSelection);
        d_graph->plotWidget()->canvas()->setCursor(Qt::PointingHandCursor);
    } else {
        setSelectionFlags(QwtPicker::PointSelection | QwtPicker::ClickSelection);
        d_graph->plotWidget()->canvas()->setCursor(QCursor(QPixmap(":/vizor.xpm"), -1, -1));
    }

    if (status_target)
        connect(this, SIGNAL(statusText(const QString &)), status_target, status_slot);
    switch (d_mode) {
    case Display:
        emit statusText(tr("Click on plot or move cursor to display coordinates!"));
        break;
    case Move:
        emit statusText(tr("Please, click on plot and move cursor!"));
        break;
    case Remove:
        emit statusText(tr("Select point and double click to remove it!"));
        break;
    }
}

DataPickerTool::~DataPickerTool()
{
    d_selection_marker.detach();
    d_graph->plotWidget()->canvas()->unsetCursor();
}

void DataPickerTool::append(const QPoint &pos)
{
    int dist, point_index;
    const int curve = d_graph->plotWidget()->closestCurve(pos.x(), pos.y(), dist, point_index);
    if (curve <= 0 || dist >= 5) { // 5 pixels tolerance
        setSelection(NULL, 0);
        return;
    }
    setSelection((QwtPlotCurve *)d_graph->plotWidget()->curve(curve), point_index);
    if (!d_selected_curve)
        return;

    QwtPlotPicker::append(transform(QwtDoublePoint(d_selected_curve->x(d_selected_point),
                                                   d_selected_curve->y(d_selected_point))));
}

void DataPickerTool::setSelection(QwtPlotCurve *curve, int point_index)
{
    if (curve == d_selected_curve && point_index == d_selected_point)
        return;

    d_selected_curve = curve;
    d_selected_point = point_index;

    if (!d_selected_curve) {
        d_selection_marker.detach();
        d_graph->plotWidget()->replot();
        return;
    }

    setAxis(d_selected_curve->xAxis(), d_selected_curve->yAxis());

    d_move_target_pos = QPoint(plot()->transform(xAxis(), d_selected_curve->x(d_selected_point)),
                               plot()->transform(yAxis(), d_selected_curve->y(d_selected_point)));

    if (((PlotCurve *)d_selected_curve)->type() == Graph::Function) {
        emit statusText(QString("%1[%2]: x=%3; y=%4")
                                .arg(d_selected_curve->title().text())
                                .arg(d_selected_point + 1)
                                .arg(QLocale().toString(d_selected_curve->x(d_selected_point), 'G',
                                                        d_app->d_decimal_digits))
                                .arg(QLocale().toString(d_selected_curve->y(d_selected_point), 'G',
                                                        d_app->d_decimal_digits)));
    } else {
        int row = ((DataCurve *)d_selected_curve)->tableRow(d_selected_point);

        Table *t = ((DataCurve *)d_selected_curve)->table();
        int xCol = t->colIndex(((DataCurve *)d_selected_curve)->xColumnName());
        int yCol = t->colIndex(d_selected_curve->title().text());

        emit statusText(QString("%1[%2]: x=%3; y=%4")
                                .arg(d_selected_curve->title().text())
                                .arg(row + 1)
                                .arg(t->text(row, xCol))
                                .arg(t->text(row, yCol)));
    }

    QwtDoublePoint selected_point_value(d_selected_curve->x(d_selected_point),
                                        d_selected_curve->y(d_selected_point));
    d_selection_marker.setValue(selected_point_value);
    if (d_selection_marker.plot() == NULL)
        d_selection_marker.attach(d_graph->plotWidget());
    d_graph->plotWidget()->replot();
}

bool DataPickerTool::eventFilter(QObject *obj, QEvent *event)
{
    switch (event->type()) {
    case QEvent::MouseButtonDblClick:
        switch (d_mode) {
        case Remove:
            removePoint();
            return true;
        default:
            if (d_selected_curve)
                emit selected(d_selected_curve, d_selected_point);
            return true;
        }
    case QEvent::MouseMove:
        if (((QMouseEvent *)event)->modifiers() == Qt::ControlModifier)
            d_move_mode = Vertical;
        else if (((QMouseEvent *)event)->modifiers() == Qt::AltModifier)
            d_move_mode = Horizontal;
        else
            d_move_mode = Free;
        break;

    case QEvent::KeyPress:
        if (keyEventFilter((QKeyEvent *)event))
            return true;
        break;
    default:
        break;
    }
    return QwtPlotPicker::eventFilter(obj, event);
}

bool DataPickerTool::keyEventFilter(QKeyEvent *ke)
{
    const int delta = 5;
    switch (ke->key()) {
    case Qt::Key_Enter:
    case Qt::Key_Return:
        if (d_selected_curve)
            emit selected(d_selected_curve, d_selected_point);
        return true;

    case Qt::Key_Up:
        if (d_graph && d_selected_curve) {
            int n_curves = d_graph->curves();
            int start = d_graph->curveIndex(d_selected_curve) + 1;
            QwtPlotCurve *c;
            for (int i = start; i < start + n_curves; ++i)
                if ((c = d_graph->curve(i % n_curves))->dataSize() > 0) {
                    setSelection(c, qMin(c->dataSize() - 1, d_selected_point));
                    break;
                }
            d_graph->plotWidget()->replot();
        }
        return true;

    case Qt::Key_Down:
        if (d_graph && d_selected_curve) {
            int n_curves = d_graph->curves();
            int start = d_graph->curveIndex(d_selected_curve) + n_curves - 1;
            QwtPlotCurve *c;
            for (int i = start; i > start - n_curves; --i)
                if ((c = d_graph->curve(i % n_curves))->dataSize() > 0) {
                    setSelection(c, qMin(c->dataSize() - 1, d_selected_point));
                    break;
                }
            d_graph->plotWidget()->replot();
        }
        return true;

    case Qt::Key_Right:
    case Qt::Key_Plus:
        if (d_graph) {
            if (d_selected_curve) {
                int n_points = d_selected_curve->dataSize();
                setSelection(d_selected_curve, (d_selected_point + 1) % n_points);
                d_graph->plotWidget()->replot();
            } else
                setSelection(d_graph->curve(0), 0);
        }
        return true;

    case Qt::Key_Left:
    case Qt::Key_Minus:
        if (d_graph) {
            if (d_selected_curve) {
                int n_points = d_selected_curve->dataSize();
                setSelection(d_selected_curve, (d_selected_point - 1 + n_points) % n_points);
                d_graph->plotWidget()->replot();
            } else
                setSelection(d_graph->curve(d_graph->curves() - 1), 0);
        }
        return true;

    // The following keys represent a direction, they are
    // organized on the keyboard.
    case Qt::Key_1:
        if (d_mode == Move) {
            moveBy(-delta, delta);
            return true;
        }
        break;
    case Qt::Key_2:
        if (d_mode == Move) {
            moveBy(0, delta);
            return true;
        }
        break;
    case Qt::Key_3:
        if (d_mode == Move) {
            moveBy(delta, delta);
            return true;
        }
        break;
    case Qt::Key_4:
        if (d_mode == Move) {
            moveBy(-delta, 0);
            return true;
        }
        break;
    case Qt::Key_6:
        if (d_mode == Move) {
            moveBy(delta, 0);
            return true;
        }
        break;
    case Qt::Key_7:
        if (d_mode == Move) {
            moveBy(-delta, -delta);
            return true;
        }
        break;
    case Qt::Key_8:
        if (d_mode == Move) {
            moveBy(0, -delta);
            return true;
        }
        break;
    case Qt::Key_9:
        if (d_mode == Move) {
            moveBy(delta, -delta);
            return true;
        }
        break;
    default:
        break;
    }
    return false;
}

void DataPickerTool::removePoint()
{
    if (!d_selected_curve)
        return;
    if (((PlotCurve *)d_selected_curve)->type() == Graph::Function) {
        QMessageBox::critical(const_cast<Graph *>(d_graph), tr("Remove point error"),
                              tr("Sorry, but removing points of a function is not possible."));
        return;
    }

    Table *t = ((DataCurve *)d_selected_curve)->table();
    if (!t)
        return;

    int col = t->colIndex(d_selected_curve->title().text());
    if (t->columnType(col) == SciDAVis::ColumnMode::Numeric) {
        t->column(col)->setValueAt(((DataCurve *)d_selected_curve)->tableRow(d_selected_point),
                                   0.0);
        t->column(col)->setInvalid(((DataCurve *)d_selected_curve)->tableRow(d_selected_point),
                                   true);
    } else {
        QMessageBox::warning(const_cast<Graph *>(d_graph), tr("Warning"),
                             tr("This operation cannot be performed on curves plotted from columns "
                                "having a non-numerical format."));
    }

    d_selection_marker.detach();
    d_graph->plotWidget()->replot();
    d_graph->setFocus();
    d_selected_curve = NULL;
}

void DataPickerTool::move(const QPoint &point)
{
    if (d_mode == Move && d_selected_curve) {
        switch (d_move_mode) {
        case Free:
            d_move_target_pos = point;
            break;
        case Vertical:
            d_move_target_pos.setY(point.y());
            break;
        case Horizontal:
            d_move_target_pos.setX(point.x());
            break;
        }
        double new_x_val = d_graph->plotWidget()->invTransform(d_selected_curve->xAxis(),
                                                               d_move_target_pos.x());
        double new_y_val = d_graph->plotWidget()->invTransform(d_selected_curve->yAxis(),
                                                               d_move_target_pos.y());
        d_selection_marker.setValue(new_x_val, new_y_val);
        if (d_selection_marker.plot() == NULL)
            d_selection_marker.attach(d_graph->plotWidget());
        d_graph->replot();

        int row = ((DataCurve *)d_selected_curve)->tableRow(d_selected_point);
        emit statusText(QString("%1[%2]: x=%3; y=%4")
                                .arg(d_selected_curve->title().text())
                                .arg(row + 1)
                                .arg(QLocale().toString(new_x_val, 'G', d_app->d_decimal_digits))
                                .arg(QLocale().toString(new_y_val, 'G', d_app->d_decimal_digits)));
    }

    QwtPlotPicker::move(d_move_target_pos);
}

bool DataPickerTool::end(bool ok)
{
    if (d_mode == Move && d_selected_curve) {
        if (((PlotCurve *)d_selected_curve)->type() == Graph::Function) {
            QMessageBox::critical(d_graph, tr("Move point error"),
                                  tr("Sorry, but moving points of a function is not possible."));
            return QwtPlotPicker::end(ok);
        }
        Table *t = ((DataCurve *)d_selected_curve)->table();
        if (!t)
            return QwtPlotPicker::end(ok);
        double new_x_val = d_graph->plotWidget()->invTransform(d_selected_curve->xAxis(),
                                                               d_move_target_pos.x());
        double new_y_val = d_graph->plotWidget()->invTransform(d_selected_curve->yAxis(),
                                                               d_move_target_pos.y());
        int row = ((DataCurve *)d_selected_curve)->tableRow(d_selected_point);
        int xcol = t->colIndex(((DataCurve *)d_selected_curve)->xColumnName());
        int ycol = t->colIndex(d_selected_curve->title().text());
        if (t->columnType(xcol) == SciDAVis::ColumnMode::Numeric
            && t->columnType(ycol) == SciDAVis::ColumnMode::Numeric) {
            t->column(xcol)->setValueAt(row, new_x_val);
            t->column(ycol)->setValueAt(row, new_y_val);
            d_app->updateCurves(t, d_selected_curve->title().text());
            d_app->modifiedProject();
        } else
            QMessageBox::warning(d_graph, tr("Warning"),
                                 tr("This operation cannot be performed on curves plotted from "
                                    "columns having a non-numerical format."));
    }
    return QwtPlotPicker::end(ok);
}

void DataPickerTool::moveBy(int dx, int dy)
{
    if (!d_selected_curve)
        return;
    move(d_move_target_pos + QPoint(dx, dy));
    end(true);
}

QwtText DataPickerTool::trackerText(const QwtDoublePoint &point) const
{
    return plot()->axisScaleDraw(xAxis())->label(point.x()).text() + ", "
            + plot()->axisScaleDraw(yAxis())->label(point.y()).text();
}
