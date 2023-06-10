#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    highlighter = new SyntaxHighlighter;
    setAcceptDrops(true);
    setWindowIcon(QIcon(":/icons/icon.png"));
    setWindowTitle("Text Editor");
    setWindowState(Qt::WindowState::WindowMaximized);

    setUpTabWidget();
    setUpFileExplorerDock();
    setUpOpenedDocsDock();
    createFile();
    setUpMenuBar();
    setUpToolBar();

    setCentralWidget(tabs);
    tabs->currentWidget()->setFocus();
}

MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::setUpTabWidget()
{
    tabs = new QTabWidget(this);
    tabs->setMovable(true);
    tabs->setTabsClosable(true);
    tabs->setUsesScrollButtons(true);
    connect(tabs, SIGNAL(tabCloseRequested(int)), this, SLOT(closeFile(int)));

    QToolButton *new_tab_button = new QToolButton;
    new_tab_button->setText("+");
    connect(new_tab_button, SIGNAL(clicked()), this, SLOT(createFile()));
    tabs->setCornerWidget(new_tab_button, Qt::TopRightCorner);
}

void MainWindow::setUpMenuBar()
{
    QMenu* fileMenu = new QMenu("File");
    QMenu* editMenu = new QMenu("Edit");
    QMenu* viewMenu = new QMenu("View");

    fileMenu->addAction("New File",   this, SLOT(createFile()),    Qt::CTRL + Qt::Key_N);
    fileMenu->addAction("Open File",  this, SLOT(openFile()),      Qt::CTRL + Qt::Key_O);
    fileMenu->addSeparator();
    fileMenu->addAction("Save File",  this, SLOT(saveFile()),      Qt::CTRL + Qt::Key_S);
    fileMenu->addAction("Save As...", this, SLOT(saveFileAs()),    Qt::SHIFT + Qt::CTRL + Qt::Key_S);
    fileMenu->addAction("Save All",   this, SLOT(saveAllFiles()));
    fileMenu->addSeparator();
    fileMenu->addAction("Close File", this, SLOT(closeFile()),     Qt::CTRL + Qt::Key_W);
    fileMenu->addAction("Close All",  this, SLOT(closeAllFiles()), Qt::SHIFT + Qt::CTRL + Qt::Key_W);
    fileMenu->addSeparator();
    fileMenu->addAction("Exit",       this, SLOT(closeWindow()),   Qt::CTRL + Qt::Key_Q);

    editMenu->addAction("Cut",        this, SLOT(slotCut()),       Qt::CTRL + Qt::Key_X);
    editMenu->addAction("Copy",       this, SLOT(slotCopy()),      Qt::CTRL + Qt::Key_C);
    editMenu->addAction("Paste",      this, SLOT(slotPaste()),     Qt::CTRL + Qt::Key_V);
    editMenu->addAction("Delete",     this, SLOT(slotClear()),     Qt::CTRL + Qt::Key_Backspace);
    editMenu->addAction("Select All", this, SLOT(slotSelectAll()), Qt::CTRL + Qt::Key_A);

    viewMenu->addAction(file_explorer_dock->toggleViewAction());
    viewMenu->addAction(opened_docs_dock->toggleViewAction());

    ui->menubar->addMenu(fileMenu);
    ui->menubar->addMenu(editMenu);
    ui->menubar->addMenu(viewMenu);
}

void MainWindow::setUpToolBar()
{
    ui->maintoolbar->addAction(QIcon(":/icons/new_file.png"),       "Create New File", this, SLOT(createFile()));
    ui->maintoolbar->addAction(QIcon(":/icons/open_file.png"),      "Open File",       this, SLOT(openFile()));
    ui->maintoolbar->addAction(QIcon(":/icons/save_file.png"),      "Save File",       this, SLOT(saveFile()));
    ui->maintoolbar->addAction(QIcon(":/icons/save_all_files.png"), "Save All Files",  this, SLOT(saveAllFiles()));
}

