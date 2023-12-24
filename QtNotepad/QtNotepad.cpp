#include "QtNotepad.h"

QtNotepad::QtNotepad(QWidget * parent) : QMainWindow(parent)
{
    menu = nullptr;
    fileIndex = 1;
    setWindowIcon(QIcon(":/images/icon.ico"));
    setWindowTitle("QtNotepad");
    resize(800, 600);
    makeTabWidget();
    makeFileExplorerDock();
    makeOpenedFilesDock();
    makeActions();
    makeMenuBar();
    makeToolBar();
    setCentralWidget(tabWgt);
    label = new QLabel(this);
    statusBar()->addPermanentWidget(label);
    loadSettings();
}

void QtNotepad::closeEvent(QCloseEvent* event)
{
    bool flag = false;
    for (int i = 0; i < tabWgt->count(); ++i)
    {
        if (tabWgt->tabWhatsThis(i) != "Without changes")
        {
            flag = true;
            break;
        }
    }
    if (flag)
    {
        SaveDialog* dialog = createDialog();
        dialog->setModal(true);
        if (dialog->exec() == QDialog::Rejected) {
            delete dialog;
            event->ignore();
            return;
        }
        event->accept();
        delete dialog;
    }
    saveSettings();
}


void QtNotepad::makeTabWidget()
{
    tabWgt = new QTabWidget(this);
    tabWgt->setTabsClosable(true);
    tabWgt->setUsesScrollButtons(true);
    tabWgt->setMovable(true);
    connect(tabWgt, SIGNAL(tabCloseRequested(int)), this, SLOT(closeFile(int)));
}

void QtNotepad::makeActions()
{
    create = new QAction(QIcon(":/images/new_file.png"), tr("New"));
    open = new QAction(QIcon(":/images/open_file.png"), tr("Open"));
    save = new QAction(QIcon(":/images/save_file.png"), tr("Save"));
    saveAll = new QAction(QIcon(":/images/save_all_files.png"), tr("Save all"));
    close = new QAction(tr("Close"));
    saveAs = new QAction(tr("Save as"));
    closeAll = new QAction(tr("Close all"));
    exit = new QAction(tr("Exit"));

    create->setShortcut(QKeySequence("CTRL+N"));
    open->setShortcut(QKeySequence("CTRL+O"));
    save->setShortcut(QKeySequence("CTRL+S"));
    saveAll->setShortcut(QKeySequence("CTRL+SHIFT+S"));
    close->setShortcut(QKeySequence("CTRL+W"));
    saveAs->setShortcut(QKeySequence("CTRL+ALT+S"));
    closeAll->setShortcut(QKeySequence("CTRL+SHIFT+W"));
    exit->setShortcut(QKeySequence("CTRL+Q"));

    connect(create, SIGNAL(triggered()), SLOT(createFile()));
    connect(open, SIGNAL(triggered()), SLOT(openFile()));
    connect(save, SIGNAL(triggered()), SLOT(saveFile()));
    connect(saveAll, SIGNAL(triggered()), SLOT(saveAllFiles()));
    connect(close, SIGNAL(triggered()), SLOT(closeFile()));
    connect(saveAs, SIGNAL(triggered()), SLOT(saveFileAs()));
    connect(closeAll, SIGNAL(triggered()), SLOT(closeAllFiles()));
    connect(exit, SIGNAL(triggered()), SLOT(closeWindow()));
}

