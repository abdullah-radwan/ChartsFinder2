#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingswindow.h"
#include <QMessageBox>
#include <UpdateController>
#include <Updater>
#include <QCloseEvent>

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    // Read the config
    config = Config::readConfig();

    // Set the translators
    setTranslator();

    ui->setupUi(this);

    // If the window size isn't empty, set it
    if(!config.mainWinGeo.isEmpty()) setGeometry(config.mainWinGeo);

    // Set the downloader
    setDownloader();

    show();

    qDebug() << "Main window is shown";

    // Check for updates if needed
    if(config.checkUpdates) checkForUpdates();
}

void MainWindow::setTranslator()
{
    // No need to translate if the language is English, or the system language is English
    if(config.language == "English" || (config.language == "System" && QLocale::system().language() == QLocale::English)) return;

    qDebug() << "Setting translators";

    qtTranslator = new QTranslator(this);
    translator = new QTranslator(this);
    autoUpdaterTranslator = new QTranslator(this);

    // Set the system locale
    if(config.language == "System")
    {
        locale = QLocale::system();
        qDebug() << "Locale is set to system:" << locale.name();
    } else
    {
        locale = QLocale(config.language);
        qDebug() << "Locale is set to" << config.language;
    }

    // Load the translations
    if(qtTranslator->load("qt_" + locale.name(), qApp->applicationDirPath() + "/translations"))
        qDebug() << "Qt translation is loaded";
    else qDebug() << "Couldn't load Qt translation";

    if(translator->load("chartsfinder2_" + locale.name(), qApp->applicationDirPath() + "/translations"))
        qDebug() << "Charts Finder translation is loaded";
    else qDebug() << "Couldn't load Charts Finder translation";

    if(autoUpdaterTranslator->load("qtautoupdatergui_" + locale.name(), qApp->applicationDirPath() + "/translations"))
        qDebug() << "Auto updater translation is loaded";
    else qDebug() << "Couldn't load Auto updater translation";

    // Install the translators
    qApp->installTranslator(qtTranslator);
    qApp->installTranslator(translator);
    qApp->installTranslator(autoUpdaterTranslator);

    qDebug() << "Translators are set";
}

void MainWindow::setDownloader()
{
    downloader = new Downloader();

    if(!downloader->valid)
    {
        ui->downloadLabel->show();
        ui->downloadLabel->setText("Warning: Couldn't initialize the downloader!");
        ui->getButton->setEnabled(false);
    }

    connect(downloader, &Downloader::exists, [&](QString airport)
    {
        ui->downloadLabel->show();
        ui->downloadLabel->setText(tr("%1 charts already exists.").arg(airport));
    });

    connect(downloader, &Downloader::searchingForCharts, [&](QString airport)
    {
        ui->getButton->hide();
        ui->cancelButton->show();
        ui->downloadLabel->show();
        ui->downloadLabel->setText(tr("Searching for %1 charts...").arg(airport));
    });

    connect(downloader, &Downloader::downloadingCharts, [&](QString airport)
    {
        ui->downloadLabel->setText(tr("Downloading %1 charts...").arg(airport));
        ui->progressBar->show();
    });

    connect(downloader, &Downloader::downloadingFolderChart, [&](QString airport, QString chart)
    {
        ui->downloadLabel->setText(tr("Downloading %1 %2 chart...").arg(airport, chart));
        if(ui->progressBar->isHidden()) ui->progressBar->show();
    });

    connect(downloader, &Downloader::downloadProgress, [&](int received, int total)
    {
        ui->progressBar->setMaximum(total);
        ui->progressBar->setValue(received);
    });

    connect(downloader, &Downloader::finished, [&](QString airport)
    {
        ui->downloadLabel->setText(tr("%1 charts downloaded.").arg(airport));
        ui->progressBar->setValue(0);
        ui->progressBar->hide();
    });

    connect(downloader, &Downloader::notFound, [&](QString airport)
    {
        ui->downloadLabel->setText(tr("Couldn't find %1 charts.").arg(airport));
        ui->progressBar->hide();
    });

    connect(downloader, &Downloader::failed, [&](QString airport, QString error)
    {
        ui->downloadLabel->setText(tr("Failed to download %1 charts. The following error happened:\n").arg(airport) + error);
        ui->progressBar->hide();
    });

    connect(downloader, &Downloader::processFinished, [&]()
    {
        ui->progressBar->hide();
        ui->cancelButton->hide();
        ui->getButton->show();
    });

    connect(downloader, &Downloader::processCanceled, [&]()
    {
        ui->downloadLabel->setText(tr("Process was canceled."));
        ui->progressBar->hide();
        ui->cancelButton->hide();
        ui->getButton->show();
    });
}

