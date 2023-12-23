#pragma once
#include "editor.h"


class NumberArea : public QWidget
{
    Editor* editor;
public:
    NumberArea(Editor*);
    QSize sizeHint() const override;

protected:
    void paintEvent(QPaintEvent* event) override;
};

