#include "NumberArea.h"
NumberArea::NumberArea(Editor* parent) : QWidget(parent), editor(parent){}

QSize NumberArea::sizeHint() const
{
    return QSize(editor->lineNumberAreaWidth(), 0);
}

void NumberArea::paintEvent(QPaintEvent* event)
{
    editor->lineNumberAreaPaintEvent(event);
}