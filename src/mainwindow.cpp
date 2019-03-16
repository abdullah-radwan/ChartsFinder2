#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "settingswindow.h"
#include <QDesktopServices>
#include <QMessageBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->config = Config::readConfig();

    downloader = new Downloader();

    connect(downloader, &Downloader::exists, [&](QString airport){
        ui->downloadLabel->show();
        ui->downloadLabel->setText(airport + " charts already exists.");
    });
    connect(downloader, &Downloader::searchingForCharts, [&](QString airport){
        ui->getButton->hide();
        ui->cancelButton->show();
        ui->downloadLabel->show();
        ui->downloadLabel->setText("Searching for " + airport + " charts...");
    });
    connect(downloader, &Downloader::downloadingCharts, [&](QString airport){
        ui->downloadLabel->setText("Downloading " + airport + " charts...");
        ui->progressBar->show();
    });
    connect(downloader, &Downloader::downloadingFolderChart, [&](QString airport, QString chart){
        ui->downloadLabel->setText("Downloading " + airport + " " + chart + " chart...");
        if(ui->progressBar->isHidden()) ui->progressBar->show();
    });
    connect(downloader, &Downloader::downloadProgress, [&](int bytesReceived, int bytesTotal){
        ui->progressBar->setMaximum(bytesTotal);
        ui->progressBar->setValue(bytesReceived);
    });
    connect(downloader, &Downloader::finished, [&](QString airport){
        ui->downloadLabel->setText(airport + " charts downloaded.");
        ui->progressBar->setValue(0);
        ui->progressBar->hide();
    });
    connect(downloader, &Downloader::notFound, [&](QString airport){
        ui->downloadLabel->setText("Couldn't find " + airport + " charts.");
        ui->progressBar->hide();
    });
    connect(downloader, &Downloader::failed, [&](QString airport){
        ui->downloadLabel->setText("Failed to download " + airport + " charts.");
        ui->progressBar->hide();
    });
    connect(downloader, &Downloader::processFinished, [&](){
        ui->progressBar->hide();
        ui->cancelButton->hide();
        ui->getButton->show();
    });
    connect(downloader, &Downloader::canceled, [&](){
        ui->downloadLabel->setText("Process canceled.");
        ui->progressBar->hide();
        ui->cancelButton->hide();
        ui->getButton->show();
    });
}

void MainWindow::on_getButton_clicked()
{
    QStringList airports = ui->icaoEdit->text().toUpper().split(" ");

    if(!QFileInfo(config.path).exists()) QDir(config.path).mkpath(".");

    downloader->download(config, airports);
}

void MainWindow::on_cancelButton_clicked() { downloader->cancel(); }

void MainWindow::on_actionSettings_triggered()
{
    SettingsWindow settingsWin(this, config);

    settingsWin.show();

    QEventLoop loop;

    connect(&settingsWin, SIGNAL(closed()), &loop, SLOT(quit()));

    loop.exec();

    config.path = settingsWin.config.path;
    config.openCharts = settingsWin.config.openCharts;
    config.removeFiles = settingsWin.config.removeFiles;
    config.resources = settingsWin.config.resources;
}

MainWindow::~MainWindow()
{
    Config::writeConfig(config);

    delete ui;
}

void MainWindow::on_actionAbout_triggered()
{
    QMessageBox::about(this, "About Charts Finder", "Charts Finder <br>"
        "A tool to get charts for your flights. <br> <br>"

        "Version: 2.0 <br>"
        "Build date: Mar 2019 <br>"
        "Check for updates on <a href='https://github.com/abdullah-radwan/ChartsFinder2/releases'>"
        "Github</a>. <br> <br>"

        "Copyright © Abdullah Radwan");
}

void MainWindow::on_actionDonate_triggered() { QDesktopServices::openUrl(QUrl("https://www.paypal.me/abdullahradwan1")); }

void MainWindow::on_icaoEdit_returnPressed() { on_getButton_clicked(); }
