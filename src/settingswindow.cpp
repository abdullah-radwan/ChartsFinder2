#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "adddialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

SettingsWindow::SettingsWindow(QWidget *parent, Config::ConfigStruct config) : QMainWindow(parent), ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    // Set config
    this->config = config;

    if(!config.settingsWinGeo.isEmpty()) setGeometry(config.settingsWinGeo);

    // Set the UI
    ui->openChartCheck->setChecked(config.openChart);

    ui->openFolderCheck->setChecked(config.openFolder);

    ui->removeCheck->setChecked(config.removeFiles);

    ui->pathLabel->setText(tr("Charts folder: ") + config.path);

    setLangCombo();

    setResources();

    setUpdateCombo();

    show();

    qDebug() << "Settings window is shown";
}

void SettingsWindow::setLangCombo()
{
    // Add the system language and format it
    ui->langCombo->addItem(tr("System language: %1").arg(QLocale::languageToString(QLocale::system().language())));

    ui->langCombo->setItemData(0, "System");

    // Add English
    ui->langCombo->addItem("English");

    ui->langCombo->setItemData(1, "English");

    // Get the available translations from the existing translations files
    QStringList fileNames = QDir(qApp->applicationDirPath() + "/translations").entryList(QStringList("chartsfinder2_*.qm"));

    for (int fileIndex = 0; fileIndex < fileNames.size(); ++fileIndex)
    {
        // Format the filename to get the locale
        QString locale = fileNames[fileIndex];

        locale.truncate(locale.lastIndexOf('.'));

        locale.remove(0, locale.indexOf('_') + 1);

        // Add the formatted locale
        ui->langCombo->addItem(QLocale(locale).nativeLanguageName());

        ui->langCombo->setItemData(fileIndex + 2, locale);
    }

    // Set the index to the active language
    if(config.language == "System") ui->langCombo->setCurrentIndex(0);
    else ui->langCombo->setCurrentText(QLocale(config.language).nativeLanguageName());

    // Connect to show 'You need to restart' message
    connect(ui->langCombo, SIGNAL(currentIndexChanged(QString)), this, SLOT(onLanguageChange()));
}

void SettingsWindow::onLanguageChange()
{
    QMessageBox::information(this, tr("You must restart the application"), tr("You must restart the application to change the language."));
}

int SettingsWindow::getUpdatePeriod()
{
    // Get the update period from the current index
    switch (ui->updatesCombo->currentIndex())
    {
    case 0: return 3;
    case 1: return 7;
    case 2: return 14;
    default: return 0; // Never
    }
}

void SettingsWindow::setUpdateCombo()
{
    // Set the update combo based on the update period
    switch (config.updatePeriod)
    {
    case 3: ui->updatesCombo->setCurrentIndex(0); break;
    case 7: ui->updatesCombo->setCurrentIndex(1); break;
    case 14: ui->updatesCombo->setCurrentIndex(2); break;
    case 0: ui->updatesCombo->setCurrentIndex(3); break; // Never
    }
}

void SettingsWindow::on_pathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Choose the charts folder"), config.path);

    if(path.isEmpty()) return;

    path += "/";

    config.path = path;

    qDebug() << "Path changed:" << path;

    ui->pathLabel->setText(tr("Charts folder: ") + config.path);
}

void SettingsWindow::setResources()
{
    adding = true;

    // Reset data
    ui->resTable->setRowCount(0);

    // Add data
    for(int row = 0; row < config.resources.size(); row++)
    {
        addResource(row, config.resources.at(row), false);
        QCoreApplication::processEvents();
    }

    // Resize columns to the URLs
    ui->resTable->resizeColumnsToContents();

    // Select the first row
    ui->resTable->selectRow(0);

    adding = false;
}

void SettingsWindow::addResource(int row, Config::Resource resource, bool selectRow)
{
    ui->resTable->insertRow(row);

    // Add the URL
    ui->resTable->setItem(row, 0, new QTableWidgetItem(resource.url));

    ui->resTable->setItem(row, 1, new QTableWidgetItem(resource.type ? tr("Folder") : tr("Normal")));

    ui->resTable->setItem(row, 2, new QTableWidgetItem(resource.suffix));

    // If the row should be selected (useful for add resource method)
    if(selectRow) ui->resTable->selectRow(row);
}

void SettingsWindow::on_addButton_clicked()
{
    adding = true;

    // Set the dialog and run it. + 1 to show the first value as 1
    AddDialog addDialog(this, config.resources.size());

    addDialog.exec();

    if(addDialog.added)
    {
        qDebug() << "Adding resource" << addDialog.url << "type:" << addDialog.type;

        config.resources.insert(addDialog.order, { addDialog.url, addDialog.type == "1", addDialog.suffix });

        addResource(addDialog.order, { addDialog.url, addDialog.type == "1", addDialog.suffix });

        statusBar()->showMessage(tr("The resource %1 is added.").arg(addDialog.url), 5000);
    }

    adding = false;
}

