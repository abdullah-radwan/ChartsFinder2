#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QMessageBox>
#include <QCloseEvent>

MainWindow::MainWindow(ConfigEditor::Config *config) : QMainWindow(nullptr), ui(new Ui::MainWindow),
    settingsDialog(this, config), downloaderThread(this)
{
    // Read the config
    this->config = config;

    // Set the downloader
    setDownloader();

    setUpdater();

    ui->setupUi(this);

    if (config->mainSize != QSize(-1, -1)) resize(config->mainSize);
    if (config->mainPos != QPoint(-1, -1)) move(config->mainPos);

    show();

    // Check for updates if needed
    if (config->checkUpdates) checkUpdates();
}

void MainWindow::setDownloader()
{
    downloader = new Downloader(config);

    if (!downloader->valid) {
        statusBar()->showMessage(tr("Warning: Couldn't initialize the downloader!"));

        ui->getButton->setEnabled(false);
    }

    downloader->moveToThread(&downloaderThread);

    connect(this, &MainWindow::download, downloader, &Downloader::download);

    connect(downloader, &Downloader::chartsExists, this, &MainWindow::onChartsExists);
    connect(downloader, &Downloader::searchingCharts, this, &MainWindow::onSearchingCharts);
    connect(downloader, &Downloader::downloadingCharts, this, &MainWindow::onDownloadingCharts);
    connect(downloader, &Downloader::downloadingFolderCharts, this, &MainWindow::onDownloadingFolderCharts);
    connect(downloader, &Downloader::downloadProgress, this, &MainWindow::onDownloadProgress);
    connect(downloader, &Downloader::downloadFinished, this, &MainWindow::onDownloadFinished);
    connect(downloader, &Downloader::processFinished, this, &MainWindow::onProcessFinished);

    connect(downloader, &Downloader::chartsNotFound, this, &MainWindow::onChartsNotFound);
    connect(downloader, &Downloader::downloadFailed, this, &MainWindow::onDownloadFailed);
    connect(downloader, &Downloader::processCanceled, this, &MainWindow::onProcessCanceled);

    connect(&downloaderThread, &QThread::finished, downloader, &QObject::deleteLater);

    downloaderThread.start();
}

void MainWindow::setUpdater()
{
    updater = QtAutoUpdater::Updater::create("qtifw", {{"path", qApp->applicationDirPath() + "/maintenancetool"}},
    qApp);

    if (updater) {
        connect(updater, &QtAutoUpdater::Updater::checkUpdatesDone, [&](QtAutoUpdater::Updater::State
        state) {
            switch (state) {
            case QtAutoUpdater::Updater::State::Error:
                qDebug() << "An error occured";
                config->checkUpdates = false;
                break;

            case QtAutoUpdater::Updater::State::NoUpdates:
                qDebug() << "No updates are found";
                break;

            case QtAutoUpdater::Updater::State::NewUpdates:
                qDebug() << "An update was found" << updater->updateInfo();
                break;

            default:
                break;
            }
        });
    } else {
        qDebug() << "Couldn't create the updater";
    }
}

void MainWindow::checkUpdates()
{
    if (!updater) {
        QMessageBox::warning(this, tr("Updater initialization"), tr("Couldn't initialize the updater."));
        return;
    }

    qDebug() << "Starting updater";

    (new QtAutoUpdater::UpdateController(updater, this))->
            start(QtAutoUpdater::UpdateController::DisplayLevel::ExtendedInfo);
}

void MainWindow::on_getButton_clicked()
{
    if (downloader->running) {
        downloader->cancel();
        return;
    }

    if (ui->airportsEdit->text().isEmpty())
        return;

    // Set all letters to capitial and split the codes
    QStringList airports = ui->airportsEdit->text().toUpper().split(" ");

    if (QFileInfo(config->chartsPath).exists()) {
        emit download(airports);
    } else {
        // Create the charts folder
        if (QDir(config->chartsPath).mkpath(".")) {
            emit download(airports);
        // If it couldn't be created
        } else {
            statusBar()->showMessage(tr("Couldn't download the charts: Couldn't create the charts folder."));
        }
    }
}

void MainWindow::on_airportsEdit_returnPressed()
{
    on_getButton_clicked();
}

void MainWindow::on_actionSettings_triggered()
{
    settingsDialog.exec();
}

void MainWindow::on_actionCheck_triggered()
{
    checkUpdates();
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Charts Finder"), tr("Charts Finder\n"
                                                           "An app to get IFR charts for your flights.\n"

                                                           "Version: %1\n"
                                                           "Release date: %2\n"

                                                           "Copyright © Abdullah Radwan")
                       // Set the version and the release date to the current locale
                       .arg("2.3", locale().toString(QDate::fromString("7/6/2020", "d/M/yyyy"), "d MMMM yyyy")));
}

void MainWindow::onChartsExists(QString airport)
{
    statusBar()->showMessage(tr("%1 charts already exists.").arg(airport));
}

void MainWindow::onSearchingCharts(QString airport)
{
    ui->getButton->setText(tr("Cancel"));

    ui->getButton->setIcon(QIcon(":/cancel.svg"));

    statusBar()->showMessage(tr("Searching for %1 charts...").arg(airport));
}

void MainWindow::onDownloadingCharts(QString airport)
{
    statusBar()->showMessage(tr("Downloading %1 charts...").arg(airport));

    ui->progressBar->show();
}

void MainWindow::onDownloadingFolderCharts(QString airport, QString chart)
{
    statusBar()->showMessage(tr("Downloading %1 %2 chart...").arg(airport, chart));

    if (ui->progressBar->isHidden())
        ui->progressBar->show();
}

void MainWindow::onDownloadProgress(double received, double total)
{
    ui->progressBar->setMaximum(total);

    ui->progressBar->setValue(received);
}

void MainWindow::onDownloadFinished(QString airport)
{
    statusBar()->showMessage(tr("%1 charts were downloaded.").arg(airport));

    ui->progressBar->setValue(0);

    ui->progressBar->hide();
}

void MainWindow::onProcessFinished()
{
    ui->progressBar->hide();

    ui->getButton->setIcon(QIcon(":/download.svg"));

    ui->getButton->setText(tr("Get charts"));
}

void MainWindow::onChartsNotFound(QString airport)
{
    statusBar()->showMessage(tr("Couldn't find %1 charts.").arg(airport));

    ui->progressBar->hide();
}

void MainWindow::onDownloadFailed(QString airport, QString error)
{
    statusBar()->showMessage(tr("Couldn't download %1 charts. The following error happened:\n").arg(
                                 airport) + error);

    ui->progressBar->hide();
}

void MainWindow::onProcessCanceled()
{
    statusBar()->showMessage(tr("The download process was canceled."));

    ui->progressBar->hide();

    ui->getButton->setText(tr("Get charts"));

    ui->getButton->setIcon(QIcon(":/download.svg"));
}

void MainWindow::closeEvent(QCloseEvent *bar)
{
    // Confirm exists if there is downloads in progress
    if (downloader->running) {
        if (QMessageBox::question(this, tr("Are you sure you want to quit?"),
                                  tr("Are you sure you want to quit? There are downloads in progress."))) {
            downloader->cancel();
        } else {
            bar->ignore();
            return;
        }
    }

    // Save the window size and position
    config->mainSize = size();
    config->mainPos = pos();

    bar->accept();
}

MainWindow::~MainWindow()
{
    downloaderThread.quit();
    downloaderThread.wait();

    delete ui;
}