void QtNotepad::makeMenuBar()
{
    QMenu* fileMenu = new QMenu(tr("File"), this);
    QMenu* editMenu = new QMenu(tr("Edit"), this);
    QMenu* viewMenu = new QMenu(tr("View"), this);

    fileMenu->addAction(create);
    fileMenu->addAction(open);
    fileMenu->addAction(save);
    fileMenu->addAction(saveAs);
    fileMenu->addAction(saveAll);
    fileMenu->addAction(close);
    fileMenu->addAction(closeAll);
    fileMenu->addAction(exit);

    editMenu->addAction(tr("Cut"), this, SLOT(cut()), QKeySequence("CTRL+X"));
    editMenu->addAction(tr("Copy"), this, SLOT(copy()), QKeySequence("CTRL+C"));
    editMenu->addAction(tr("Paste"), this, SLOT(paste()), QKeySequence("CTRL+V"));
    editMenu->addAction(tr("Delete"), this, SLOT(clear()), QKeySequence("CTRL+DELETE"));
    editMenu->addAction(tr("Select all"), this, SLOT(selectAll()), QKeySequence("CTRL+A"));

    viewMenu->addAction(fileExplorer->toggleViewAction());
    viewMenu->addAction(openedFiles->toggleViewAction());

    menuBar()->addMenu(fileMenu);
    menuBar()->addMenu(editMenu);
    menuBar()->addMenu(viewMenu);
}

void QtNotepad::makeToolBar()
{
    QToolBar* toolBar = addToolBar(tr("File"));
    toolBar->addAction(create);
    toolBar->addAction(open);
    toolBar->addAction(save);
    toolBar->addAction(saveAll);
}

void QtNotepad::makeFileExplorerDock()
{
    filesModel = new QFileSystemModel(this);
    filesModel->setRootPath(QDir::rootPath());

    tree = new QTreeView(this);
    tree->setModel(filesModel);
    tree->setColumnHidden(1, true);
    tree->setColumnHidden(2, true);
    tree->setColumnHidden(3, true);
    tree->setHeaderHidden(true);
    connect(tree, SIGNAL(doubleClicked(QModelIndex)), SLOT(openFile(QModelIndex)));

    fileExplorer = new QDockWidget(tr("Explorer"), this);
    fileExplorer->setWidget(tree);
    fileExplorer->setMaximumWidth(700);
    fileExplorer->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    fileExplorer->hide();
    addDockWidget(Qt::LeftDockWidgetArea, fileExplorer);
}

void QtNotepad::makeOpenedFilesDock()
{
    currFiles = new QListWidget(this);

    connect(tabWgt->tabBar(), SIGNAL(tabMoved(int, int)), SLOT(changeTabIndex(int, int)));
    connect(tabWgt->tabBar(), SIGNAL(currentChanged(int)), SLOT(changeCurrIndex(int)));
    connect(tabWgt->tabBar(), SIGNAL(tabCloseRequested(int)), SLOT(changeIndexOnDelete()));
    connect(tabWgt->tabBar(), SIGNAL(tabMoved(int, int)), SLOT(changeContainers(int, int)));
    connect(currFiles, SIGNAL(itemClicked(QListWidgetItem*)), SLOT(changeCurrIndex(QListWidgetItem*)));
    connect(currFiles, SIGNAL(currentRowChanged(int)), tabWgt, SLOT(setCurrentIndex(int)));

    openedFiles = new QDockWidget(tr("Opened files"), this);
    openedFiles->setWidget(currFiles);
    openedFiles->setFeatures(QDockWidget::DockWidgetClosable | QDockWidget::DockWidgetMovable);
    openedFiles->hide();
    addDockWidget(Qt::LeftDockWidgetArea, openedFiles);
}

void QtNotepad::createFile()
{
    Editor* editor = new Editor(this);
    QString name = "Unnamed" + QString::number(fileIndex);

    int index = tabWgt->addTab(editor, name);
    tabWgt->setCurrentIndex(index);
    tabWgt->setTabToolTip(index, "");
    tabWgt->setTabWhatsThis(index, "Without changes");
    connect(editor, SIGNAL(textChanged()), SLOT(changeParameter()));

    QListWidgetItem* item = new QListWidgetItem;
    item->setText(tabWgt->tabText(index));
    item->setToolTip(tabWgt->tabToolTip(index));

    currFiles->addItem(item);
    changeCurrIndex(index);

    filepaths.push_back(QDir::current());
    filenames.push_back(name);
    fileIndex++;

    connect(editor, &Editor::cursorPositionChanged, this, &QtNotepad::statusBarChange);
}

