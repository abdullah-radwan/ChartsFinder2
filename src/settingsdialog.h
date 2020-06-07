#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include "configeditor.h"

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class SettingsDialog;
}
QT_END_NAMESPACE

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    SettingsDialog(QWidget *parent, ConfigEditor::Config *config);
    ~SettingsDialog();

private slots:
    void onLanguageChange();
    void on_pathButton_clicked();

    void on_resTable_cellChanged(int row, int column);
    void on_removeButton_clicked();
    void on_upButton_clicked();
    void on_downButton_clicked();
    void on_addButton_clicked();
    void on_resetButton_clicked();

    void on_SettingsDialog_finished(int result);

private:
    ConfigEditor::Config *config;
    Ui::SettingsDialog *ui;
    bool adding;

    void setLangCombo();
    int getUpdatePeriod();
    void setUpdateCombo();

    void setResources();
    void addResource(int row, ConfigEditor::Resource resource, bool selectRow = true);
};

#endif // SETTINGSDIALOG_H
