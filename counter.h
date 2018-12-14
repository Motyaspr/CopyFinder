#ifndef COUNTER_H
#define COUNTER_H

#include <QByteArray>
#include <QVector>
#include <QString>
#include <vector>
#include <QMap>
#include <math.h>
#include <QThread>
#include <QCryptographicHash>
#include <QCommonStyle>
#include <QDesktopWidget>
#include <QDir>
#include <QFileDialog>
#include <QFileInfo>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QDirIterator>
#include <unordered_map>
#include <QDir>
#include <fstream>
#include <iostream>
#include <QTime>



class Counter :  public QObject
{
Q_OBJECT

public:
    Counter(QString const& root);

public slots:
    void doSearch(QString const& dir);
signals:
    void send_duplicates(QVector<QString> const &);
    void send_status(QString const &);
    void finish();
private:
    QByteArray get_hash(QString const& filepath);
    QString get_first_k(QString const& filepath, qint64 k);
    QString root;
    void get_duplicates();
    QVector<QString> curans;
    QTime t;
};

#endif // COUNTER_H
