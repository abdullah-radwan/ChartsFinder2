#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "downloader.h"

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

    void on_actionDonate_triggered();

    void on_icaoEdit_returnPressed();

private:
    Ui::MainWindow *ui;

    Downloader* downloader;

    Config::configStruct config;
};

#endif // MAINWINDOW_H