void QtNotepad::openFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose file"), "");
    if (path.isEmpty())
    {
        QMessageBox::warning(this, tr("Error"), tr("There is no such path!"), QMessageBox::Ok);
        return;
    }
    openFile(path);
}

void QtNotepad::openFile(QModelIndex index)
{
    if (!filesModel->isDir(index)) openFile(filesModel->filePath(index));
}

void QtNotepad::openFile(const QString& path)
{
    for (int i = 0; i < tabWgt->count(); ++i)
    {
        if (tabWgt->tabToolTip(i) == path)
        {
            QMessageBox::warning(this, tr("Error"), tr("The file is already open!"), QMessageBox::Ok);
            return;
        }
    }

    QString name = path.section("/", -1, -1);
    QFile file(path);

    if (file.open(QIODevice::ReadOnly))
    {
        filepaths.push_back(QFileInfo(file).path());
        filenames.push_back(name);

        Editor* tmp = new Editor(this);
        QString buffer = file.readAll();
        QString extension = QFileInfo(name).suffix();

        if (!extension.isEmpty())
        {
            highlighter = new SyntaxHighlighter(extension, tmp->document());
            if (!highlighter->isSupported()) delete highlighter;
        }
        file.close();
        tmp->appendPlainText(buffer);

        int index = tabWgt->addTab(tmp, name);
        tabWgt->setCurrentIndex(index);
        tabWgt->setTabWhatsThis(index, "Without changes");
        tabWgt->setTabToolTip(index, path);
        tabWgt->repaint();
        QListWidgetItem* item = new QListWidgetItem;
        item->setText(tabWgt->tabText(index));
        item->setToolTip(tabWgt->tabToolTip(index));
        currFiles->addItem(item);
        changeCurrIndex(index);
        connect(tmp, SIGNAL(textChanged()), SLOT(changeParameter()));
        connect(tmp, &Editor::cursorPositionChanged, this, &QtNotepad::statusBarChange);
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("Can't open the file!"), QMessageBox::Ok);
        return;
    }
}

void QtNotepad::saveFile()
{
    if (tabWgt->tabToolTip(tabWgt->currentIndex()) == "")
    {
        saveFileAs();
        return;
    }
    QString path = tabWgt->tabToolTip(tabWgt->currentIndex());
    QFile file(path);
    if (file.open(QIODevice::WriteOnly))
    {
        Editor* curr = qobject_cast<Editor*>(tabWgt->currentWidget());
        if (curr)
        {
            file.write(curr->document()->toPlainText().toUtf8());
            file.close();
            tabWgt->setTabWhatsThis(tabWgt->currentIndex(), "Without changes");
            tabWgt->setTabText(tabWgt->currentIndex(), path.section("/", -1, -1));
        }
    }
    else
    {
        QMessageBox::warning(this, tr("Error"), tr("Can't save the file!"), QMessageBox::Ok);
        return;
    }
}

void QtNotepad::saveFileAs()
{
    QString name = tabWgt->tabText(tabWgt->currentIndex()).remove('*');
    QString path = QFileDialog::getSaveFileName(this, "Save " + name, name);
    if (path.isEmpty()) return;
    if (QFileInfo(path).suffix().isEmpty()) path.append(".txt");
    else
    {
        QFile file(path);
        if (file.open(QIODevice::WriteOnly))
        {
            Editor* curr = qobject_cast<Editor*>(tabWgt->currentWidget());
            if (curr)
            {
                file.write(curr->document()->toPlainText().toUtf8());
                file.close();
            }
        }
        else
        {
            QMessageBox::warning(this, tr("Error"), tr("Can't save the file!"), QMessageBox::Ok);
            return;
        }
    }
    filepaths.replace(tabWgt->currentIndex(), QDir(QFileInfo(path).path()));
    filenames.replace(tabWgt->currentIndex(), QFileInfo(path).fileName());
    name = path.section("/", -1, -1);

    tabWgt->tabBar()->setTabText(tabWgt->currentIndex(), name);
    tabWgt->tabBar()->setTabToolTip(tabWgt->currentIndex(), path);
    tabWgt->setTabWhatsThis(tabWgt->currentIndex(), "Without changes");
    saveFile();
}

