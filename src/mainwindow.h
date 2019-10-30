#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "downloader.h"
#include <UpdateController>
#include <QTranslator>
#include <QLocale>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);

    ~MainWindow();

private slots:
    void on_getButton_clicked();

    void on_actionSettings_triggered();

    void on_actionAbout_triggered();

    void on_icaoEdit_returnPressed();

    void on_actionCheck_for_updates_triggered();

private:
    Ui::MainWindow *ui;

    Downloader* downloader;

    QtAutoUpdater::Updater* updater;

    Config::ConfigStruct config;

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
