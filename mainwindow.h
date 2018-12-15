#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <memory>
#include <QThread>
#include <QTime>


#include <counter.h>

namespace Ui {
class MainWindow;
}

class main_window : public QMainWindow
{
Q_OBJECT
    QThread Thread;
public:
    explicit main_window(QWidget *parent = 0);
    ~main_window();

public slots:
    void show_duplicates(QVector<QString> const &duplicates);
    void show_status(QString const &);
    void select_directory();
    void show_about_dialog();
    void delete_items();
    void show_result();
    void stop_search();
//    void select_all();
    void show_progress(qint16);
signals:
    void find_duplicates(QString const& dir);
private:
    std::unique_ptr<Ui::MainWindow> ui;
    Counter *counter = nullptr;
    QTime t;
    QString my_dir;
};

#endif // MAINWINDOW_H