void MainWindow::setUpFileExplorerDock()
{
    file_system_model = new QDirModel;
    file_system_view  = new QTreeView;
    file_system_view->setModel(file_system_model);
    file_system_view->setRootIndex(file_system_model->index("/home"));
    file_system_view->setColumnHidden(1, true);
    file_system_view->setColumnHidden(2, true);
    file_system_view->setColumnHidden(3, true);
    file_system_view->setHeaderHidden(true);
    connect(file_system_view, SIGNAL(doubleClicked(QModelIndex)), this, SLOT(openFile(QModelIndex)));

    file_explorer_dock = new QDockWidget("File explorer", this);

    file_explorer_dock->setWidget(file_system_view);
    file_explorer_dock->setFeatures(QDockWidget::DockWidgetClosable);
    file_explorer_dock->hide();

    addDockWidget(Qt::LeftDockWidgetArea, file_explorer_dock);
}

void MainWindow::setUpOpenedDocsDock()
{
    opened_docs_widget = new QListWidget;

    connect(tabs->tabBar(),      SIGNAL(tabMoved(int, int)),            this, SLOT(changeTabIndexInList(int, int)));
    connect(opened_docs_widget, SIGNAL(itemClicked(QListWidgetItem*)), this, SLOT(updateCurrentIndex(QListWidgetItem*)));
    connect(opened_docs_widget, SIGNAL(currentRowChanged(int)),        tabs, SLOT(setCurrentIndex(int)));
    connect(tabs->tabBar(),      SIGNAL(currentChanged(int)),           this, SLOT(updateCurrentIndex(int)));
    connect(tabs->tabBar(),      SIGNAL(tabCloseRequested(int)),        this, SLOT(updateCurrentIndexOnDelete(int)));

    opened_docs_dock  = new QDockWidget("Opened files", this);

    opened_docs_dock->setWidget(opened_docs_widget);
    opened_docs_dock->setFeatures(QDockWidget::DockWidgetClosable);
    opened_docs_dock->hide();
    addDockWidget(Qt::RightDockWidgetArea, opened_docs_dock);
}

void MainWindow::createFile()
{
    Editor *new_text_edit = new Editor;
    int index = tabs->addTab(new_text_edit, "untitled");
    tabs->setCurrentIndex(index);
    tabs->setTabToolTip(index, "");
    tabs->setTabWhatsThis(index, "No changes");
    connect(new_text_edit, SIGNAL(textChanged()), this, SLOT(updateParameter()));

    QListWidgetItem* new_item = new QListWidgetItem;
    new_item->setText(tabs->tabText(index));
    new_item->setToolTip(tabs->tabToolTip(index));
    opened_docs_widget->addItem(new_item);

    updateCurrentIndex(index);
}

void MainWindow::openFile()
{
    QString filepath = QFileDialog::getOpenFileName(this, "Choose file", "/home");
    if (filepath.isEmpty())
        return;
    openFile(filepath);
}

void MainWindow::openFile(const QString& filepath)
{
    for (int i = 0; i < tabs->count(); ++i)
        if (tabs->tabToolTip(i) == filepath)
        {
            tabs->setCurrentIndex(i);
            return;
        }

    QString filename = filepath.section("/",-1,-1);
    QFile file(filepath);

    if (file.open(QIODevice::ReadOnly))
    {
        Editor* temp_text = qobject_cast<Editor *>(tabs->currentWidget());
        if (temp_text->document()->isEmpty() &&
            tabs->tabToolTip(tabs->currentIndex()) == "" &&
            tabs->tabText(tabs->currentIndex()) == "untitled") {
            deleteTabFromList(tabs->currentIndex());
            delete tabs->widget(tabs->currentIndex());
        }

        Editor *new_text_edit = new Editor;
        QString content = file.readAll();
        file.close();
        new_text_edit->appendPlainText(content);
        int index = tabs->addTab(new_text_edit, filename);
        tabs->setCurrentIndex(index);
        tabs->setTabToolTip(index, filepath);
        tabs->setTabWhatsThis(index, "No changes");
        connect(new_text_edit, SIGNAL(textChanged()), this, SLOT(updateParameter()));

        QListWidgetItem* new_item = new QListWidgetItem;
        new_item->setText(tabs->tabText(index));
        new_item->setToolTip(tabs->tabToolTip(index));
        opened_docs_widget->addItem(new_item);

        QString file_extension = QFileInfo(filename).suffix();

        if (file_extension == "cpp" || file_extension == "h" || file_extension == "hpp" || file_extension == "py" || file_extension == "scala" )
            highlighter->setDocument(qobject_cast<Editor *>(tabs->currentWidget())->document());

        tabs->setTabWhatsThis(index, "No changes");
        updateCurrentIndex(index);
        new_text_edit->verticalScrollBar()->setValue(0);
    }
    else
    {
        (new QErrorMessage(this))->showMessage("Cannot open file!");
        return;
    }
}

