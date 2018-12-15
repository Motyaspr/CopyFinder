#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "counter.h"

#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QtWidgets>
#include <QLocale>
#include <QTime>
#include <QColor>

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
    setWindowTitle(QString("CopyFinder"));
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
    ui->progressBar->setHidden(true);
 //   scan_directory(QDir::homePath());
}

main_window::~main_window()
{
    Thread.exit();
    Thread.wait();
}

void main_window::select_directory()
{
    QString dir = QFileDialog::getExistingDirectory(this, "Select Directory for Scanning",
                                                    QString(), QFileDialog::ShowDirsOnly | QFileDialog::DontResolveSymlinks);

    if (dir.size() == 0) {
        return;
    }
    my_dir = dir;
    t.start();
    ui->treeWidget->clear();
    setWindowTitle(QString("Duplicates in directory - %1").arg(dir));
    counter = new Counter(dir);
    counter->moveToThread(&Thread);
    ui->progressBar->setRange(0, 100);
    qRegisterMetaType<QVector<QString>>("QVector<QString>");
    connect(&Thread, &QThread::finished, counter, &QObject::deleteLater);
    connect(this, &main_window::find_duplicates, counter, &Counter::doSearch);
    connect(counter, SIGNAL(send_progress(qint16)), this, SLOT(show_progress(qint16)));
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
    ui->actionDelete->setHidden(true);
    ui->progressBar->setHidden(false);
    ui->progressBar->setValue(0);
    ui->progressBar->setTextVisible(true);
    Thread.start();
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
        QString rel_path = "";
        for (int i = my_dir.size() + 1; i < file.size(); i++)
            rel_path += file[i];
        childItem->setText(0, rel_path);
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
            QString path = my_dir + '/' + item->text(0);
            QFile file(path);
            if (file.remove()) {
                if (item->parent()->childCount() == 2){
                    delete item->parent();
                    //item->parent()->removeChild(item);
                } else {
                    item->parent()->setText(0, QString("There are " + QString::number(item->parent()->childCount() - 1) + " files"));
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
    ui->actionScan_Directory->setDisabled(false);
    show_status("Finished in " + QString::number(t.elapsed()) + " ms");
    Thread.quit();
    Thread.wait();
}

void main_window::stop_search()
{
    Thread.requestInterruption();
}

void main_window::show_progress(qint16 x)
{
    ui->progressBar->setFormat(QString::number(x)+"%");
    ui->progressBar->setValue(x);
}

void main_window::show_about_dialog() {
    QMessageBox::aboutQt(this);
}




void main_window::show_status(QString const &txt) {
    if (ui->treeWidget->topLevelItemCount() == 0){
        ui->actionDelete->setHidden(true);
        QTreeWidgetItem *status_item = new QTreeWidgetItem(ui->treeWidget);
        status_item->setText(0, txt);
        ui->treeWidget->addTopLevelItem(status_item);
    } else{
        ui->treeWidget->topLevelItem(0)->setText(0, txt);
    }
}
