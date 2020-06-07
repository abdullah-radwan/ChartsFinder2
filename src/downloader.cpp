#include "downloader.h"

#include <QGumboParser/qgumbodocument.h>
#include <QGumboParser/qgumbonode.h>
#include <QDesktopServices>
#include <QTimer>
#include <QDir>
#include <QFile>
#include <QUrl>
#include <QEventLoop>
#include <QDebug>

int progressCallback(void *clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    Q_UNUSED(ultotal) Q_UNUSED(ulnow) // Avoid unused variable warning

    auto downloader = static_cast<Downloader *>(clientp);

    // If the progress is to be shown (not needed while checking for URL validity)
    if (downloader->showProgress)
        emit downloader->downloadProgress(dlnow, dltotal);

    // Return 0 if canceled is false, 1 (to abort the operation) if canceled is true
    return downloader->canceled;
}

// Dummy callback to avoid cURL writing in the output
size_t dummyCallack(void *ptr, size_t size, size_t nmemb, void *userdata)
{
    Q_UNUSED(ptr) Q_UNUSED(userdata)

    return size * nmemb;
}

size_t writeCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    // Write the data to the file
    return static_cast<size_t>(static_cast<QFile *>(userdata)->write(ptr, size * nmemb));
}

size_t writeHtmlCallback(char *ptr, size_t size, size_t nmemb, void *userdata)
{
    // Add the HTML to the string
    static_cast<QString *>(userdata)->append(ptr);

    return size * nmemb;
}

Downloader::Downloader(ConfigEditor::Config *config)
{
    this->config = config;
    curl = curl_easy_init();

    if (curl) {
        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        // Fail if the server returns error code
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
        // Write error details
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
        // Set a progress callback
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        // Set progress function
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
        // Set the data as this downloader instance
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
    }
    else {
        qDebug() << "Couldn't initialize cURL";
        valid = false;
    }
}

void Downloader::download(QStringList airports)
{
    running = true;
    canceled = false;

    int index = 0;

    QStringList suffixes;

    // Add every known suffix to check if the charts exist
    for (ConfigEditor::Resource resource : config->resources) {
        resource.suffix.prepend(".");

        if (!suffixes.contains(resource.suffix))
            suffixes.append(resource.suffix);
    }

    for (const QString &airport : airports) {
        if (canceled) break;

        qDebug() << "Downloading" << airport << "charts";

        if (checkExists(airport, suffixes)) {
            // Set the index for the next airport
            index++;

            // If there is another airport, delay 3 seconds
            if (index < airports.size())
                wait3Seconds();

            // Go to the next airport
            continue;
        }

        emit searchingCharts(airport);

        bool found = false;
        bool oFailed = false;

        for (ConfigEditor::Resource resource : config->resources) {
            resource.suffix.prepend(".");

            bool failed = false;
            oFailed = false;

            QString url;

            // Set the initial URL based on the resource type
            // For folder, just set the URL
            // For normal, set the URL plus the suffix to check if it's available
            if (resource.type) url = resource.url.arg(airport);
            else url = resource.url.arg(airport) + resource.suffix;

            // Connect initially here to check if the URL is valid
            showProgress = false;
            curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummyCallack);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

            result = curl_easy_perform(curl);

            if (canceled)
                break;

            // If the URL isn't valid, go to the next resource
            if (result != CURLE_OK) {
                qDebug() << "Couldn't find the charts in" << url;
                continue;
            } else {
                qDebug() << "Downloading from" << url;
            }

            // If it's a folder resource
            if (resource.type) {
                QString html;

                // Download the page
                showProgress = false;
                curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeHtmlCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);

                result = curl_easy_perform(curl);

                if (canceled)
                    break;

                // If the download failed
                if (result != CURLE_OK) {
                    oFailed = true;
                    break;
                }

                // Parse the HTML page
                QGumboDocument doc = QGumboDocument::parse(html);

                // Get the root tag
                QGumboNode root = doc.rootNode();

                // Get all URLs
                QGumboNodes nodes = root.getElementsByTagName(HtmlTag::A);

                QStringList files;

                // Check all links in the page
                for (const QGumboNode &node : nodes) {
                    // Get the link
                    QString attr = node.getAttribute("href");

                    // Add it if it points to the resource files suffix
                    if (attr.endsWith(resource.suffix))
                        files.append(url + attr);
                }

                // If empty, go to the next resource
                if (files.isEmpty()) {
                    continue;
                } else {
                    found = true;
                }

                // Set the airport charts folder path
                QString chartsPath = config->chartsPath + airport + "/";

                // If the charts path couldn't be created
                if (!QDir(chartsPath).mkpath(".")) {
                    oFailed = true;
                    break;
                }

                for (const QString &file : files) {
                    // Get the chart file name by getting the last entry of the URL
                    // Which is the filename, then remove the extension.
                    QString chartName = file.split("/").last().remove(resource.suffix);

                    emit downloadingFolderCharts(airport, chartName);

                    // Download the PDF file
                    failed = !downloadFile(file, chartsPath + chartName + resource.suffix);

                    if (canceled)
                        break;

                    if (failed)
                        break;
                }

                if (canceled) {
                    // If canceled, delete the airport charts folder
                    QDir(chartsPath).removeRecursively();
                    break;
                }

                if (failed) {
                    oFailed = true;
                    break;
                }

                emit downloadFinished(airport);

                if (config->openCharts)
                    QDesktopServices::openUrl(QUrl::fromLocalFile(chartsPath));

                break;
            } else {
                emit downloadingCharts(airport);

                QString downloadPath = config->chartsPath + airport + resource.suffix;

                // If file download failed
                failed = !downloadFile(url, downloadPath);

                if (failed) {
                    oFailed = true;
                    break;
                }

                found = true;

                emit downloadFinished(airport);

                if (config->openCharts)
                    QDesktopServices::openUrl(QUrl::fromLocalFile(downloadPath));

                break;
            }
        }

        if (canceled) {
            emit processCanceled();
            break;
        }

        if (oFailed) {
            downloadError(airport, result);
        } else if (!found) {
            qDebug() << "Couldn't find" << airport << "charts";
            emit chartsNotFound(airport);
        } else {
            qDebug() << airport << "charts were downloaded";
        }

        // Go to the next airport
        index++;

        // Delay 3 seconds if there is another airport
        if (index < airports.size()) {
            wait3Seconds();
        } else {
            if (config->openFolder)
                QDesktopServices::openUrl(QUrl::fromLocalFile(config->chartsPath));

            emit processFinished();
        }
    }

    running = false;
}

