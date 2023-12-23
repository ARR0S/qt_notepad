#pragma once
#include <QWidget>
#include <QPlainTextEdit>
#include <QPainter>
#include <QTextBlock>


class Editor : public QPlainTextEdit
{
    Q_OBJECT
        QWidget* lineNumberArea;
public:
    Editor(QWidget* parent = nullptr);

    void lineNumberAreaPaintEvent(QPaintEvent*);
    int lineNumberAreaWidth();

protected:
    void resizeEvent(QResizeEvent* event) override;

private slots:
    void changeLineNumberAreaWidth(int);
    void currLine();
    void changeLineNumberArea(const QRect&, int);
};

