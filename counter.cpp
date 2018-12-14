#include <counter.h>

#include <qdiriterator.h>

const long long MAXN = 5;

void Counter::get_duplicates()
{

    QString dir = this->root;
    emit send_status("RUN");
    QMap<qint64, QVector<QString>> groups;
    QDirIterator it(dir, QDir::Files, QDirIterator::Subdirectories);
    while (it.hasNext()) {
        QString path = it.next();
        QFileInfo info(path);
        qint64 size = info.size();
        groups[size].push_back(path);
        if (QThread::currentThread()->isInterruptionRequested()){
            emit finish();
            return;
        }
    }

    QVector<QVector<QString>> ans;
    for (auto it = groups.begin(); it != groups.end(); ++it){
        if (it.value().size() == 1)
            continue;
        QMap<QString, QVector<QString>> small_groups;
        qint64 sz = it.key();
        for (QString file : it.value()){
            small_groups[get_first_k(file, std::min(sz, MAXN))].push_back(file);
        }
        if (QThread::currentThread()->isInterruptionRequested()){
            emit finish();
            return;
        }
        for (auto it1 = small_groups.begin(); it1 != small_groups.end(); ++it1){
            if (it1.value().size() == 1)
                continue;
            if (QThread::currentThread()->isInterruptionRequested()){
                emit finish();
                return;
            }
            QMap<QByteArray, QVector<QString>> final_groups;
            for (QString file : it.value()){
                final_groups[get_hash(file)].push_back(file);
            }
            for (auto it2 = final_groups.begin(); it2 != final_groups.end(); ++it2){
                if (it2.value().size() == 1)
                    continue;
                if (QThread::currentThread()->isInterruptionRequested()){
                    emit finish();
                    return;
                }
                curans.clear();
                for (int i = 0; i < it2.value().size(); i++)
                    curans.push_back(it2.value()[i]);
                emit send_duplicates(curans);
            }
        }


    }
    //emit send_status("Finished" + QString::number(t.elapsed()) + "ms");
    emit finish();

}

Counter::Counter(const QString &root)
{
    this->root = root;
}

void Counter::doSearch(const QString &dir)
{
    get_duplicates();
}

QByteArray Counter::get_hash(const QString &filepath)
{
    QCryptographicHash sha(QCryptographicHash::Sha256);
    QFile file(filepath);
    if (file.open(QIODevice::ReadOnly)) {
        sha.addData(file.readAll());
    }
    return sha.result();
}

QString Counter::get_first_k(QString const& filepath, qint64 x){
    QFile file(filepath);
    if (file.open(QIODevice::ReadOnly)) {
        QString s = file.read(x);

    return s;
    }
}