bool Downloader::checkExists(QString airport, QStringList suffixes)
{
    for (const QString &suffix : suffixes) {
        // If file with the airport name and resource suffix exists
        if (QFileInfo(config->chartsPath + airport + suffix).exists()) {
            qDebug() << airport << "charts file already exists";

            emit chartsExists(airport);

            if (config->openCharts)
                QDesktopServices::openUrl(QUrl::fromLocalFile(config->chartsPath + airport + suffix));

            if (config->openFolder)
                QDesktopServices::openUrl(QUrl::fromLocalFile(config->chartsPath));

            return true;
        }
    }

    // If a folder with the airport name exists
    if (QFileInfo(config->chartsPath + airport).exists()) {
        qDebug() << airport << "charts folder already exists";

        emit chartsExists(airport);

        if (config->openCharts)
            QDesktopServices::openUrl(QUrl::fromLocalFile(config->chartsPath + airport));

        if (config->openFolder)
            QDesktopServices::openUrl(QUrl::fromLocalFile(config->chartsPath));

        return true;
    } else return false;
}

bool Downloader::downloadFile(QString url, QString filePath)
{
    QFile file(filePath);

    // If the file didn't open, return failed
    if (!file.open(QIODevice::WriteOnly))
        return false;

    showProgress = true;
    curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

    // Get the chart name by getting the last entry of the URL
    QString chartName = url.split("/").last();

    qDebug() << "Downloading" << chartName;

    result = curl_easy_perform(curl);

    // Close the file
    file.close();

    if (result == CURLE_OK) {
        qDebug() << chartName << "was downloaded";
        // If aborted, remove the file as it's invalid
    } else if (result == CURLE_ABORTED_BY_CALLBACK) {
        file.remove();
    } else {
        qDebug() << "Couldn't download" << chartName;
    }

    return result == CURLE_OK;
}

void Downloader::downloadError(QString airport, CURLcode errorCode)
{
    // Get a readable error
    QString error = curl_easy_strerror(errorCode);

    // Write the detailed error message in the log
    qDebug() << "Error:" << error << errorBuffer;

    emit downloadFailed(airport, error);
}

void Downloader::wait3Seconds()
{
    QEventLoop loop;

    // Wait 3 seconds
    QTimer::singleShot(3000, &loop, SLOT(quit()));

    loop.exec();
}

void Downloader::cancel()
{
    qDebug() << "Download was canceled";

    canceled = true;
}

Downloader::~Downloader()
{
    curl_easy_cleanup(curl);
}
