#include <counter.h>

#include <qdiriterator.h>

const long long MAXN = 5;

void Counter::get_duplicates() {

    QString dir = this->root;
    emit send_status("RUN");
    QMap <qint64, QVector<QString>> groups;
    QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        total++;
        QString path = it.next();
        QFileInfo info(path);
        qint64 size = info.size();
        groups[size].push_back(path);
        if (QThread::currentThread()->isInterruptionRequested()) {
            emit send_progress(get_percent());
            emit finish();
            return;
        }

    }
    emit send_progress(1);

    QVector <QVector<QString>> ans;
    for (auto it = groups.begin(); it != groups.end(); ++it) {
        if (it.value().size() == 1){
            current++;
            continue;
        }
        QMap <QString, QVector<QString>> small_groups;
        qint64 sz = it.key();
        for (QString file : it.value()) {
            try{
                small_groups[get_first_k(file, std::min(sz, MAXN))].push_back(file);
            } catch(QString&){
                current++;
            }
        }
        if (QThread::currentThread()->isInterruptionRequested()) {
            emit send_progress(get_percent());
            emit finish();
            return;
        }
        for (auto it1 = small_groups.begin(); it1 != small_groups.end(); ++it1) {
            if (it1.value().size() == 1){
                current++;
                continue;
            }
            if (QThread::currentThread()->isInterruptionRequested()) {
                emit send_progress(get_percent());
                emit finish();
                return;
            }
            QMap <QByteArray, QVector<QString>> final_groups;
            for (QString file : it.value()) {
                try{
                    final_groups[get_hash(file)].push_back(file);
                } catch(QString&){
                    current++;
                }
            }
            for (auto it2 = final_groups.begin(); it2 != final_groups.end(); ++it2) {
                if (it2.value().size() == 1){
                    current++;
                    continue;
                }
                if (QThread::currentThread()->isInterruptionRequested()) {
                    emit send_progress(get_percent());
                    emit finish();
                    return;
                }
                current += it2.value().size();
                emit send_duplicates(it2.value());
                emit send_progress(get_percent());
            }
        }


    }
    emit send_progress(100);
    //emit send_status("Finished" + QString::number(t.elapsed()) + "ms");
    emit finish();

}

qint16 Counter::get_percent()
{
    double x = static_cast<double>(((1.0) * current) / total);
    return static_cast<qint16>(100 * x);
}

Counter::Counter(const QString &root) {
    this->root = root;
}

void Counter::doSearch(const QString &dir) {
    get_duplicates();
}

QByteArray Counter::get_hash(const QString &filepath) {
    QCryptographicHash sha(QCryptographicHash::Sha256);
    QFile file(filepath);

    if (file.open(QIODevice::ReadOnly)) {
        sha.addData(file.readAll());
        return sha.result();
    }
    throw QString("File is not open");
}

QString Counter::get_first_k(QString const &filepath, qint64 x) {
    QFile file(filepath);
    if (file.open(QIODevice::ReadOnly)) {
        QString s = file.read(x);
        return s;
    }
    throw QString("File is not open");
}


