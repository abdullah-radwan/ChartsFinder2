#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include "curl/curl.h"
#include "configeditor.h"

class Downloader : public QObject
{
    Q_OBJECT

public:
    Downloader(ConfigEditor::Config *config);
    ~Downloader();
    void cancel();

    bool valid = true;
    bool running = false;
    bool canceled;
    bool showProgress;

signals:
    void chartsExists(QString airport);
    void searchingCharts(QString airport);
    void downloadingCharts(QString airport);
    void downloadingFolderCharts(QString airport, QString chart);
    void downloadProgress(double received, double total);
    void downloadFinished(QString airport);
    void processFinished();

    void chartsNotFound(QString airport);
    void downloadFailed(QString airport, QString error);
    void processCanceled();

public slots:
    void download(QStringList airports);

private:
    bool checkExists(QString airport, QStringList suffixes);
    bool downloadFile(QString url, QString filePath);
    void downloadError(QString airport, CURLcode errorCode);
    void wait3Seconds();

    ConfigEditor::Config *config;
    CURL *curl;
    CURLcode result;
    char errorBuffer[CURL_ERROR_SIZE];
};

#endif // DOWNLOADER_H
