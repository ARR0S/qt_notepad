#include "Menu.h"
Menu::Menu(QWidget* parent) : QDialog(parent)
{
    table = new QTableWidget(this);
    QStringList header{ "File", "Path" };
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(header);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->setSectionResizeMode(QHeaderView::Fixed);

    QDialogButtonBox* btn = new QDialogButtonBox(QDialogButtonBox::Yes | QDialogButtonBox::No);
    connect(btn, SIGNAL(accepted()), SLOT(accept()));
    connect(btn, SIGNAL(rejected()), SLOT(reject()));

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(table);
    layout->addWidget(new QLabel("Save changes?"));
    layout->addWidget(btn);
    setLayout(layout);
}

void Menu::fill(const QVector<QDir>& paths, const QStringList& names)
{
    table->clearContents();
    for (int i = 0; i < paths.count(); ++i)
    {
        table->insertRow(i);
        table->setItem(i, 0, new QTableWidgetItem(names[i], Qt::DisplayRole));
        table->setItem(i, 1, new QTableWidgetItem(paths[i].path(), Qt::DisplayRole));
    }
}

void Menu::closeEvent(QCloseEvent* event)
{
    emit done(-1);
}