void MainWindow::saveFile() {
    if (tabs->tabToolTip(tabs->currentIndex()) == "") {
        saveFileAs();
        return;
    }
    QString filepath = tabs->tabToolTip(tabs->currentIndex());
    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly))
    {
        file.write(qobject_cast<Editor *>(tabs->currentWidget())->document()->toPlainText().toUtf8()); // unsafe getting!
        file.close();
        tabs->setTabWhatsThis(tabs->currentIndex(), "No changes");
    }
    else
    {
        (new QErrorMessage(this))->showMessage("Cannot save file!");
        return;
    }
}

void MainWindow::saveFileAs() {
    QString filename = tabs->tabText(tabs->currentIndex());
    QString filepath = QFileDialog::getSaveFileName(this, "Save " + filename, "/home/" + filename);
    if (filepath.isEmpty())
        return;
    if (QFileInfo(filepath).suffix().isEmpty())
        filepath.append(".txt");

    QFile file(filepath);
    if (file.open(QIODevice::WriteOnly)) {
        file.write(qobject_cast<Editor *>(tabs->currentWidget())->document()->toPlainText().toUtf8());
        file.close();
    }
    else
    {
        (new QErrorMessage(this))->showMessage("Cannot save file!");
        return;
    }

    filename = filepath.section("/",-1,-1);
    tabs->tabBar()->setTabText(tabs->currentIndex(), filename);
    tabs->tabBar()->setTabToolTip(tabs->currentIndex(), filepath);

    QString file_extension = QFileInfo(filename).suffix();

    if (file_extension == "cpp" || file_extension == "h" || file_extension == "hpp" || file_extension == "py" || file_extension == "scala")
        highlighter->setDocument(qobject_cast<Editor *>(tabs->currentWidget())->document());

    tabs->setTabWhatsThis(tabs->currentIndex(), "No changes");
}

void MainWindow::saveAllFiles()
{
    int current_index = tabs->currentIndex();
    for (int i = 0; i < tabs->count(); ++i)
    {
        tabs->setCurrentIndex(i);
        if (tabs->tabWhatsThis(tabs->currentIndex()) != "No changes")
            saveFile();
    }
    tabs->setCurrentIndex(current_index);
}

void MainWindow::closeFile(int index_)
{
    if (tabs->tabWhatsThis(tabs->currentIndex()) != "No changes")
    {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Saving changes", "Save changes before closing?",
                                      QMessageBox::Yes|QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            saveFile();
        }
    }
    delete tabs->widget(index_);

    deleteTabFromList(index_);

    if (!tabs->count())
        createFile();
    tabs->currentWidget()->setFocus();
}

void MainWindow::closeFile() {
    closeFile(tabs->currentIndex());
}

