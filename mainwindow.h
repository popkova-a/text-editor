#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QGridLayout>
#include <QFileDialog>
#include "Editor.h"
#include "highlighter.h"
#include <QErrorMessage>
#include <QListWidget>
#include <QMessageBox>
#include <QErrorMessage>
#include <QFile>
#include <QFileDialog>
#include <QFileInfo>
#include <QtPrintSupport/QPrinter>
#include <QDockWidget>
#include <QListWidget>
#include <QListWidgetItem>
#include <QTreeView>
#include <QDirModel>
#include <QModelIndex>
#include <QToolBar>
#include <QTabBar>
#include <QToolButton>
#include <QScrollBar>
#include <QTableWidget>
#include <QPushButton>
#include <QLabel>
#include <QDialogButtonBox>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    QTabWidget *tabs;
    QListWidget *opened_docs_widget; //т.к. не разобралась с QStringListModel
    SyntaxHighlighter *highlighter;

    Ui::MainWindow *ui;

    QDirModel *file_system_model;
    QTreeView *file_system_view;

    QDockWidget *file_explorer_dock;
    QDockWidget *opened_docs_dock;

    void setUpTabWidget();
    void setUpMenuBar();
    void setUpToolBar();
    void setUpFileExplorerDock();
    void setUpOpenedDocsDock();
    void closeEvent(QCloseEvent*) override;

private slots:

    void updateParameter();
    void createFile();
    void openFile();
    void openFile(const QString&);
    void openFile(QModelIndex);
    void saveFile();
    void saveFileAs();
    void saveAllFiles();
    void closeFile();
    void closeFile(int);
    void closeAllFiles();
    void closeWindow();


    void changeTabIndexInList(int, int);
    void deleteTabFromList(int);
    void updateCurrentIndex(int);
    void updateCurrentIndex(QListWidgetItem*);
    void updateCurrentIndexOnDelete(int);

    void slotCut();
    void slotCopy();
    void slotPaste();
    void slotSelectAll();
    void slotClear();
};
#endif // MAINWINDOW_H
