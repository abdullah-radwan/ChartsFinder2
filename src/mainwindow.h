#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include "settingsdialog.h"
#include "downloader.h"

#include <QMainWindow>
#include <QtAutoUpdaterWidgets>
#include <QThread>

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(ConfigEditor::Config *config);
    ~MainWindow();

signals:
    void download(QStringList airports);

private slots:
    void on_getButton_clicked();
    void on_airportsEdit_returnPressed();

    void on_actionSettings_triggered();
    void on_actionCheck_triggered();
    void on_actionAbout_triggered();

    void onChartsExists(QString airport);
    void onSearchingCharts(QString airport);
    void onDownloadingCharts(QString airport);
    void onDownloadingFolderCharts(QString airport, QString chart);
    void onDownloadProgress(double received, double total);
    void onDownloadFinished(QString airport);
    void onProcessFinished();

    void onChartsNotFound(QString airport);
    void onDownloadFailed(QString airport, QString error);
    void onProcessCanceled();

private:
    ConfigEditor::Config *config;
    Ui::MainWindow *ui;
    QtAutoUpdater::Updater *updater;
    SettingsDialog settingsDialog;

    Downloader *downloader;
    QThread downloaderThread;

    void setDownloader();
    void setUpdater();
    void checkUpdates();

    void closeEvent(QCloseEvent *bar) override;
};

#endif // MAINWINDOW_H
