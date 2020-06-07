#include "configeditor.h"
#include "mainwindow.h"

#include <QApplication>
#include <QDateTime>
#include <QStandardPaths>

static QFile *logFile;

void messageHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    Q_UNUSED(context) Q_UNUSED(type)

    QTextStream out(logFile);

    // Write the message time and the message
    out << QDateTime::currentDateTime().toString("hh:mm:ss.zzz ");

    out << msg << endl;

    out.flush();
}

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    app.setApplicationName("ChartsFinder2");
    app.setApplicationDisplayName("Charts Finder");

    qRegisterMetaTypeStreamOperators<ConfigEditor::Resource>("Resource");
    qRegisterMetaTypeStreamOperators<QList<ConfigEditor::Resource>>("Resources");

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

    qDebug() << "Charts Finder 2.3, 7 June 2020";
    qDebug() << "Current date:" << QDateTime::currentDateTime().toString("yyyy-MM-dd");

    // Init cURL
    curl_global_init(CURL_GLOBAL_ALL);

    ConfigEditor configEditor(dataPath);
    configEditor.readConfig();

    QLocale locale;
    QTranslator qtTranslator, translator, updaterTranslator;

    // No need to translate if the language is English, or the system language is English
    if (configEditor.config.language != "English" || (configEditor.config.language == "System"
                                                      && QLocale::system().language() != QLocale::English)) {
        qDebug() << "Setting translators";

        // Set the system locale
        if (configEditor.config.language == "System") {
            qDebug() << "Locale was set to system:" << locale.name();
        } else {
            locale = QLocale(configEditor.config.language);
            qDebug() << "Locale was set to" << configEditor.config.language;
        }

        QLocale::setDefault(locale);

        // Load the translations
        if (qtTranslator.load("qt_" + locale.name(), app.applicationDirPath() + "/translations")) {
            qDebug() << "Qt translation was loaded";
        } else {
            qDebug() << "Couldn't load Qt translation";
        }

        if (translator.load("chartsfinder2_" + locale.name(), app.applicationDirPath() + "/translations")) {
            qDebug() << "Charts Finder translation was loaded";
        } else {
            qDebug() << "Couldn't load Charts Finder translation";
        }

        if (updaterTranslator.load("qtautoupdater_" + locale.name(), app.applicationDirPath() + "/translations")) {
            qDebug() << "Updater translation was loaded";
        } else {
            qDebug() << "Couldn't load updater translation";
        }

        // Install the translators
        app.installTranslator(&qtTranslator);
        app.installTranslator(&translator);
        app.installTranslator(&updaterTranslator);

        qDebug() << "Translators were set";
    }

    MainWindow mainWin(&configEditor.config);

    int result = app.exec();

    configEditor.writeConfig();

    logFile->close();
    delete logFile;

    if (result == 1111)
        QProcess::startDetached(QApplication::applicationFilePath());

    return result;
}
