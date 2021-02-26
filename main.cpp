#include <QCoreApplication>
#include <QtGlobal>
#include <QtCore>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>
#include "defs.h"

constexpr int amount_of_ints = 10;
double gamm;
QString fname;
double time_Pfail;
double time_Ifail;
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
    if( argc != 5) {
        QINFO << UKR("некоректний аргумент");
        return -1;
    }
    if(argc >=2) {
        fname = QString(argv[1]);
    }
    if(argc >= 3) {
        gamm = std::stod(argv[2]);
    }
    if(argc >= 4) {
        time_Pfail = std::stod(argv[3]);
    }
    if(argc >= 5) {
        time_Ifail = std::stod(argv[4]);
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
        QDEB << UKR("не можу відкрити файл ") << fname;
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
    // verify empty data
    if(lwords.length()  == 0) {
       QINFO << UKR("пустий файл");
        emit finished();
    }
    if(gamm > 1.0 || gamm < 0.0) {
        QINFO << UKR("gamma invalid");
        emit finished();
        return;
    }
    //for(int i=0 ; i < lwords.length() ; i++)
    //    QDEB << lwords.at(i);

    // average operating time to failure
    average = avg2(lwords);
    QINFO << UKR("середнє на відмову:Tcp=") << average;
    // sort sampling:
    std::sort(lwords.begin(), lwords.end());
   // for(int i=0 ; i < lwords.length() ; i++)
    //    QDEB << lwords.at(i);
    // find max one, get size of interval
    static double intr_sz = lwords.last() / amount_of_ints;
    // statistical density of distribution
    static double f[amount_of_ints] = { 0 };
    for(int intri = 1; intri <= amount_of_ints; intri++) {
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
    static double P[amount_of_ints] = { 1.0 };
    // S = fi * h - probability of the trouble
    // P =  S -- probability of trouble-free
    int gamma_p;
    double pmax = 1.0;
    for(int i=1; i <= amount_of_ints; i++) {
        pmax -= f[i] * intr_sz;
        P[i] = pmax;
        QDEB << UKR("інтервал") << i << UKR("Ймовірність безвідмовної роботи") << P[i];
        if(P[i-1] > gamm && P[i] <= gamm ) {
            gamma_p = i;
        }
    }
    // calc operating time to failure
    double d = (P[gamma_p] - gamm) / (P[gamma_p] - P[gamma_p-1]);
    QINFO << "d="<< d;
    double Ty = gamma_p * intr_sz - intr_sz * d;
    QINFO << UKR("y-відсотковий наробіток на відмову:") << Ty;
    // probability of trouble-free operation
    double Ptf = 1.0;
    int i = 1;
    while ((i * intr_sz) <= time_Pfail) {
        if(i > amount_of_ints) {
            QINFO << UKR("помилка в значенні time_Pfail(час для ймовірності безвідмовної роботи)");
            emit finished();
            return ;
        }
        Ptf -= f[i] * intr_sz;
        i++;
    }
    Ptf -= f[i] * (time_Pfail - (i - 1) * intr_sz);
    // sec time calc for another timeex 1

    double Ptf2 = 1.0;
    i = 1;
    while ((i * intr_sz) <= time_Ifail ) {
        if(i > amount_of_ints) {
            QINFO << UKR("помилка в значенні time_Ifail(час для інтенсивності відмов)");
            emit finished();
            return ;
        }
        Ptf2 -= f[i] * intr_sz;
        i++;
    }
    Ptf2 -= f[i] * (time_Ifail - (i - 1) * intr_sz);

    double lambda = f[i] / Ptf2;
    QINFO << UKR("ймовірність безвідмовної роботи:") << Ptf << UKR(" на час:") << time_Pfail;
    QINFO << UKR("інтенсивність відмов:") << lambda << UKR(" на час:") << time_Ifail;

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
