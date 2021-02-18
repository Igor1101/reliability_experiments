#ifndef DEFS_H
#define DEFS_H

#include <QDebug>
#define UKR(STR)	QStringLiteral(STR)
#define QINFO 	qInfo().noquote()
#define QWARN 	qWarning().noquote()
#define QDEB	qDebug().noquote()
#define QERR	qError().noquote()
#define QCRI	qCritical().noquote()

#endif // DEFS_H
