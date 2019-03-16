#ifndef DOWNLOADER_H
#define DOWNLOADER_H

#include <QtNetwork>
#include <QMap>
#include "config.h"

class Downloader : public QObject
{
    Q_OBJECT
public:
    Downloader();

    void download(Config::configStruct config, QStringList airports);

    void cancel();

    ~Downloader();

signals:
    void exists(QString airport);

    void searchingForCharts(QString airport);

    void downloadingCharts(QString airport);

    void downloadingFolderChart(QString airport, QString chart);

    void downloadProgress(int bytesReceived, int bytesTotal);

    void finished(QString airport);

    void notFound(QString airport);

    void failed(QString airport);

    void processFinished();

    void canceled();

private:
    bool downloadFile(QString url, QString filePath);
    bool checkExists(QString path, QString airport, bool openCharts);

    QNetworkAccessManager manager;

    QNetworkReply* actReply;

    QEventLoop loop;

    bool isCanceled;
};

#endif // DOWNLOADER_H