void QtNotepad::saveAllFiles()
{
    int index = tabWgt->currentIndex();
    for (int i = 0; i < tabWgt->count(); ++i)
    {
        tabWgt->setCurrentIndex(i);
        if (tabWgt->tabWhatsThis(tabWgt->currentIndex()) != "Without changes") saveFile();
    }
    tabWgt->setCurrentIndex(index);
}

void QtNotepad::closeFile()
{
    closeFile(tabWgt->currentIndex());
}

void QtNotepad::closeFile(int index)
{
    if (tabWgt->tabWhatsThis(tabWgt->currentIndex()) != "Without changes")
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Warning"), tr("Save before closing?"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) saveFile();
    }
    filepaths.remove(index);
    filenames.removeAt(index);
    delete tabWgt->widget(index);
    deleteTab(index);
}

void QtNotepad::closeAllFiles()
{
    bool flag = false;
    for (int i = 0; i < tabWgt->count(); ++i)
    {
        if (tabWgt->tabWhatsThis(i) != "Without changes")
        {
            flag = true;
            break;
        }
    }
    if (flag)
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, tr("Warning"), tr("Save the changes?"),
            QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) saveAllFiles();
    }
    filepaths.clear();
    filenames.clear();
    fileIndex = 1;
    while (tabWgt->count() > 0) delete tabWgt->widget(0);
    currFiles->clear();
}

void QtNotepad::closeWindow()
{
    QStringList names;
    QVector<QDir> paths;

    if (menu) delete menu;
    menu = new Menu(this);
    menu->setWindowTitle(tr("Warning"));

    for (int i = 0; i < tabWgt->count(); i++)
    {
        if (tabWgt->tabText(i).contains('*'))
        {
            filenames.push_back(names[i]);
            filepaths.push_back(paths[i]);
        }
    }

    if (paths.isEmpty()) QApplication::quit();
    else
    {
        menu->fill(paths, names);
        int event = menu->exec();
        if (event == QDialog::Accepted) saveAllFiles();
        if (event != -1) QApplication::quit();
    }
}

void QtNotepad::deleteTab(int index)
{
    delete currFiles->takeItem(index);
}

void QtNotepad::changeTabIndex(int from, int to)
{
    currFiles->insertItem(to, currFiles->takeItem(from));
    statusBarChange();
}

void QtNotepad::changeCurrIndex(QListWidgetItem* item)
{
    tabWgt->setCurrentIndex(item->listWidget()->row(item));
    statusBarChange();
}

void QtNotepad::changeCurrIndex(int index)
{
    currFiles->setCurrentRow(index);
    statusBarChange();
}

void QtNotepad::changeIndexOnDelete()
{
    currFiles->setCurrentRow(currFiles->count() - 1);
    statusBarChange();
}

void::QtNotepad::changeParameter()
{
    if (!(tabWgt->tabText(tabWgt->currentIndex())).contains("*"))
    {
        tabWgt->setTabWhatsThis(tabWgt->currentIndex(), "Changes");
        tabWgt->setTabText(tabWgt->currentIndex(), tabWgt->tabText(tabWgt->currentIndex()) + "*");
    }
}

void QtNotepad::changeContainers(int from, int to)
{
    QString name = filenames.at(from);
    filenames.replace(from, filenames.at(to));
    filenames.replace(to, name);

    QDir path = filepaths.at(from);
    filepaths.replace(from, filepaths.at(to));
    filepaths.replace(to, path);
}

void QtNotepad::copy()
{
    Editor* editor = qobject_cast<Editor*>(tabWgt->currentWidget());
    editor->copy();
}

void QtNotepad::cut()
{
    if (Editor* editor = qobject_cast<Editor*>(tabWgt->currentWidget()))
    {
        editor->cut();
    }
    else
    {
        qDebug() << "Error in casting";
    }
}

