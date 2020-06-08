#include "settingsdialog.h"
#include "ui_settingsdialog.h"
#include "adddialog.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QDebug>

SettingsDialog::SettingsDialog(QWidget *parent, ConfigEditor::Config *config) : QDialog(parent),
    ui(new Ui::SettingsDialog)
{
    // Set config
    this->config = config;

    ui->setupUi(this);

    // Set the UI
    ui->chartsCheck->setChecked(config->openCharts);
    ui->folderCheck->setChecked(config->openFolder);
    ui->pathLabel->setText(tr("Charts folder: ") + config->chartsPath);

    setLangCombo();
    setResources();
    setUpdateCombo();

    if (config->settingsSize != QSize(-1, -1))
        resize(config->settingsSize);
    if (config->settingsPos != QPoint(-1, -1))
        move(config->settingsPos);
}

void SettingsDialog::setLangCombo()
{
    // Add the system language and format it
    ui->langCombo->addItem(tr("System language: %1").arg(QLocale::languageToString(
                                                             QLocale::system().language())));

    ui->langCombo->setItemData(0, "System");

    // Add English
    ui->langCombo->addItem("English");

    ui->langCombo->setItemData(1, "en");

    // Get the available translations from the existing translations files
    QStringList fileNames = QDir(qApp->applicationDirPath() + "/translations").entryList(
                                QStringList("chartsfinder2_*.qm"));

    for (int fileIndex = 0; fileIndex < fileNames.size(); ++fileIndex) {
        // Format the filename to get the locale
        QString locale = fileNames[fileIndex];

        locale.truncate(locale.lastIndexOf('.'));

        locale.remove(0, locale.indexOf('_') + 1);

        // Add the formatted locale
        ui->langCombo->addItem(QLocale(locale).nativeLanguageName());

        ui->langCombo->setItemData(fileIndex + 2, locale);
    }

    // Set the index to the active language
    if (config->language == "System") {
        ui->langCombo->setCurrentIndex(0);
    } else if (config->language == "en") {
        ui->langCombo->setCurrentIndex(1);
    } else {
        ui->langCombo->setCurrentText(QLocale(config->language).nativeLanguageName());
    }

    // Connect to show 'You need to restart' message
    connect(ui->langCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLanguageChange()));
}

void SettingsDialog::onLanguageChange()
{
    QMessageBox::information(this, tr("Application restart"),
                             tr("The application must be restarted to change the language."));
}

int SettingsDialog::getUpdatePeriod()
{
    // Get the update period from the current index
    switch (ui->updatesCombo->currentIndex()) {
    case 0:
        return 7;
    case 1:
        return 14;
    case 2:
        return 28;
    default:
        return 0; // Should never happen
    }
}

void SettingsDialog::setUpdateCombo()
{
    // Set the update combo based on the update period
    switch (config->updatePeriod) {
    case 7:
        ui->updatesCombo->setCurrentIndex(0);
        break;
    case 14:
        ui->updatesCombo->setCurrentIndex(1);
        break;
    case 28:
        ui->updatesCombo->setCurrentIndex(2);
        break;
    case 0:
        ui->updatesCombo->setCurrentIndex(3);
        break; // Should never happen
    }
}

void SettingsDialog::on_pathButton_clicked()
{
    QString chartsPath = QFileDialog::getExistingDirectory(this, tr("Choose the charts folder"),
                                                           config->chartsPath);

    if (chartsPath.isEmpty())
        return;

    chartsPath += "/";

    config->chartsPath = chartsPath;

    qDebug() << "Charts path was changed:" << chartsPath;

    ui->pathLabel->setText(tr("Charts folder: ") + config->chartsPath);
}

void SettingsDialog::setResources()
{
    adding = true;

    // Reset data
    ui->resTable->setRowCount(0);

    // Add data
    for (int row = 0; row < config->resources.size(); row++) {
        addResource(row, config->resources.at(row), false);
        QCoreApplication::processEvents();
    }

    // Resize columns to the URLs
    ui->resTable->resizeColumnsToContents();

    // Select the first row
    ui->resTable->selectRow(0);

    adding = false;
}

void SettingsDialog::addResource(int row, ConfigEditor::Resource resource, bool selectRow)
{
    ui->resTable->insertRow(row);

    // Add the URL
    ui->resTable->setItem(row, 0, new QTableWidgetItem(resource.url));

    ui->resTable->setItem(row, 1, new QTableWidgetItem(resource.type ? tr("Folder") : tr("Normal")));

    ui->resTable->setItem(row, 2, new QTableWidgetItem(resource.suffix));

    // If the row should be selected (useful for add resource method)
    if (selectRow) ui->resTable->selectRow(row);
}

void SettingsDialog::on_addButton_clicked()
{
    adding = true;

    // Set the dialog and run it. + 1 to show the first value as 1
    AddDialog addDialog(this, config->resources.size());

    if (addDialog.exec() == QDialog::Accepted) {
        qDebug() << "Adding resource" << addDialog.url << "type:" << addDialog.type;

        config->resources.insert(addDialog.order, { addDialog.url, addDialog.type == "1", addDialog.suffix });

        addResource(addDialog.order, { addDialog.url, addDialog.type == "1", addDialog.suffix });

        ui->statusBar->showMessage(tr("The resource %1 was added.").arg(addDialog.url), 5e3);
    }

    adding = false;
}

