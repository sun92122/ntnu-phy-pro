#include "ApplicationWindowTest.h"
#include "FFT.h"
#include "MultiLayer.h"
#include <QMdiArea>
#include <iostream>

#include "utils.h"

TEST_F(ApplicationWindowTest, fft)
{
    auto table = newTable("1", 30, 2);
    table->setColName(0, "x");
    table->setColName(1, "y");

    auto &colX = *table->column(0);
    auto &colY = *table->column(1);
    for (int r = 0; r < table->numRows(); ++r) {
        colX.setValueAt(r, r);
        colY.setValueAt(r, sin(r));
    }

    auto graph = new Graph(this);
    graph->insertCurve(table, "x", "y", Graph::Line);

    auto fft1 = new FFT(this, table, "y");
    fft1->run();
    // FFT creates a hidden table, which we need to find.
    Table *table1 = nullptr;
    for (auto i : windowsList())
        if (auto t = dynamic_cast<Table *>(i))
            if (t != table) {
                table1 = t;
                break;
            }

    EXPECT_TRUE(table1);

    auto fft2 = new FFT(this, graph, "y");
    fft2->run();
    Table *table2 = nullptr;
    for (auto i : windowsList())
        if (auto t = dynamic_cast<Table *>(i))
            if (t != table && t != table1) {
                table2 = t;
                break;
            }

    EXPECT_TRUE(table2);

    if (!table1 || !table2)
        return;
    // check for data equality of tables
    EXPECT_EQ(table1->numCols(), table2->numCols());
    EXPECT_EQ(table1->numRows(), table2->numRows());
    for (int c = 0; c < table1->numCols(); ++c) {
        auto &col1 = *table1->column(c);
        auto &col2 = *table2->column(c);
        for (int r = 0; r < table1->numRows(); ++r)
            EXPECT_EQ(col1.valueAt(r), col2.valueAt(r));
    }
}
