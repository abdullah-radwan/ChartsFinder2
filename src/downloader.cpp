#include "downloader.h"
#include <QGumboParser/qgumbodocument.h>
#include <QGumboParser/qgumbonode.h>
#include <QDesktopServices>

int progressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    Q_UNUSED(ultotal); Q_UNUSED(ulnow);

    // Process GUI events
    QCoreApplication::processEvents();

    auto downloader = static_cast<Downloader*>(clientp);

    // If the progress is to be shown (Not needed while checking for URL validity)
    if(downloader->showProgress) emit downloader->downloadProgress(dlnow, dltotal);

    // Return 0 if canceled if false, 1 (to abort the operation) if canceled is true
    return downloader->canceled;
}

// Dummy callback to avoid cURL writing in the output
size_t dummyCallack(void* ptr, size_t size, size_t nmemb, void* userdata)
{
    Q_UNUSED(ptr); Q_UNUSED(userdata);

    return size * nmemb;
}

size_t writeCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    // Write the data to the file
    return static_cast<size_t>(static_cast<QFile*>(userdata)->write(ptr, size * nmemb));
}

size_t writeHtmlCallback(char* ptr, size_t size, size_t nmemb, void* userdata)
{
    // Add the HTML to the string
    static_cast<QString*>(userdata)->append(ptr);

    return size * nmemb;
}

Downloader::Downloader()
{
    curl = curl_easy_init();

    if(!curl)
    {
        qDebug() << "Warning: Couldn't initialize cURL!";

        valid = false;
    }
}

void Downloader::download(Config::configStruct config, QStringList airports)
{
    // Reset indications
    running = true;
    canceled = false;

    int index = 0;

    foreach(QString airport, airports)
    {
        if(canceled) break;

        qDebug() << "Downloading" << airport << "charts";

        // If the chart already exists
        if(checkExists(config.path, airport, config.openCharts))
        {
            // Go to the next airport
            index++;

            // If there is another airport, delay 3 seconds
            if(index < airports.size()) wait3Seconds();

            // Go to the next airport
            continue;
        }

        // Set the GUI
        emit searchingForCharts(airport);

        // Process GUI events
        QCoreApplication::processEvents();

        // Set internal indication
        bool found = false;
        bool oFailed = false;

        foreach(QStringList resource, config.resources)
        {
            bool failed = false;
            oFailed = false;

            // Connect initially here to check if the URL is valid
            setCurl();
            curl_easy_setopt(curl, CURLOPT_URL, resource.first().arg(airport).toStdString().c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummyCallack);

            result = curl_easy_perform(curl);

            curl_easy_reset(curl);

            if(canceled) break;

            // If the URL isn't valid, go to the next resource
            if(result != CURLE_OK) {
                qDebug() << "Can't find charts in" << resource.first().arg(airport);
                continue;
            } else qDebug() << "Downloading from" << resource.first().arg(airport);

            // If it's a normal resource
            if(resource.last() == "0")
            {
                emit downloadingCharts(airport);

                QCoreApplication::processEvents();

                // If file download failed
                failed = !downloadFile(resource.first().arg(airport), config.path + airport + ".pdf");

                // Set the outer failed and break
                if(failed) { oFailed = true; break; }

                found = true;

                emit finished(airport);

                // Open the chart file if downloaded
                if(config.openCharts) QDesktopServices::openUrl(QUrl::fromLocalFile(config.path + airport + ".pdf"));

                break;
            }
            else
            {
                QString html;

                // Download the page
                setCurl();
                curl_easy_setopt(curl, CURLOPT_URL, resource.first().arg(airport).toStdString().c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeHtmlCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);

                result = curl_easy_perform(curl);

                curl_easy_reset(curl);

                if(canceled) break;

                // If the download failed
                if(result != CURLE_OK) { oFailed = true; break; }

                // Parse the HTML page
                QGumboDocument doc = QGumboDocument::parse(html);

                // Get the root tag
                QGumboNode root = doc.rootNode();

                // Get all URLs
                QGumboNodes nodes = root.getElementsByTagName(HtmlTag::A);

                QStringList filesUrl;

                // Check all links in the page
                foreach(QGumboNode node, nodes)
                {
                    QCoreApplication::processEvents();

                    // Get the link
                    QString attr = node.getAttribute("href");

                    // Add it if it points to a PDF file
                    if(attr.endsWith(".pdf")) filesUrl.append(resource.first().arg(airport) + attr);
                }

                // If empty, go to the next resource
                if(filesUrl.isEmpty()) continue;
                else found = true;

                // Set the airports charts folder path
                QString chartsPath = config.path + airport + "/";

                // If the charts path couldn't be created
                if(!QDir(chartsPath).mkpath(".")) { oFailed = true; break; }

                foreach(QString url, filesUrl)
                {
                    QCoreApplication::processEvents();

                    // Get the chart file name by getting the last entry of the URL
                    // Which is the filename, then remove the extention.
                    QString chartName = url.split("/").last().remove(".pdf");

                    emit downloadingFolderChart(airport, chartName);

                    // Download the PDF file
                    failed = !downloadFile(url, chartsPath + chartName + ".pdf");

                    if(canceled) break;

                    if(failed) break;
                }

                if(canceled)
                {
                    // If canceled and remove files is set, delete the airport charts folder
                    if(config.removeFiles) QDir(chartsPath).removeRecursively();
                    break;
                }

                if(failed) { oFailed = true; break; }

                emit finished(airport);

                // Open the airport charts folder
                if(config.openCharts) QDesktopServices::openUrl(QUrl::fromLocalFile(chartsPath));

                break;
            }
        }

        if(canceled) { emit processCanceled(); break; }

        if(oFailed) downloadFailed(airport, result);
        else if(!found) { qDebug() << "Couldn't find" << airport << "charts"; emit notFound(airport); }
        else qDebug() << airport << "charts were downloaded";

        // Go to the next airport
        index++;

        // Delay 3 seconds if there is another airport
        if(index < airports.size()) wait3Seconds();
        else emit processFinished();
    }

    running = false;
}

