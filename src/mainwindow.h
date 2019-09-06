#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "downloader.h"
#include <UpdateController>
#include <QTranslator>
#include <QLocale>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void on_getButton_clicked();

    void on_cancelButton_clicked();

    void on_actionSettings_triggered();

    void on_actionAbout_triggered();

    void on_icaoEdit_returnPressed();

    void on_actionCheck_for_updates_triggered();

private:
    Ui::MainWindow *ui;

    Downloader* downloader;

    QtAutoUpdater::UpdateController* controller;

    Config::configStruct config;

    QTranslator* qtTranslator;
    QTranslator* translator;
    QTranslator* autoUpdaterTranslator;
    QLocale locale;

    void setTranslator();
    void setDownloader();
    void checkForUpdates();

    void closeEvent(QCloseEvent *bar);
};

#endif // MAINWINDOW_H
