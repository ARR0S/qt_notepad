#include "Editor.h"
#include <QPainter>
#include <QTextBlock>
#include "NumberArea.h"


Editor::Editor(QWidget* parent) : QPlainTextEdit(parent)
{
    lineNumberArea = new NumberArea(this);

    connect(this, SIGNAL(blockCountChanged(int)), SLOT(changeLineNumberAreaWidth(int)));
    connect(this, SIGNAL(updateRequest(QRect, int)), SLOT(changeLineNumberArea(QRect, int)));
    connect(this, SIGNAL(cursorPositionChanged()), SLOT(currLine()));

    changeLineNumberAreaWidth(0);
    currLine();
}

int Editor::lineNumberAreaWidth()
{
    int numbers = 1;
    int m = qMax(1, blockCount());
    while (m >= 10)
    {
        m /= 10;
        ++numbers;
    }
    return 3 + numbers * fontMetrics().horizontalAdvance(QLatin1Char('9'));
}

void Editor::changeLineNumberAreaWidth(int)
{
    setViewportMargins(lineNumberAreaWidth(), 0, 0, 0);
}

void Editor::changeLineNumberArea(const QRect& rect, int n)
{
    if (n) lineNumberArea->scroll(0, n);
    else lineNumberArea->update(0, rect.y(), lineNumberArea->width(), rect.height());
    if (rect.contains(viewport()->rect())) changeLineNumberAreaWidth(0);
}

void Editor::resizeEvent(QResizeEvent* event)
{
    QPlainTextEdit::resizeEvent(event);
    QRect rect = contentsRect();
    lineNumberArea->setGeometry(QRect(rect.left(), rect.top(), lineNumberAreaWidth(), rect.height()));
}

void Editor::currLine()
{
    QList<QTextEdit::ExtraSelection> selections;
    if (!isReadOnly())
    {
        QTextEdit::ExtraSelection selection;
        selection.format.setProperty(QTextFormat::FullWidthSelection, true);
        selection.format.setBackground(QColor(Qt::yellow).lighter(180));
        selection.cursor = textCursor();
        selection.cursor.clearSelection();
        selections.append(selection);
    }
    setExtraSelections(selections);
}

void Editor::lineNumberAreaPaintEvent(QPaintEvent* event)
{
    QPainter painter(lineNumberArea);
    QTextBlock block = firstVisibleBlock();
    int top = (int)blockBoundingGeometry(block).translated(contentOffset()).top();
    int bottom = top + (int)blockBoundingRect(block).height();
    int number = block.blockNumber();

    painter.fillRect(event->rect(), Qt::lightGray);

    while (block.isValid() && top <= event->rect().bottom())
    {
        if (block.isVisible() && bottom >= event->rect().top())
        {
            painter.setPen(Qt::black);
            painter.drawText(0, top, lineNumberArea->width(), fontMetrics().height(),
                Qt::AlignRight, QString::number(number + 1));
        }
        block = block.next();
        top = bottom;
        bottom = top + (int)blockBoundingRect(block).height();
        ++number;
    }
}