bool Downloader::checkExists(QString path, QString airport, bool openCharts)
{
    // If a PDF file with the airport name exists
    if(QFileInfo(path + airport + ".pdf").exists())
    {
        qDebug() << airport << "charts' file already exists";

        emit exists(airport);

        // Open it if open charts is set
        if(openCharts) QDesktopServices::openUrl(QUrl::fromLocalFile(path + airport + ".pdf"));

        return true;
    }
    // If a folder with the airport name exists
    else if(QFileInfo(path + airport).exists())
    {
        qDebug() << airport << "charts' folder already exists";

        emit exists(airport);

        if(openCharts) QDesktopServices::openUrl(QUrl::fromLocalFile(path + "/" + airport));

        return true;
    }
    else return false;
}

bool Downloader::downloadFile(QString url, QString filePath)
{ 
    QFile file(filePath);

    // If the file didn't open, return failed
    if(!file.open(QIODevice::WriteOnly)) return false;

    setCurl(true);
    curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeCallback);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &file);

    // Get the chart name by getting the last entry of the URL
    QString chartName = url.split("/").last();

    qDebug() << "Downloading" << chartName;

    result = curl_easy_perform(curl);

    curl_easy_reset(curl);

    // Close the file
    file.close();

    if(result == CURLE_OK) qDebug() << chartName << "was downloaded";
    // If aborted, remove the file as it'll be invalid
    else if(result == CURLE_ABORTED_BY_CALLBACK) file.remove();
    else qDebug() << "Failed to download" << chartName;

    return result == CURLE_OK;
}

void Downloader::downloadFailed(QString airport, CURLcode errorCode)
{
    // Get a readable error
    QString error = curl_easy_strerror(errorCode);

    // Write the detailed error message in the log
    qDebug() << "Error:" << error << errorBuffer;

    emit failed(airport, error);
}

void Downloader::setCurl(bool showProgress)
{
    // Set the global show progress
    this->showProgress = showProgress;

    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
    curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
    curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
    curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
    curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
    curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
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
    qDebug() << "Download is canceled";

    canceled = true;
}

Downloader::~Downloader()
{
    curl_easy_cleanup(curl);
}
