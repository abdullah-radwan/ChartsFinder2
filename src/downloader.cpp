#include "downloader.h"
#include <QDesktopServices>
#include <qgumbodocument.h>
#include <qgumbonode.h>

Downloader::Downloader(){}

Downloader::~Downloader(){}

void Downloader::download(Config::configStruct config, QStringList airports)
{
    isCanceled = false;

    int index = 0;

    foreach(QString airport, airports)
    {
        if(isCanceled) break;

        if(checkExists(config.path, airport, config.openCharts))
        {
            index++;

            if(index < airports.size())
            {
                QTimer::singleShot(3000, &loop, SLOT(quit()));
                loop.exec();
            }

            continue;
        }

        QCoreApplication::processEvents();

        emit searchingForCharts(airport);

        bool found = false;
        bool oFailed = false;

        foreach(QStringList resource, config.resources)
        {
            bool failed = false;
            oFailed = false;

            QNetworkRequest request(resource.first().arg(airport));
            QNetworkReply* reply = manager.head(request);

            actReply = reply;

            QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));
            loop.exec();

            QCoreApplication::processEvents();

            if(reply->error()) continue;

            if(resource.last() == "Normal")
            {
                emit downloadingCharts(airport);

                if(isCanceled) break;

                QCoreApplication::processEvents();

                failed = !(downloadFile(resource.first().arg(airport), config.path + airport + ".pdf"));

                if(isCanceled) break;

                if(failed) { oFailed = true; break; }

                found = true;

                emit finished(airport);

                if(config.openCharts) QDesktopServices::openUrl(QUrl::fromLocalFile(config.path + airport + ".pdf"));

                break;
            }

            else
            {
                actReply = reply = manager.get(request);

                QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply* reply)
                {
                    failed = (reply->error() != QNetworkReply::NetworkError::NoError);
                });

                QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));

                loop.exec();

                if(failed) { oFailed = true; break; }

                const char* html = reply->readAll().toStdString().c_str();

                auto doc = QGumboDocument::parse(html);

                auto root = doc.rootNode();

                auto nodes = root.getElementsByTagName(HtmlTag::A);

                QStringList filesUrl;

                foreach(QGumboNode node, nodes)
                {
                    QCoreApplication::processEvents();

                    QString attr = node.getAttribute("href");

                    if(attr.endsWith(".pdf")) filesUrl.append(resource.first().arg(airport) + attr);
                }

                if(!filesUrl.isEmpty()) found = true;
                else continue;

                QString chartsPath = config.path + airport + "/";

                QDir(chartsPath).mkpath(".");

                foreach(QString url, filesUrl)
                {
                    QCoreApplication::processEvents();

                    QString chartName = url.split("/").last().remove(".pdf");

                    emit downloadingFolderChart(airport, chartName);

                    QString filePath = chartsPath + chartName + ".pdf";

                    failed = !downloadFile(url, filePath);

                    if(isCanceled) break;

                    if(failed) break;
                }

                if(isCanceled)
                {
                    if(config.removeFiles) QDir(chartsPath).removeRecursively();
                    break;
                }

                if(failed) { oFailed = true; break; }

                if(config.openCharts) QDesktopServices::openUrl(QUrl::fromLocalFile(chartsPath));

                emit finished(airport);

                break;
            }
        }

        if(isCanceled) {emit canceled(); break;}

        index++;

        if(oFailed) emit failed(airport);
        else if(!found) emit notFound(airport);

        if(index < airports.size())
        {
            QEventLoop loop;
            QTimer::singleShot(3000, &loop, SLOT(quit()));
            loop.exec();
        }
        else emit processFinished();
    }
}

bool Downloader::checkExists(QString path, QString airport, bool openCharts)
{
    if(QFileInfo(path + airport + ".pdf").exists())
    {
        emit exists(airport);

        if(openCharts) QDesktopServices::openUrl(QUrl::fromLocalFile(path + airport + ".pdf"));

        return true;
    }
    else if(QFileInfo(path + "/" + airport).exists())
    {
        emit exists(airport);

        if(openCharts) QDesktopServices::openUrl(QUrl::fromLocalFile(path + "/" + airport));

        return true;
    }
    else return false;
}

bool Downloader::downloadFile(QString url, QString filePath)
{
    QNetworkReply* reply = manager.get(QNetworkRequest(url));

    actReply = reply;

    bool failed = false;

    QObject::connect(reply, &QNetworkReply::downloadProgress, [&](qint64 bytesReceived, qint64 bytesTotal)
    {
        emit downloadProgress(int(bytesReceived), int(bytesTotal));
    });

    QObject::connect(&manager, &QNetworkAccessManager::finished, [&](QNetworkReply* reply)
    {
        failed = (reply->error() != QNetworkReply::NetworkError::NoError);
    });

    QObject::connect(&manager, SIGNAL(finished(QNetworkReply*)), &loop, SLOT(quit()));

    loop.exec();

    QCoreApplication::processEvents();

    if(failed) return false;

    QFile file(filePath);
    failed = failed || !file.open(QIODevice::WriteOnly);
    failed = failed || file.write(reply->readAll()) == -1;
    file.close();

    reply->deleteLater();

    return !failed;
}

void Downloader::cancel() { actReply->close(); isCanceled = true; }
