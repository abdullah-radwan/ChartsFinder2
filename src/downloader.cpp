#include "downloader.h"
#include <QGumboParser/qgumbodocument.h>
#include <QGumboParser/qgumbonode.h>
#include <QDesktopServices>

int progressCallback(void* clientp, double dltotal, double dlnow, double ultotal, double ulnow)
{
    Q_UNUSED(ultotal) Q_UNUSED(ulnow) // Avoid unused variable warning

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
    Q_UNUSED(ptr) Q_UNUSED(userdata)

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

    if(curl)
    {
        // Follow redirects
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        // Fail if the server returns error code
        curl_easy_setopt(curl, CURLOPT_FAILONERROR, 1);
        // Know error details
        curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, errorBuffer);
        // Set a progress callback
        curl_easy_setopt(curl, CURLOPT_NOPROGRESS, 0);
        // Set progress function
        curl_easy_setopt(curl, CURLOPT_PROGRESSFUNCTION, progressCallback);
        // Set the data as this downloader instance
        curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, this);
    }
    // If NULL
    else
    {
        qDebug() << "Warning: Couldn't initialize cURL!";

        valid = false;
    }
}

void Downloader::download(Config::ConfigStruct config, QStringList airports)
{
    // Reset indications
    running = true;
    canceled = false;

    int index = 0;

    QStringList suffixes;

    // Add every known suffix to check if the chart exists
    for(const Config::Resource& resource : config.resources) if(!suffixes.contains(resource.suffix)) suffixes.append(resource.suffix);

    for(const QString& airport : airports)
    {
        if(canceled) break;

        qDebug() << "Downloading" << airport << "chart";

        // If the chart already exists
        if(checkExists(config.path, airport, suffixes, config.openChart, config.openFolder))
        {
            // Go to the next airport
            index++;

            // If there is another airport, delay 3 seconds
            if(index < airports.size()) wait3Seconds();

            // Go to the next airport
            continue;
        }

        // Set the GUI
        emit searchingForChart(airport);

        // Process GUI events
        QCoreApplication::processEvents();

        // Set internal indication
        bool found = false;
        bool oFailed = false;

        for(const Config::Resource& resource : config.resources)
        {
            bool failed = false;
            oFailed = false;

            QString url;

            // Set the initial URL based on the resource type
            // For normal, set the URL plus the suffix to check if it's available
            // For folder, just set the URL
            if(resource.type) url = resource.url.arg(airport);
            else url = resource.url.arg(airport) + resource.suffix;

            // Connect initially here to check if the URL is valid
            showProgress = false;
            curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, dummyCallack);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, NULL);

            result = curl_easy_perform(curl);

            if(canceled) break;

            // If the URL isn't valid, go to the next resource
            if(result != CURLE_OK) {
                qDebug() << "Couldn't find the chart in" << url;
                continue;
            } else qDebug() << "Downloading from" << url;

            // If it's a normal resource
            if(resource.type)
            {
                QString html;

                // Download the page
                showProgress = false;
                curl_easy_setopt(curl, CURLOPT_URL, url.toStdString().c_str());
                curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, writeHtmlCallback);
                curl_easy_setopt(curl, CURLOPT_WRITEDATA, &html);

                result = curl_easy_perform(curl);

                if(canceled) break;

                // If the download failed
                if(result != CURLE_OK) { oFailed = true; break; }

                // Parse the HTML page
                QGumboDocument doc = QGumboDocument::parse(html);

                // Get the root tag
                QGumboNode root = doc.rootNode();

                // Get all URLs
                QGumboNodes nodes = root.getElementsByTagName(HtmlTag::A);

                QStringList files;

                // Check all links in the page
                for(const QGumboNode& node : nodes)
                {
                    QCoreApplication::processEvents();

                    // Get the link
                    QString attr = node.getAttribute("href");

                    // Add it if it points to a PDF file
                    if(attr.endsWith(resource.suffix)) files.append(url + attr);
                }

                // If empty, go to the next resource
                if(files.isEmpty()) continue;
                else found = true;

                // Set the airport chart folder path
                QString chartPath = config.path + airport + "/";

                // If the chart path couldn't be created
                if(!QDir(chartPath).mkpath(".")) { oFailed = true; break; }

                for(const QString& file : files)
                {
                    QCoreApplication::processEvents();

                    // Get the chart file name by getting the last entry of the URL
                    // Which is the filename, then remove the extension.
                    QString chartName = file.split("/").last().remove(resource.suffix);

                    emit downloadingFolderChart(airport, chartName);

                    // Download the PDF file
                    failed = !downloadFile(file, chartPath + chartName + resource.suffix);

                    if(canceled) break;

                    if(failed) break;
                }

                if(canceled)
                {
                    // If canceled and remove files is set, delete the airport chart folder
                    if(config.removeFiles) QDir(chartPath).removeRecursively();
                    break;
                }

                if(failed) { oFailed = true; break; }

                emit finished(airport);

                // Open the airport chart folder
                if(config.openChart) QDesktopServices::openUrl(QUrl::fromLocalFile(chartPath));

                if(config.openFolder) QDesktopServices::openUrl(QUrl::fromLocalFile(config.path));

                break;
            }
            else
            {
                emit downloadingChart(airport);

                QCoreApplication::processEvents();

                QString downloadPath = config.path + airport + resource.suffix;

                // If file download failed
                failed = !downloadFile(url, downloadPath);

                // Set the outer failed and break
                if(failed) { oFailed = true; break; }

                found = true;

                emit finished(airport);

                // Open the chart file if downloaded
                if(config.openChart) QDesktopServices::openUrl(QUrl::fromLocalFile(downloadPath));

                if(config.openFolder) QDesktopServices::openUrl(QUrl::fromLocalFile(config.path));

                break;
            }
        }

        if(canceled) { emit processCanceled(); break; }

        if(oFailed) downloadFailed(airport, result);
        else if(!found) { qDebug() << "Couldn't find" << airport << "chart"; emit notFound(airport); }
        else qDebug() << airport << "chart is downloaded";

        // Go to the next airport
        index++;

        // Delay 3 seconds if there is another airport
        if(index < airports.size()) wait3Seconds();
        else emit processFinished();
    }

    running = false;
}