void SettingsWindow::on_resTable_cellChanged(int row, int column)
{
    // This method will be called even if the resources are being added
    if(adding) return;

    // There are changes below which will cause the method to be called
    adding = true;

    // Get the resource associated with the item
    Config::Resource resource = config.resources.at(row);

    // Get the item which is changed
    QTableWidgetItem* item = ui->resTable->item(row, column);

    // Get the changed data
    QString data = item->text().toLower();

    // If the change is in the type, and the entered type isn't English or not 'Normal' nor 'Folder'
    if(column == 1 && !(data == "normal" || data == "folder"))
    {
        statusBar()->showMessage(tr("The resource type must be in English. Enter 'Normal' for normal resources,"
                                    " or 'Folder' for folder resources.", "Don't translate 'Normal' and 'Folder'."), 5000);

        // Set the item to the original type
        item->setText(resource.type ? tr("Folder") :  tr("Normal"));

        adding = false;

        return;
    }

    if(column == 0)
    {
        if(data.isEmpty())
        {
            statusBar()->showMessage(tr("The resource URL can't be empty."), 5000);

            item->setText(resource.url);

            adding = false;

            return;
        }
        else if(!data.startsWith("http") || !data.contains("%1") || data.endsWith(".pdf"))
        {
            statusBar()->showMessage(tr("The resource URL format is incorrect."), 5000);

            item->setText(resource.url);

            adding = false;

            return;
        }

        resource.url = data;
    }
    else if(column == 1)
    {
        // Set the data type for the resource list
        data == "normal" ? data = "0" : data = "1";

        // Set the item to the translated type
        item->setText(data == "0" ? tr("Normal") : tr("Folder"));

        resource.type = data == "1";
    }
    else if(column == 2)
    {
        if(data.isEmpty())
        {
           statusBar()->showMessage(tr("The resource files suffix can't be empty."), 5000);

           item->setText(resource.suffix);

           adding = false;

           return;
        }
        else if(!data.startsWith("."))
        {
            data.insert(0, ".");

            item->setText(data);
        }

        resource.suffix = data;
    }

    config.resources.replace(row, resource);

    adding = false;
}

void SettingsWindow::on_removeButton_clicked()
{
    // If the table is empty
    if(!ui->resTable->rowCount()) return;

    // Get the selected item
    int index = ui->resTable->selectedItems().first()->row();

    // Get the resource URL
    QString resourceUrl = config.resources.at(index).url;

    qDebug() << "Removing resource" << resourceUrl;

    ui->resTable->removeRow(index);

    config.resources.removeAt(index);

    statusBar()->showMessage(tr("The resource %1 is removed.").arg(resourceUrl), 5000);
}

void SettingsWindow::on_upButton_clicked()
{
    // If the table is empty (proceeding will cause CTD)
    if(!ui->resTable->rowCount()) return;

    // Get the selected row index
    int index = ui->resTable->selectedItems().first()->row();

    // Get the selected resource
    Config::Resource resource = config.resources.at(index);

    // If it can be moved up
    if(index - 1 >= 0)
    {
        qDebug() << "Moving resource" << resource.url << "up";

        // Remove the resource
        ui->resTable->removeRow(index);

        config.resources.removeAt(index);

        // Move it up
        index--;

        // Add it again
        addResource(index, resource);

        config.resources.insert(index, resource);

        statusBar()->showMessage(tr("The resource %1 is moved up.").arg(resource.url), 5000);
    }
}

void SettingsWindow::on_downButton_clicked()
{
    if(!ui->resTable->rowCount()) return;

    int index = ui->resTable->selectedItems().first()->row();

    Config::Resource resource = config.resources.at(index);

    // If it can be moved down
    if(index + 1 < config.resources.size())
    {
        qDebug() << "Moving resource" << resource.url << "down";

        ui->resTable->removeRow(index);
        config.resources.removeAt(index);

        // Move it down
        index++;

        addResource(index, resource);
        config.resources.insert(index, resource);

        statusBar()->showMessage(tr("The resource %1 is moved down.").arg(resource.url), 5000);
    }
}

void SettingsWindow::on_resetButton_clicked()
{
    // If the user really wants to reset
    if(QMessageBox::question(this, tr("Resetting the resources"), tr("Are you sure you want to reset the resources?")) == QMessageBox::Yes)
    {
        qDebug() << "Resetting resources";

        config.resources.clear();

        config.resources.append({"http://imageserver.fltplan.com/merge/merge191107/%1", false, ".pdf"});
        config.resources.append({"http://www.sia-enna.dz/PDF/AIP/AD/AD2/%1/", true, ".pdf"});
        config.resources.append({"http://caa.gov.ly/ais/wp-content/uploads/2017/AIP/AD/%1", false, ".pdf"});
        config.resources.append({"http://www.caiga.ru/common/AirInter/validaip/aip/ad/ad2/rus/%1/", true, ".pdf"});
        config.resources.append({"http://kan.kg/ais/eaip/aipcharts/%1/", true, ".pdf"});
        config.resources.append({"https://www.ais.gov.mm/eAIP/2019-10-24-AIRAC/graphics/eAIP/AD/%1/", true, ".pdf"});
        config.resources.append({"http://www.aip.net.nz/pdf/%1", false, ".pdf"});
        config.resources.append({"http://vau.aero/navdb/chart/%1", false, ".pdf"});
        config.resources.append({"http://sa-ivao.net/charts_file/%1", false, ".pdf"});
        config.resources.append({"http://europlanet.de/vaFsP/charts/%1", false, ".pdf"});
        config.resources.append({"http://www.fly-sea.com/charts/%1", false, ".pdf"});
        config.resources.append({"http://uvairlines.com/admin/resources/charts/%1", false, ".pdf"});
        config.resources.append({"https://www.virtualairlines.eu/charts/%1", false, ".pdf"});

        setResources();

        statusBar()->showMessage(tr("The resources are reset."), 5000);
    }
}

void SettingsWindow::closeEvent(QCloseEvent *bar)
{
    // Set the config data
    config.settingsWinGeo = geometry();

    config.openChart = ui->openChartCheck->isChecked();

    config.openFolder = ui->openFolderCheck->isChecked();

    config.removeFiles = ui->removeCheck->isChecked();

    config.updatePeriod = getUpdatePeriod();

    config.language = ui->langCombo->currentData().toString();

    bar->accept();

    emit closed();

    qDebug() << "Settings window is closed";
}

SettingsWindow::~SettingsWindow() { delete ui; }
