#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QMainWindow>
#include "config.h"

namespace Ui {
class SettingsWindow;
}

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent, Config::configStruct config);

    ~SettingsWindow();

    Config::configStruct config;

signals:
    void closed();

private slots:
    void on_pathButton_clicked();

    void on_removeButton_clicked();

    void on_upButton_clicked();

    void on_downButton_clicked();

    void on_addButton_clicked();

    void on_resetButton_clicked();

    void onLanguageChange();

private:
    void closeEvent(QCloseEvent *bar);

    void setResources();

    void addResource(int row, QString url, QString type, bool selectRow = true);

    void setUpdateCombo();

    void setLangCombo();

    int getUpdatePeriod();

    Ui::SettingsWindow *ui;
};

#endif // SETTINGSWINDOW_H
