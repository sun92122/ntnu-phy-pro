/***************************************************************************
    File                 : Column.cpp
    Project              : SciDAVis
    Description          : Aspect that manages a column
    --------------------------------------------------------------------
    Copyright            : (C) 2007-2009 Tilman Benkert (thzs*gmx.net)
                           (replace * with @ in the email addresses)

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

#include "core/column/Column.h"
#include "core/column/ColumnPrivate.h"
#include "core/column/columncommands.h"
#include "lib/XmlStreamReader.h"
#include <QIcon>
#include <QXmlStreamWriter>
#include <QtDebug>

Column::Column(const QString &name, SciDAVis::ColumnMode mode) : AbstractColumn(name)
{
    d_column_private = new Private(this, mode);
    d_string_io = new ColumnStringIO(this);
    d_column_private->inputFilter()->input(0, d_string_io);
    outputFilter()->input(0, this);
    addChild(d_column_private->inputFilter());
    addChild(outputFilter());
}

template<>
void Column::initPrivate(std::unique_ptr<QVector<qreal>> d, IntervalAttribute<bool> v)
{
    d_column_private = new Private(this, SciDAVis::TypeDouble, SciDAVis::ColumnMode::Numeric, d.release(), v);
}

template<>
void Column::initPrivate(std::unique_ptr<QStringList> d, IntervalAttribute<bool> v)
{
    d_column_private =
            new Private(this, SciDAVis::TypeQString, SciDAVis::ColumnMode::Text, d.release(), v);
}

template<>
void Column::initPrivate(std::unique_ptr<QList<QDateTime>> d, IntervalAttribute<bool> v)
{
    d_column_private = new Private(this, SciDAVis::TypeQDateTime, SciDAVis::ColumnMode::DateTime,
                                   d.release(), v);
}

void Column::init()
{
    d_string_io = new ColumnStringIO(this);
    d_column_private->inputFilter()->input(0, d_string_io);
    outputFilter()->input(0, this);
    addChild(d_column_private->inputFilter());
    addChild(outputFilter());
}

Column::~Column()
{
    delete d_string_io;
    delete d_column_private;
}

void Column::setColumnMode(SciDAVis::ColumnMode mode, AbstractFilter *conversion_filter)
{
    if (mode != columnMode()) // mode changed
    {
        beginMacro(QObject::tr("%1: change column type").arg(name()));
        AbstractSimpleFilter *old_input_filter = inputFilter();
        AbstractSimpleFilter *old_output_filter = outputFilter();
        exec(new ColumnSetModeCmd(d_column_private, mode, conversion_filter));
        if (d_column_private->inputFilter() != old_input_filter) {
            removeChild(old_input_filter);
            addChild(d_column_private->inputFilter());
            d_column_private->inputFilter()->input(0, d_string_io);
        }
        if (outputFilter() != old_output_filter) {
            removeChild(old_output_filter);
            addChild(outputFilter());
            outputFilter()->input(0, this);
        }
        endMacro();
    }
    // mode is not changed, but DateTime numeric converter changed
    else if ((mode == SciDAVis::ColumnMode::DateTime) && (nullptr != conversion_filter)) {
        auto numeric_datetime_converter =
                reinterpret_cast<NumericDateTimeBaseFilter *>(conversion_filter);
        if (nullptr != numeric_datetime_converter)
            // the ownership of converter is taken
            d_column_private->setNumericDateTimeFilter(numeric_datetime_converter);
    }
}

bool Column::copy(const AbstractColumn *other)
{
    Q_CHECK_PTR(other);
    if (other->dataType() != dataType())
        return false;
    exec(new ColumnFullCopyCmd(d_column_private, other));
    return true;
}

bool Column::copy(const AbstractColumn *source, int source_start, int dest_start, int num_rows)
{
    Q_CHECK_PTR(source);
    if (source->dataType() != dataType())
        return false;
    exec(new ColumnPartialCopyCmd(d_column_private, source, source_start, dest_start, num_rows));
    return true;
}

void Column::insertRows(int before, int count)
{
    if (count > 0)
        exec(new ColumnInsertEmptyRowsCmd(d_column_private, before, count));
}

void Column::removeRows(int first, int count)
{
    if (count > 0)
        exec(new ColumnRemoveRowsCmd(d_column_private, first, count));
}

void Column::setPlotDesignation(SciDAVis::PlotDesignation pd)
{
    if (pd != plotDesignation())
        exec(new ColumnSetPlotDesignationCmd(d_column_private, pd));
}

void Column::clear()
{
    exec(new ColumnClearCmd(d_column_private));
}

void Column::notifyReplacement(const AbstractColumn *replacement)
{
    emit aboutToBeReplaced(this, replacement);
}

void Column::clearValidity()
{
    exec(new ColumnClearValidityCmd(d_column_private));
}

void Column::clearMasks()
{
    exec(new ColumnClearMasksCmd(d_column_private));
}

void Column::setInvalid(Interval<int> i, bool invalid)
{
    exec(new ColumnSetInvalidCmd(d_column_private, i, invalid));
}

void Column::setInvalid(int row, bool invalid)
{
    setInvalid(Interval<int>(row, row), invalid);
}

void Column::setMasked(Interval<int> i, bool mask)
{
    exec(new ColumnSetMaskedCmd(d_column_private, i, mask));
}

void Column::setMasked(int row, bool mask)
{
    setMasked(Interval<int>(row, row), mask);
}

void Column::setFormula(Interval<int> i, QString formula)
{
    exec(new ColumnSetFormulaCmd(d_column_private, i, formula));
}

void Column::setFormula(int row, QString formula)
{
    setFormula(Interval<int>(row, row), formula);
}

void Column::clearFormulas()
{
    exec(new ColumnClearFormulasCmd(d_column_private));
}

void Column::setTextAt(int row, const QString &new_value)
{
    exec(new ColumnSetTextCmd(d_column_private, row, new_value));
}

void Column::replaceTexts(int first, const QStringList &new_values)
{
    if (!new_values.isEmpty())
        exec(new ColumnReplaceTextsCmd(d_column_private, first, new_values));
}

void Column::setDateAt(int row, const QDate &new_value)
{
    setDateTimeAt(row, QDateTime(new_value, timeAt(row)));
}

void Column::setTimeAt(int row, const QTime &new_value)
{
    setDateTimeAt(row, QDateTime(dateAt(row), new_value));
}

void Column::setDateTimeAt(int row, const QDateTime &new_value)
{
    exec(new ColumnSetDateTimeCmd(d_column_private, row, new_value));
}

void Column::replaceDateTimes(int first, const QList<QDateTime> &new_values)
{
    if (!new_values.isEmpty())
        exec(new ColumnReplaceDateTimesCmd(d_column_private, first, new_values));
}

void Column::setValueAt(int row, double new_value)
{
    exec(new ColumnSetValueCmd(d_column_private, row, new_value));
}

void Column::replaceValues(int first, const QVector<qreal> &new_values)
{
    if (!new_values.isEmpty())
        exec(new ColumnReplaceValuesCmd(d_column_private, first, new_values));
}

QString Column::textAt(int row) const
{
    return d_column_private->textAt(row);
}

QDate Column::dateAt(int row) const
{
    return d_column_private->dateAt(row);
}

QTime Column::timeAt(int row) const
{
    return d_column_private->timeAt(row);
}

QDateTime Column::dateTimeAt(int row) const
{
    return d_column_private->dateTimeAt(row);
}

double Column::valueAt(int row) const
{
    return d_column_private->valueAt(row);
}

QIcon Column::icon() const
{
    switch (dataType()) {
    case SciDAVis::TypeDouble:
        return QIcon(QPixmap(":/numerictype.png"));
    case SciDAVis::TypeQString:
        return QIcon(QPixmap(":/texttype.png"));
    case SciDAVis::TypeQDateTime:
        return QIcon(QPixmap(":/datetype.png"));
    }
    return QIcon();
}

void Column::save(QXmlStreamWriter *writer) const
{
    writer->writeStartElement("column");
    writeBasicAttributes(writer);
    writer->writeAttribute("type", SciDAVis::enumValueToString(dataType(), "ColumnDataType"));
    writer->writeAttribute("mode", SciDAVis::enumValueToString(static_cast<int>(columnMode()), "ColumnMode"));
    writer->writeAttribute("plot_designation",
                           SciDAVis::enumValueToString(plotDesignation(), "PlotDesignation"));
    writeCommentElement(writer);
    writer->writeStartElement("input_filter");
    d_column_private->inputFilter()->save(writer);
    writer->writeEndElement();
    writer->writeStartElement("output_filter");
    outputFilter()->save(writer);
    writer->writeEndElement();
    QList<Interval<int>> masks = maskedIntervals();
    foreach (Interval<int> interval, masks) {
        writer->writeStartElement("mask");
        writer->writeAttribute("start_row", QString::number(interval.start()));
        writer->writeAttribute("end_row", QString::number(interval.end()));
        writer->writeEndElement();
    }
    QList<Interval<int>> formulas = formulaIntervals();
    foreach (Interval<int> interval, formulas) {
        writer->writeStartElement("formula");
        writer->writeAttribute("start_row", QString::number(interval.start()));
        writer->writeAttribute("end_row", QString::number(interval.end()));
        writer->writeCharacters(formula(interval.start()));
        writer->writeEndElement();
    }
    int i;
    switch (dataType()) {
    case SciDAVis::TypeDouble:
        for (i = 0; i < rowCount(); i++) {
            writer->writeStartElement("row");
            writer->writeAttribute("type",
                                   SciDAVis::enumValueToString(dataType(), "ColumnDataType"));
            writer->writeAttribute("index", QString::number(i));
            writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
            writer->writeCharacters(QString::number(valueAt(i), 'e', 16));
            writer->writeEndElement();
        }
        break;
    case SciDAVis::TypeQString:
        for (i = 0; i < rowCount(); i++) {
            writer->writeStartElement("row");
            writer->writeAttribute("type",
                                   SciDAVis::enumValueToString(dataType(), "ColumnDataType"));
            writer->writeAttribute("index", QString::number(i));
            writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
            writer->writeCharacters(textAt(i));
            writer->writeEndElement();
        }
        break;

    case SciDAVis::TypeQDateTime: {
        { // this conversion is needed to store base class;
            NumericDateTimeBaseFilter numericFilter(
                    *(d_column_private->getNumericDateTimeFilter()));
            writer->writeStartElement("numericDateTimeFilter");
            numericFilter.save(writer);
            writer->writeEndElement();
        }
        for (i = 0; i < rowCount(); i++) {
            writer->writeStartElement("row");
            writer->writeAttribute("type",
                                   SciDAVis::enumValueToString(dataType(), "ColumnDataType"));
            writer->writeAttribute("index", QString::number(i));
            writer->writeAttribute("invalid", isInvalid(i) ? "yes" : "no");
#if QT_VERSION < 0x040400 // avoid a bug in Qt < 4.4
            QString str = dateTimeAt(i).toString("yyyy-dd-MM hh:mm:ss:zzz");
            int should_be_length = 4;
            int actual_length = dateTimeAt(i).toString("yyyy").length();
            while (actual_length < should_be_length) {
                str.prepend("0");
                actual_length++;
            }
            writer->writeCharacters(str);
#else
            writer->writeCharacters(dateTimeAt(i).toString("yyyy-dd-MM hh:mm:ss:zzz"));
#endif
            writer->writeEndElement();
        }
        break;
    }
    }
    writer->writeEndElement(); // "column"
}

bool Column::load(XmlStreamReader *reader)
{
    if (reader->isStartElement() && reader->name() == "column") {
        if (!readBasicAttributes(reader))
            return false;

        QXmlStreamAttributes attribs = reader->attributes();
        QString str;

        // read type
        str = attribs.value(reader->namespaceUri().toString(), "type").toString();
        if (str.isEmpty()) {
            reader->raiseError(tr("column type missing"));
            return false;
        }
        int type_code = SciDAVis::enumStringToValue(str, "ColumnDataType");
        if (type_code == -1) {
            reader->raiseError(tr("column type invalid"));
            return false;
        }
        // read mode
        str = attribs.value(reader->namespaceUri().toString(), "mode").toString();
        if (str.isEmpty()) {
            reader->raiseError(tr("column mode missing"));
            return false;
        }
        int mode_code = SciDAVis::enumStringToValue(str, "ColumnMode");
        if (mode_code == -1) {
            reader->raiseError(tr("column mode invalid"));
            return false;
        }
        setColumnMode((SciDAVis::ColumnMode)mode_code);
        if (type_code != int(dataType())) {
            reader->raiseError(tr("column type or mode invalid"));
            return false;
        }
        // read plot designation
        str = attribs.value(reader->namespaceUri().toString(), "plot_designation").toString();
        int pd_code = SciDAVis::enumStringToValue(str, "PlotDesignation");
        if (str.isEmpty())
            setPlotDesignation(SciDAVis::noDesignation);
        else if (pd_code == -1) {
            reader->raiseError(tr("column plot designation invalid"));
            return false;
        } else
            setPlotDesignation((SciDAVis::PlotDesignation)pd_code);

        setComment("");
        if (rowCount() > 0)
            removeRows(0, rowCount());
        clearValidity();
        clearMasks();
        clearFormulas();
        // read child elements
        while (!reader->atEnd()) {
            reader->readNext();

            if (reader->isEndElement())
                break;

            if (reader->isStartElement()) {
                bool ret_val = true;
                if (reader->name() == "comment")
                    ret_val = readCommentElement(reader);
                else if (reader->name() == "numericDateTimeFilter")
                    ret_val = XmlReadNumericDateTimeFilter(reader);
                else if (reader->name() == "input_filter")
                    ret_val = XmlReadInputFilter(reader);
                else if (reader->name() == "output_filter")
                    ret_val = XmlReadOutputFilter(reader);
                else if (reader->name() == "mask")
                    ret_val = XmlReadMask(reader);
                else if (reader->name() == "formula")
                    ret_val = XmlReadFormula(reader);
                else if (reader->name() == "row")
                    ret_val = XmlReadRow(reader);
                else // unknown element
                {
                    reader->raiseWarning(tr("unknown element '%1'").arg(reader->name().toString()));
                    if (!reader->skipToEndElement())
                        return false;
                }
                if (!ret_val)
                    return false;
            }
        }
    } else // no column element
        reader->raiseError(tr("no column element found"));

    return !reader->error();
}

bool Column::XmlReadNumericDateTimeFilter(XmlStreamReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "numericDateTimeFilter");
    if (!reader->skipToNextTag())
        return false;
    if (!d_column_private->getNumericDateTimeFilter()->load(reader))
        return false;
    if (!reader->skipToNextTag())
        return false;
    Q_ASSERT(reader->isEndElement() && reader->name() == "numericDateTimeFilter");
    return true;
}

bool Column::XmlReadInputFilter(XmlStreamReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "input_filter");
    if (!reader->skipToNextTag())
        return false;
    if (!d_column_private->inputFilter()->load(reader))
        return false;
    if (!reader->skipToNextTag())
        return false;
    Q_ASSERT(reader->isEndElement() && reader->name() == "input_filter");
    return true;
}

bool Column::XmlReadOutputFilter(XmlStreamReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "output_filter");
    if (!reader->skipToNextTag())
        return false;
    if (!outputFilter()->load(reader))
        return false;
    if (!reader->skipToNextTag())
        return false;
    Q_ASSERT(reader->isEndElement() && reader->name() == "output_filter");
    return true;
}

bool Column::XmlReadMask(XmlStreamReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "mask");

    bool ok1, ok2;
    int start, end;
    start = reader->readAttributeInt("start_row", &ok1);
    end = reader->readAttributeInt("end_row", &ok2);
    if (!ok1 || !ok2) {
        reader->raiseError(tr("invalid or missing start or end row"));
        return false;
    }
    setMasked(Interval<int>(start, end));
    if (!reader->skipToEndElement())
        return false;

    return true;
}

bool Column::XmlReadFormula(XmlStreamReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "formula");

    bool ok1, ok2;
    int start, end;
    start = reader->readAttributeInt("start_row", &ok1);
    end = reader->readAttributeInt("end_row", &ok2);
    if (!ok1 || !ok2) {
        reader->raiseError(tr("invalid or missing start or end row"));
        return false;
    }
    setFormula(Interval<int>(start, end), reader->readElementText());

    return true;
}

bool Column::XmlReadRow(XmlStreamReader *reader)
{
    Q_ASSERT(reader->isStartElement() && reader->name() == "row");

    QString str;
    int type_code;

    QXmlStreamAttributes attribs = reader->attributes();
    // verfiy type
    str = attribs.value(reader->namespaceUri().toString(), "type").toString();
    type_code = SciDAVis::enumStringToValue(str, "ColumnDataType");
    if (str.isEmpty() || type_code == -1 || type_code != int(dataType())) {
        reader->raiseError(tr("invalid or missing row type"));
        return false;
    }

    bool ok;
    int index = reader->readAttributeInt("index", &ok);
    if (!ok) {
        reader->raiseError(tr("invalid or missing row index"));
        return false;
    }

    str = reader->readElementText();
    switch (dataType()) {
    case SciDAVis::TypeDouble: {
        double value = str.toDouble(&ok);
        if (!ok) {
            reader->raiseError(tr("invalid row value"));
            return false;
        }
        setValueAt(index, value);
        break;
    }
    case SciDAVis::TypeQString:
        setTextAt(index, str);
        break;

    case SciDAVis::TypeQDateTime:
        QDateTime date_time = QDateTime::fromString(str, "yyyy-dd-MM hh:mm:ss:zzz");
        setDateTimeAt(index, date_time);
        break;
    }

    str = attribs.value(reader->namespaceUri().toString(), "invalid").toString();
    if (str == "yes")
        setInvalid(index);

    return true;
}
SciDAVis::ColumnDataType Column::dataType() const
{
    return d_column_private->dataType();
}

SciDAVis::ColumnMode Column::columnMode() const
{
    return d_column_private->columnMode();
}
int Column::rowCount() const
{
    return d_column_private->rowCount();
}

SciDAVis::PlotDesignation Column::plotDesignation() const
{
    return d_column_private->plotDesignation();
}

AbstractSimpleFilter *Column::outputFilter() const
{
    return d_column_private->outputFilter();
}

AbstractSimpleFilter *Column::inputFilter() const
{
    return d_column_private->inputFilter();
}

NumericDateTimeBaseFilter *Column::numericDateTimeBaseFilter() const
{
    return d_column_private->getNumericDateTimeFilter();
}

bool Column::isInvalid(int row) const
{
    return d_column_private->isInvalid(row);
}

bool Column::isInvalid(Interval<int> i) const
{
    return d_column_private->isInvalid(i);
}

QList<Interval<int>> Column::invalidIntervals() const
{
    return d_column_private->invalidIntervals();
}

bool Column::isMasked(int row) const
{
    return d_column_private->isMasked(row);
}

bool Column::isMasked(Interval<int> i) const
{
    return d_column_private->isMasked(i);
}

QList<Interval<int>> Column::maskedIntervals() const
{
    return d_column_private->maskedIntervals();
}

QString Column::formula(int row) const
{
    return d_column_private->formula(row);
}

QList<Interval<int>> Column::formulaIntervals() const
{
    return d_column_private->formulaIntervals();
}

void Column::notifyDisplayChange()
{
    emit dataChanged(this); // all cells must be repainted
    emit aspectDescriptionChanged(this); // the icon for the type changed
}

QString ColumnStringIO::textAt(int row) const
{
    if (d_setting)
        return d_to_set;
    else
        return d_owner->d_column_private->outputFilter()->output(0)->textAt(row);
}

void ColumnStringIO::setTextAt(int row, const QString &value)
{
    d_setting = true;
    d_to_set = value;
    d_owner->copy(d_owner->d_column_private->inputFilter()->output(0), 0, row, 1);
    d_setting = false;
    d_to_set.clear();
}

bool ColumnStringIO::copy(const AbstractColumn *other)
{
    if (other->columnMode() != SciDAVis::ColumnMode::Text)
        return false;
    d_owner->d_column_private->inputFilter()->input(0, other);
    d_owner->copy(d_owner->d_column_private->inputFilter()->output(0));
    d_owner->d_column_private->inputFilter()->input(0, this);
    return true;
}

bool ColumnStringIO::copy(const AbstractColumn *source, int source_start, int dest_start,
                          int num_rows)
{
    if (source->columnMode() != SciDAVis::ColumnMode::Text)
        return false;
    d_owner->d_column_private->inputFilter()->input(0, source);
    d_owner->copy(d_owner->d_column_private->inputFilter()->output(0), source_start, dest_start,
                  num_rows);
    d_owner->d_column_private->inputFilter()->input(0, this);
    return true;
}

void ColumnStringIO::replaceTexts(int start_row, const QStringList &texts)
{
    Column tmp("tmp", texts);
    copy(&tmp, 0, start_row, texts.size());
}
