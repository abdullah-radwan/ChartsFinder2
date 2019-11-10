#ifndef SETTINGSWINDOW_H
#define SETTINGSWINDOW_H

#include <QMainWindow>
#include "config.h"

QT_BEGIN_NAMESPACE
namespace Ui { class SettingsWindow; }
QT_END_NAMESPACE

class SettingsWindow : public QMainWindow
{
    Q_OBJECT

public:
    SettingsWindow(QWidget *parent, Config::ConfigStruct config);

    ~SettingsWindow();

    Config::ConfigStruct config;

signals:
    void closed();

private slots:
    void on_pathButton_clicked();

    void on_resTable_cellChanged(int row, int column);

    void on_removeButton_clicked();

    void on_upButton_clicked();

    void on_downButton_clicked();

    void on_addButton_clicked();

    void on_resetButton_clicked();

    void onLanguageChange();

private:
    void closeEvent(QCloseEvent *bar);

    void setResources();

    void addResource(int row, Config::Resource resource, bool selectRow = true);

    void setUpdateCombo();

    void setLangCombo();

    int getUpdatePeriod();

    Ui::SettingsWindow *ui;

    bool adding;
};

#endif // SETTINGSWINDOW_H