bool Downloader::checkExists(QString path, QString airport, QStringList suffixes, bool openChart, bool openFolder)
{
    for(const QString& suffix : suffixes)
    {
        // If a PDF file with the airport name exists
        if(QFileInfo(path + airport + suffix).exists())
        {
            qDebug() << airport << "chart' file already exists";

            emit exists(airport);

            // Open it if open chart is set
            if(openChart) QDesktopServices::openUrl(QUrl::fromLocalFile(path + airport + suffix));

            if(openFolder) QDesktopServices::openUrl(QUrl::fromLocalFile(path));

            return true;
        }
    }

    // If a folder with the airport name exists
    if(QFileInfo(path + airport).exists())
    {
        qDebug() << airport << "chart folder already exists";

        emit exists(airport);

        if(openChart) QDesktopServices::openUrl(QUrl::fromLocalFile(path + airport));

        if(openFolder) QDesktopServices::openUrl(QUrl::fromLocalFile(path));

        return true;
    }
    else return false;
}

bool Downloader::downloadFile(QString url, QString filePath)
{ 
    QFile file(filePath);

    // If the file didn't open, return failed
    if(!file.open(QIODevice::WriteOnly)) return false;

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

    if(result == CURLE_OK) qDebug() << chartName << "is downloaded";
    // If aborted, remove the file as it's invalid
    else if(result == CURLE_ABORTED_BY_CALLBACK) file.remove();
    else qDebug() << "Couldn't download" << chartName;

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