void MainWindow::closeAllFiles() {
    bool fl = false;
    for (int i = 0; i < tabs->count(); ++i)
    {
        if (tabs->tabWhatsThis(i) != "No changes")
        {
            fl = true;
            break;
        }
    }
    if (fl)
    {
        QDialog *dialog = new QDialog(this);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Save | QDialogButtonBox::Cancel);

        connect(buttonBox, SIGNAL(accepted()), dialog, SLOT(accept()));
        connect(buttonBox, SIGNAL(rejected()), dialog, SLOT(reject()));

        QTableWidget *paths = new QTableWidget(0, 2, dialog);
        paths->setHorizontalHeaderLabels(QStringList({"Name","File path"}));

        int j = 0;
        for (int i = 0; i < tabs->count(); ++i)
        {
            if (tabs->tabWhatsThis(i) != "No changes")
            {
                QTableWidgetItem *w_item1 = new QTableWidgetItem(tabs->tabText(i)),
                                 *w_item2 = new QTableWidgetItem(tabs->tabToolTip(i));

                paths->insertRow(j);

                paths->setItem(j, 0, w_item1);
                paths->setItem(j, 1 ,w_item2);
                ++j;
            }
        }

        QLabel *text = new QLabel("Save changes in these documents?");
        QVBoxLayout *layout = new QVBoxLayout(dialog);
        layout->addWidget(text);
        layout->addWidget(paths);
        layout->addWidget(buttonBox);
        dialog->setLayout(layout);

        if(dialog->exec() == QDialog::Accepted)
            saveAllFiles();
    }
    while (tabs->count() > 0)
        delete tabs->widget(0);

    opened_docs_widget->clear();

    createFile();
    tabs->currentWidget()->setFocus();
}

void MainWindow::closeWindow() {
    closeAllFiles();
    QApplication::quit();
}

void::MainWindow::updateParameter()
{
    QString file = tabs->tabBar()->tabText(tabs->currentIndex());
    tabs->setTabWhatsThis(tabs->currentIndex(), "Changed");
    disconnect(sender(), SIGNAL(textChanged()), this, SLOT(updateParameter()));
}


void MainWindow::openFile(QModelIndex file_index)
{
    if (!file_system_model->isDir(file_index))
        openFile(file_system_model->filePath(file_index));
}

void MainWindow::deleteTabFromList(int index)
{
    QListWidgetItem* temp_item = opened_docs_widget->takeItem(index);
    delete temp_item;
}

void MainWindow::changeTabIndexInList(int old_index, int new_index)
{
    QListWidgetItem* first_item  = opened_docs_widget->takeItem(old_index);
    opened_docs_widget->insertItem(new_index, first_item);
}

void MainWindow::updateCurrentIndex(QListWidgetItem* current_item)
{
    int index = current_item->listWidget()->row(current_item);
    tabs->setCurrentIndex(index);
}

void MainWindow::updateCurrentIndex(int new_selection_index)
{
    opened_docs_widget->setCurrentRow(new_selection_index);

    QString file = tabs->tabBar()->tabText(new_selection_index);
    QString file_extension = QFileInfo(file).suffix();
    if (!file_extension.isEmpty())
    {
        if (file_extension == "cpp" || file_extension == "h" || file_extension == "hpp" || file_extension == "py" || file_extension == "scala" )
            highlighter->setDocument(qobject_cast<Editor *>(tabs->currentWidget())->document());
    }
}

void MainWindow::updateCurrentIndexOnDelete(int)
{
    opened_docs_widget->setCurrentRow(opened_docs_widget->count() - 1);
}

void MainWindow::slotCopy()
{
    qobject_cast<Editor *>(tabs->currentWidget())->copy();
}

void MainWindow::slotCut()
{
    qobject_cast<Editor *>(tabs->currentWidget())->cut();
}

void MainWindow::slotSelectAll()
{
    qobject_cast<Editor *>(tabs->currentWidget())->selectAll();
}

void MainWindow::slotPaste()
{
    qobject_cast<Editor *>(tabs->currentWidget())->paste();
}

void MainWindow::slotClear()
{
    qobject_cast<Editor *>(tabs->currentWidget())->clear();
}

void MainWindow::closeEvent(QCloseEvent *)
{
    closeAllFiles();
    QApplication::quit();
}
