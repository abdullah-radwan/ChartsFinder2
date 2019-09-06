#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QEventLoop>
#include <QMap>
#include <QDebug>
#include <QCoreApplication>
#include <QTimer>
#include <QFileInfo>
#include <QDir>
#include "curl/curl.h"
#include "config.h"

class Downloader : public QObject
{
    Q_OBJECT

public:
    Downloader();

    void download(Config::configStruct config, QStringList airports);

    void cancel();

    ~Downloader();

    bool valid = true;

    bool running = false;

    bool canceled, showProgress;

signals:
    void exists(QString airport);

    void searchingForCharts(QString airport);

    void downloadingCharts(QString airport);

    void downloadingFolderChart(QString airport, QString chart);

    void downloadProgress(double received, double total);

    void finished(QString airport);

    void notFound(QString airport);

    void failed(QString airport, QString error);

    void processFinished();

    void processCanceled();

private:
    bool downloadFile(QString url, QString filePath);

    bool checkExists(QString path, QString airport, bool openCharts);

    void downloadFailed(QString airport, CURLcode errorCode);

    void setCurl(bool showProgress = false);

    void wait3Seconds();

    CURL* curl;

    CURLcode result;

    char errorBuffer[CURL_ERROR_SIZE];

};

#endif // DOWNLOADER_H