void MainWindow::checkForUpdates()
{
    bool error = false;

    controller = new QtAutoUpdater::UpdateController(qApp->applicationDirPath() + "/maintenancetool", this);

    connect(controller->updater(), &QtAutoUpdater::Updater::checkUpdatesDone, [&](bool hasUpdate, bool hasError)
    {
        qDebug() << "Updater finished";

        if(hasUpdate) qDebug() << "An update was found" << controller->updater()->updateInfo();
        else if(hasError) { qDebug() << "An error occured" << controller->updater()->errorLog(); error = true; }
        else qDebug() << "No updates were found" << controller->updater()->errorLog();
    });

    qDebug() << "Starting updater";

    controller->start(QtAutoUpdater::UpdateController::ProgressLevel);

    // If an error occured, recheck for update next time
    config.checkUpdates = !error;
}

void MainWindow::on_getButton_clicked()
{
    if(ui->icaoEdit->text().isEmpty()) return;

    // Set all letters to capitial and split the codes
    QStringList airports = ui->icaoEdit->text().toUpper().split(" ");

    if(QFileInfo(config.path).exists()) downloader->download(config, airports);
    else
    {
        // Create the charts folder
        if(QDir(config.path).mkpath(".")) downloader->download(config, airports);
        // If it couldn't be created
        else ui->downloadLabel->setText(tr("Failed to download charts due to not being able to create charts folder."));
    }
}

void MainWindow::on_icaoEdit_returnPressed() { on_getButton_clicked(); }

void MainWindow::on_cancelButton_clicked() { downloader->cancel(); }

void MainWindow::on_actionSettings_triggered()
{
    // Run the settings window
    SettingsWindow settingsWin(this, config);

    QEventLoop loop;

    // Wait until it closes
    connect(&settingsWin, &SettingsWindow::closed, &loop, &QEventLoop::quit);

    loop.exec();

    // Set the config as modified from the settings window
    config = settingsWin.config;
}

void MainWindow::on_actionCheck_for_updates_triggered() { checkForUpdates(); }

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, tr("About Charts Finder"), tr("Charts Finder\n"
                                                           "A tool to get charts for your flights.\n"

                                                           "Version: %1\n"
                                                           "Release date: %2\n"

                                                           "Copyright © Abdullah Radwan")
                       // Set the version and the release date to the current locale
                       .arg(locale.toString(2.1), locale.toString(QDate::fromString("9/2019", "M/yyyy"), "MMMM yyyy")));
}

void MainWindow::closeEvent(QCloseEvent *bar)
{
    // Confirm exists if there is downloads in progress
    if(downloader->running)
    {
        if(QMessageBox::question(this, tr("Are you sure you want to quit?"),
                                 tr("Are you sure you want to quit? There are downloads in progress."))) downloader->cancel();
        else { bar->ignore(); return; }
    }

    // Save the window size and position
    config.mainWinGeo = geometry();

    bar->accept();

    qDebug() << "Main window is closed";
}

MainWindow::~MainWindow()
{
    // Write the config
    Config::writeConfig(config);

    delete downloader;
    delete ui;
}
