#include "mainwindow.h"
#include <QApplication>
#include <QDateTime>

static QFile* logFile;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context); Q_UNUSED(type);

    QTextStream out(logFile);

    // Write the message time and the message
    out << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ");

    out << msg << endl;

    out.flush();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Set the log file path
#ifdef Q_OS_WIN
    logFile = new QFile("log.txt");
#else
    if(!QDir(".config/chartsfinder2").exists()) QDir(".config/chartsfinder2").mkpath(".");
    logFile = new QFile(".config/chartsfinder2/log.txt");
#endif

    // Open the log file
    logFile->open(QFile::WriteOnly | QFile::Text);

    // Install the message handler
    qInstallMessageHandler(messageHandler);

    qDebug() << "Charts Finder 2.1, September 2019";

    qDebug() << "Current date:" << QDateTime::currentDateTime().toString("yyyy-MM-dd");

    // Init cURL
    curl_global_init(CURL_GLOBAL_ALL);

    MainWindow mainWin;

    return app.exec();
}
