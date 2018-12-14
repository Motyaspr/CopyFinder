#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "counter.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QtWidgets>
#include <QLocale>


main_window::main_window(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    setGeometry(QStyle::alignedRect(Qt::LeftToRight, Qt::AlignCenter, size(), qApp->desktop()->availableGeometry()));

    ui->treeWidget->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    ui->treeWidget->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    ui->treeWidget->setSelectionMode(QAbstractItemView::ExtendedSelection);

    QCommonStyle style;
    setWindowTitle(QString("DupSearch"));
    ui->actionScan_Directory->setIcon(style.standardIcon(QCommonStyle::SP_MediaPlay));
    ui->actionExit->setIcon(style.standardIcon(QCommonStyle::SP_DialogCloseButton));
    ui->actionAbout->setIcon(style.standardIcon(QCommonStyle::SP_DialogHelpButton));

    connect(ui->actionScan_Directory, &QAction::triggered, this, &main_window::select_directory);
    connect(ui->actionExit, &QAction::triggered, this, &QWidget::close);
    connect(ui->actionAbout, &QAction::triggered, this, &main_window::show_about_dialog);
    connect(ui->actionStop, SIGNAL(released()), SLOT(stop_search()));
    connect(ui->actionDelete, SIGNAL(released()), SLOT(delete_items()));
    ui->actionDelete->setHidden(true);
    ui->actionStop->setHidden(true);
 //   scan_directory(QDir::homePath());
}

main_window::~main_window()
{
    searchThread.exit();
    searchThread.wait();
}

void main_window::select_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.size() == 0) {
        return;
    }
    ui->treeWidget->clear();
    setWindowTitle(QString("Duplicates in directory - %1").arg(dir));
    counter = new Counter(dir);
    counter->moveToThread(&searchThread);
    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    connect(&searchThread, &QThread::finished, counter, &QObject::deleteLater);
    connect(this, &main_window::find_duplicates, counter, &Counter::doSearch);
    connect(counter,
                         SIGNAL(send_duplicates(QVector<QString> const &)),
                         this,
                         SLOT(show_duplicates(QVector<QString> const &)));
    connect(counter,
                     SIGNAL(send_status(QString const &)),
                     this,
                     SLOT(show_status(QString const &)));
    connect(counter, &Counter::finish, this, &main_window::show_result);
    ui->actionScan_Directory->setDisabled(true);
    ui->actionStop->setHidden(false);
    searchThread.start();
    find_duplicates(dir);
}



void main_window::show_duplicates(QVector<QString> const &duplicates)
{
    QTreeWidgetItem* item = new QTreeWidgetItem(ui->treeWidget);
    item->setText(0, QString("There is " + QString::number(duplicates.size())) + " files");
    QFileInfo file_info(duplicates[0]);
    item->setText(1, QString::number(file_info.size()) + QString(" bytes"));
    for (QString file : duplicates){
        QTreeWidgetItem* childItem = new QTreeWidgetItem();
        childItem->setText(0, file);
        item->addChild(childItem);
    }
    ui->treeWidget->addTopLevelItem(item);
}


void main_window::delete_items() {
    QList<QTreeWidgetItem *> sel_items = ui->treeWidget->selectedItems();
    QMessageBox::StandardButton dialog = QMessageBox::question(this, "Deleting",
                                                                "Do you want to delete selected files?");
    if (dialog == QMessageBox::Yes){
        for (auto item : sel_items) {
            QString path = item->text(0);
            QFile file(path);
            if (file.remove()) {
                if (item->parent()->childCount() == 1){
                    delete item->parent();
                    //item->parent()->removeChild(item);
                } else {
                    item->parent()->setText(0, QString("There is " + QString::number(item->parent()->childCount() - 1) + " files"));
                    item->parent()->removeChild(item);
                }
            }

        }
    }

}

void main_window::show_result()
{
    ui->actionDelete->setHidden(false);
    ui->actionStop->setHidden(true);
    searchThread.quit();
    searchThread.wait();
}

void main_window::stop_search()
{
    searchThread.requestInterruption();
}

void main_window::show_about_dialog() {
    QMessageBox::aboutQt(this);
}




void main_window::show_status(QString const &txt) {
    ui->treeWidget->clear();
    QTreeWidgetItem *status_item = new QTreeWidgetItem(ui->treeWidget);
    status_item->setText(0, txt);
}


