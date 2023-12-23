#include "QtNotepad.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    QtNotepad w;
    w.show();
    return a.exec();
}
