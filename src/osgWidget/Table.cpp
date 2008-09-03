// -*-c++-*- osgWidget - Code by: Jeremy Moles (cubicool) 2007-2008

#include <osgWidget/Table>

namespace osgWidget {

// TODO: There is a serious, outstanding bug with regards to USING a table before ALL Widgets
// are set! FIX THIS!!!

Table::Table(const std::string& name, unsigned int rows, unsigned int cols):
Window      (name),
_rows       (rows),
_cols       (cols),
_lastRowAdd (0),
_lastColAdd (0) {
    _objects.resize(_rows * _cols);
}

Table::Table(const Table& table, const osg::CopyOp& co):
Window      (table, co),
_rows       (table._rows),
_cols       (table._cols),
_lastRowAdd (table._lastRowAdd),
_lastColAdd (table._lastColAdd) {
}

unsigned int Table::_calculateIndex(unsigned int row, unsigned int col) const {
    return (row * _cols) + col;
}

void Table::_getRows(CellSizes& rows, Getter get) const {
    for(unsigned int i = 0; i < _rows; i++) rows.push_back(
        _compare<Greater>(get, i * _cols, (i * _cols) + _cols)
    );
}

void Table::_getColumns(CellSizes& cols, Getter get) const {
    for(unsigned int i = 0; i < _cols; i++) cols.push_back(
        _compare<Greater>(get, i, 0, _cols)
    );
}

void Table::_resizeImplementation(point_type width, point_type height) {
    // We use these vectors so that we don't have to repeatedly call isFillable
    // all the time. Usage such as this can really generate a lot of moronic,
    // misinformed opposition, but until std::bit_vector is available, this is
    // what we get. Deal with it.
    std::vector<bool> rowFills;
    std::vector<bool> colFills;

    point_type numRowFills = 0.0f;
    point_type numColFills = 0.0f;

    // Enumerate each row and determine whether it can fill. If so, increment
    // our numRowFills variable and set the position in rowFills to "true."
    for(unsigned int row = 0; row < _rows; row++) {
        bool fill = isRowVerticallyFillable(row);

        if(fill) numRowFills++;

        rowFills.push_back(fill);
    }

    // Enumerate each column and determine whether it can fill. If so, increment
    // our numColFills variable and set the position in colFills to "true."
    for(unsigned int col = 0; col < _cols; col++) {
        bool fill = isColumnHorizontallyFillable(col);

        if(fill) numColFills++;

        colFills.push_back(fill);
    }

    int wrem = 0;
    int hrem = 0;

    if(numRowFills > 0.0f) {
        hrem = static_cast<int>(height) % static_cast<int>(numRowFills);

        unsigned int cur = 0;

        for(unsigned int row = 0; row < _rows; row++) {
            point_type h = height / numRowFills;
        
            if(cur >= _lastRowAdd && hrem) {
                _lastRowAdd++;
                h++;
                hrem--;
            }

            if(rowFills[row]) addHeightToRow(row, h);

            cur++;
        }
    }

    if(numColFills > 0.0f) {
        wrem = static_cast<int>(width) % static_cast<int>(numColFills);

        unsigned int cur = 0;

        for(unsigned int col = 0; col < _cols; col++) {
            point_type w = width / numColFills;

            if(cur >= _lastColAdd && wrem) {
                _lastColAdd++;
                w++;
                wrem--;
            }

            if(colFills[col]) addWidthToColumn(col, w);

            cur++;
        }
    }

    CellSizes rowHeights;
    CellSizes colWidths;

    getRowHeights(rowHeights);
    getColumnWidths(colWidths);

    point_type y = 0.0f;

    for(unsigned int row = 0; row < _rows; row++) {
        point_type x = 0.0f;

        for(unsigned int col = 0; col < _cols; col++) {
            Widget* widget = _objects[_calculateIndex(row, col)].get();

            if(widget) {
                widget->setOrigin(x, y);

                _positionWidget(widget, colWidths[col], rowHeights[row]);
            }

            x += colWidths[col];
        }

        y += rowHeights[row];
    }
}

Window::Sizes Table::_getWidthImplementation() const {
    CellSizes cols;
    CellSizes minCols;

    getColumnWidths(cols);
    getColumnMinWidths(minCols);
    
    return Sizes(
        std::accumulate(cols.begin(), cols.end(), 0.0f, Plus()),
        std::accumulate(minCols.begin(), minCols.end(), 0.0f, Plus())
    );
}

Window::Sizes Table::_getHeightImplementation() const {
    CellSizes rows;
    CellSizes minRows;

    getRowHeights(rows);
    getRowMinHeights(minRows);

    return Sizes(
        std::accumulate(rows.begin(), rows.end(), 0.0f, Plus()),
        std::accumulate(minRows.begin(), minRows.end(), 0.0f, Plus())
    );
}

bool Table::addWidget(Widget* widget) {
    return addWidget(widget, 0, 0);
}

bool Table::addWidget(Widget* widget, unsigned int row, unsigned int col) {
    return Window::insertWidget(widget, _calculateIndex(row, col));
}

void Table::getRowHeights(CellSizes& rowHeights) const {
    _getRows(rowHeights, &Widget::getHeightTotal);
}

void Table::getRowMinHeights(CellSizes& rowMinHeights) const {
    _getRows(rowMinHeights, &Widget::getMinHeightTotal);
}

void Table::getColumnWidths(CellSizes& colWidths) const {
    _getColumns(colWidths, &Widget::getWidthTotal);
}

void Table::getColumnMinWidths(CellSizes& colMinWidths) const {
    _getColumns(colMinWidths, &Widget::getMinWidthTotal);
}

void Table::addHeightToRow(unsigned int row, point_type height) {
    for(
        Iterator i = begin() + (row * _cols);
        i != begin() + ((row * _cols) + _cols);
        i++
    ) if(i->valid()) i->get()->addHeight(height);
}

void Table::addWidthToColumn(unsigned int col, point_type width) {
    // See the documentation in include/osgWidget/Window::_forEachApplyOrAssign if you want
    // to know why we need this variable.
    unsigned int c = col;

    for(Iterator i = begin() + col; i < end(); c += _cols) {
        if(i->valid()) i->get()->addWidth(width);

        if((c + _cols) < size()) i += _cols;

        else i = end();
    }
}

bool Table::isRowVerticallyFillable(unsigned int row) const {
    return static_cast<point_type>(_cols) == _getNumFill(row * _cols, (row * _cols) + _cols);
}

bool Table::isColumnHorizontallyFillable(unsigned int col) const {
    return static_cast<point_type>(_rows) == _getNumFill(col, 0, _cols);
}

}
