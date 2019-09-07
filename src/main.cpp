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

    // This should be %appdata%/ChartsFinder2 on Windows, and ~/.local/share/ChartsFinder2 on Linux
    QString dataPath = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);

    // Make it (this will fail if it's already created)
    QDir(dataPath).mkpath(".");

    // Set the log file path
    logFile = new QFile(dataPath + "/log.txt");

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