void SettingsDialog::on_resTable_cellChanged(int row, int column)
{
    // This method will be called even if the resources are being added
    if (adding)
        return;

    // There are changes below which will cause the method to be called
    adding = true;

    // Get the resource associated with the item
    ConfigEditor::Resource resource = config->resources.at(row);

    // Get the item which was changed
    QTableWidgetItem *item = ui->resTable->item(row, column);

    // Get the changed data
    QString data = item->text().toLower();

    // If the change is in the type, and the entered type isn't English or not 'Normal' nor 'Folder'
    if (column == 1 && !(data == "normal" || data == "folder")) {
        ui->statusBar->showMessage(
            tr("The resource type must be in English. Enter 'Normal' for normal resources,"
               " or 'Folder' for folder resources.", "Don't translate 'Normal' and 'Folder'."), 5e3);

        // Set the item to the original type
        item->setText(resource.type ? tr("Folder") :  tr("Normal"));

        adding = false;

        return;
    }

    if (column == 0) {
        if (data.isEmpty()) {
            ui->statusBar->showMessage(tr("The resource URL can't be empty."), 5e3);

            item->setText(resource.url);

            adding = false;

            return;
        } else if (!data.startsWith("http") || !data.contains("%1") || data.endsWith(".pdf")) {
            ui->statusBar->showMessage(tr("The resource URL format is incorrect."), 5e3);

            item->setText(resource.url);

            adding = false;

            return;
        }

        resource.url = data;
    } else if (column == 1) {
        // Set the data type for the resource list
        data == "normal" ? data = "0" : data = "1";

        // Set the item to the translated type
        item->setText(data == "0" ? tr("Normal") : tr("Folder"));

        resource.type = data == "1";
    } else if (column == 2) {
        if (data.isEmpty()) {
            ui->statusBar->showMessage(tr("The resource files suffix can't be empty."), 5e3);

            item->setText(resource.suffix);

            adding = false;

            return;
        } else if (!data.startsWith(".")) {
            data.insert(0, ".");

            item->setText(data);
        }

        resource.suffix = data;
    }

    config->resources.replace(row, resource);

    adding = false;
}

void SettingsDialog::on_removeButton_clicked()
{
    // If the table is empty
    if (!ui->resTable->rowCount())
        return;

    // Get the selected item
    int index = ui->resTable->selectedItems().first()->row();

    // Get the resource URL
    QString resourceUrl = config->resources.at(index).url;

    qDebug() << "Removing resource" << resourceUrl;

    ui->resTable->removeRow(index);

    config->resources.removeAt(index);

    ui->statusBar->showMessage(tr("The resource %1 was removed.").arg(resourceUrl), 5e3);
}

void SettingsDialog::on_upButton_clicked()
{
    // If the table is empty (proceeding will cause CTD)
    if (!ui->resTable->rowCount())
        return;

    // Get the selected row index
    int index = ui->resTable->selectedItems().first()->row();

    // Get the selected resource
    ConfigEditor::Resource resource = config->resources.at(index);

    // If it can be moved up
    if (index - 1 >= 0) {
        qDebug() << "Moving resource" << resource.url << "up";

        // Remove the resource
        ui->resTable->removeRow(index);

        config->resources.removeAt(index);

        // Move it up
        index--;

        // Add it again
        addResource(index, resource);

        config->resources.insert(index, resource);

        ui->statusBar->showMessage(tr("The resource %1 was moved up.").arg(resource.url), 5e3);
    }
}

void SettingsDialog::on_downButton_clicked()
{
    if (!ui->resTable->rowCount())
        return;

    int index = ui->resTable->selectedItems().first()->row();

    ConfigEditor::Resource resource = config->resources.at(index);

    // If it can be moved down
    if (index + 1 < config->resources.size()) {
        qDebug() << "Moving resource" << resource.url << "down";

        ui->resTable->removeRow(index);
        config->resources.removeAt(index);

        // Move it down
        index++;

        addResource(index, resource);
        config->resources.insert(index, resource);

        ui->statusBar->showMessage(tr("The resource %1 was moved down.").arg(resource.url), 5e3);
    }
}

void SettingsDialog::on_resetButton_clicked()
{
    // If the user really wants to reset
    if (QMessageBox::question(this, tr("Resetting the resources"),
                              tr("Are you sure you want to reset the resources?")) == QMessageBox::Yes) {
        qDebug() << "Resetting resources";

        ConfigEditor::resetResources(config);
        ConfigEditor::setFltPlan(config);

        setResources();

        ui->statusBar->showMessage(tr("The resources were reset."), 5e3);
    }
}

void SettingsDialog::on_SettingsDialog_finished(int result)
{
    Q_UNUSED(result)

    config->openCharts = ui->chartsCheck->isChecked();
    config->openFolder = ui->folderCheck->isChecked();
    config->updatePeriod = getUpdatePeriod();

    config->language = ui->langCombo->currentData().toString();
    config->settingsSize = size();
    config->settingsPos = pos();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}
