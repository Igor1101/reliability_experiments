#include <QCoreApplication>
#include <QtGlobal>
#include <QtCore>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>
#include "defs.h"

constexpr int amount_of_ints = 10;
double gamma;
QString fname;
double avg2(QList<int> const& v);

class Task : public QObject
{
    Q_OBJECT
public:
    Task(QObject *parent = 0) : QObject(parent) {}
    QList<int> lwords;
    double average;
public slots:
    void run();

signals:
    void finished();
};

#include "main.moc"
int main(int argc, char *argv[])
{
#ifdef __linux__

#elif _WIN32
    // force output utf8
    system("chcp 65001");
#else

#endif
    if( argc != 3) {
        QINFO << UKR("некоректний аргумент");
        return -1;
    }
    if(argc >=2) {
        fname = std::stod(argv[1]);
    }
    if(argc >= 3) {
        gamma = std::stod(argv[2]);
    }
    // make QT ignore args
    argc = 1;
    QCoreApplication a(argc, argv);
    // Task parented to the application so that it
    // will be deleted by the application.
    Task *task = new Task(&a);

    // This will cause the application to exit when
    // the task signals finished.
    QObject::connect(task, SIGNAL(finished()), &a, SLOT(quit()));

    // This will run the task from the application event loop.
    QTimer::singleShot(0, task, SLOT(run()));

    return a.exec();
}

void Task::run()
{
    QINFO << UKR("запущено");
    // get sampling to failure
    QFile file(fname);
    if (!file.open(QIODevice::ReadOnly)) {
        QDEB << UKR("не можу відкрити файл");
        QDEB << file.errorString();
        emit finished();
        return;
    }

    while (!file.atEnd()) {
        QByteArray line = file.readLine();
        QString l = line.split(',').first();
        // if its just not '\r\r' newline
        if(l.size() > 2) {
            lwords.append(l.toInt());
        }
    }
    for(int i=0 ; i < lwords.length() ; i++)
        QDEB << lwords.at(i);

    // average operating time to failure
    average = avg2(lwords);
    QINFO << UKR("середнє на відмову:") << average;
    // sort sampling:
    std::sort(lwords.begin(), lwords.end());

    for(int i=0 ; i < lwords.length() ; i++)
        QDEB << lwords.at(i);
    // find max one, get size of interval
    static double intr_sz = lwords.last() / amount_of_ints;
    // statistical density of distribution
    static double f[amount_of_ints];
    for(int intri = 0; intri < amount_of_ints; intri++) {
        int Ni = 0;
        double intrmin = intr_sz * (intri - 1);
        double intrmax = intr_sz * intri;
        for(int lw = 0; lw<lwords.length(); lw++) {
            if(lwords.at(lw) >= intrmin  && lwords.at(lw) < intrmax) {
                // it`s inside interval!
                Ni++;
            }
        }
        f[intri] = Ni / (lwords.length() * intr_sz);
        QDEB << UKR("інтервал") << intri << UKR("статистична щільність розподілу") << f[intri];
    };
    // probability of trouble-free operation
    static double P[amount_of_ints];
    // S = fi * h - probability of the trouble
    // P = 1 - S -- probability of trouble-free
    for(int i=0; i < amount_of_ints; i++) {
        P[i] = 1 - f[i] * intr_sz;
        QDEB << UKR("інтервал") << i << UKR("Ймовірність безвідмовної роботи") << P[i];
    }
    // calc operating time to failure
    double d01 = (P[0] - gamma) / (P[0] - P[1]);
    double Ty = 0 + intr_sz * d01;
    QDEB << UKR("наробіток на відмову:") << Ty;
    emit finished();
}

// average without overflow!
double avg2(QList<int> const& v)
{
    int n = 0;
    double mean = 0.0;
    for (auto x : v) {
        double delta = x - mean;
        mean += delta/++n;
    }
    return mean;
}