void QtNotepad::selectAll()
{
    if (Editor* editor = qobject_cast<Editor*>(tabWgt->currentWidget()))
    {
        editor->copy();
    }
    else
    {
        qDebug() << "Error in casting";
    }
}

void QtNotepad::paste()
{
    if (Editor* editor = qobject_cast<Editor*>(tabWgt->currentWidget()))
    {
        editor->paste();
    }
    else
    {
        qDebug() << "Error in casting";
    }
}

void QtNotepad::clear()
{
    if (Editor* editor = qobject_cast<Editor*>(tabWgt->currentWidget()))
    {
        editor->clear();
    }
    else
    {
        qDebug() << "Error in casting";
    }
}

SaveDialog* QtNotepad::createDialog()
{
    SaveDialog* dialog = new SaveDialog(this);
    connect(dialog->btnSave, SIGNAL(clicked()), this, SLOT(saveAllFiles()));
    int row = 0;
    for (int i = 0; i < currFiles->count(); ++i) {
        tabWgt->setCurrentIndex(i);
        if (tabWgt->tabWhatsThis(i) != "Without changes") {
            QTableWidgetItem* file = new QTableWidgetItem(tabWgt->tabText(i));
            QTableWidgetItem* path = new QTableWidgetItem(filepaths.at(i).canonicalPath());
            dialog->table->insertRow(row);
            dialog->table->setItem(row, 0, file);
            dialog->table->setItem(row, 1, path);
            ++row;
        }
    }
    return dialog;
}

SaveDialog::SaveDialog(QWidget* parent) : QDialog(parent)
{
    setWindowTitle(tr("Save the changes?"));
    setFixedSize(500, 400);

    table = new QTableWidget;
    table->setColumnCount(2);
    table->setHorizontalHeaderLabels(QStringList() << tr("File") << tr("Path"));
    table->setFixedSize(480, 300);
    table->horizontalHeader()->setStretchLastSection(true);
    table->verticalHeader()->hide();

    label = new QLabel(tr("Files with unsaved changes:"));
    label->setFixedSize(400, 15);

    btnSave = new QPushButton(tr("Save all"));
    btnNoSave = new QPushButton(tr("Don't save"));
    btnCancel = new QPushButton(tr("Cancel"));
    btnSave->setFixedSize(155, 30);
    btnNoSave->setFixedSize(155, 30);
    btnCancel->setFixedSize(155, 30);

    QGridLayout* layout = new QGridLayout;
    layout->addWidget(label, 0, 0);
    layout->addWidget(table, 1, 0, 1, 3);
    layout->addWidget(btnSave, 2, 0);
    layout->addWidget(btnNoSave, 2, 1);
    layout->addWidget(btnCancel, 2, 2);
    setLayout(layout);

    connect(btnSave, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnNoSave, SIGNAL(clicked()), this, SLOT(accept()));
    connect(btnCancel, SIGNAL(clicked()), this, SLOT(reject()));
}

void QtNotepad::statusBarChange() {
    Editor* curr = qobject_cast<Editor*>(tabWgt->currentWidget());
    if (curr) label->setText(QString("String: %1 Column: %2 ")
        .arg((curr->cursorRect().bottom() - 18) / 16 + 1)
        .arg((curr->cursorRect().left() - 3) / 4 + 1));
}

void QtNotepad::saveSettings() {
    QSettings settings("Company", "QtNotepad");
    QStringList openedTabs;
    for (int i = 0; i < tabWgt->count(); ++i) {
        if (tabWgt->tabToolTip(i) != "")
            openedTabs << tabWgt->tabToolTip(i);
    }

    settings.setValue("OpenedTabs", openedTabs);
}

void QtNotepad::loadSettings() {
    QSettings settings("Company", "QtNotepad");
    QStringList openedTabs = settings.value("OpenedTabs").toStringList();
    for (const QString& filePath : openedTabs) {
        openFile(filePath);
        tabWgt->setCurrentIndex(0);
    }

}
