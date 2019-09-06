#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "adddialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>
#include <QDebug>

SettingsWindow::SettingsWindow(QWidget *parent, Config::configStruct config) : QMainWindow(parent), ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    // Set config
    this->config = config;

    if(!config.settingsWinGeo.isEmpty()) setGeometry(config.settingsWinGeo);

    // Set the UI
    ui->openCheck->setChecked(config.openCharts);

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

        ui->langCombo->setItemData(ui->langCombo->count() - 1, locale);
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
    default: return 3; // To avoid compiler warning
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
    }
}

void SettingsWindow::on_pathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, tr("Select charts folder"));

    if(path.isEmpty()) return;

    // Add slash if no slash is added
    if(!path.endsWith("/")) path += "/";

    config.path = path;

    qDebug() << "Path changed:" << path;

    ui->pathLabel->setText(tr("Charts folder: ") + config.path);
}

void SettingsWindow::setResources()
{
    // Reset data
    ui->resTable->setRowCount(0);

    // Add data
    for(int row = 0; row < config.resources.size(); row++)
    {
        addResource(row, config.resources.at(row).first(), config.resources.at(row).last(), false);
        QCoreApplication::processEvents();
    }

    // Resize columns to the URLs
    ui->resTable->resizeColumnsToContents();

    // Select the first row
    ui->resTable->selectRow(0);
}

void SettingsWindow::addResource(int row, QString url, QString type, bool selectRow)
{
    ui->resTable->insertRow(row);

    // Add the URL
    ui->resTable->setItem(row, 0, new QTableWidgetItem(url));

    // If it's a normal resource
    if(type == "0") ui->resTable->setItem(row, 1, new QTableWidgetItem(tr("Normal")));
    else ui->resTable->setItem(row, 1, new QTableWidgetItem(tr("Folder")));

    // If the row should be selected (useful for add resource method)
    if(selectRow) ui->resTable->selectRow(row);
}

void SettingsWindow::on_addButton_clicked()
{
    // Set the dialog and run it. + 1 to show the first value as 1
    AddDialog addDialog(this, config.resources.size() + 1);

    addDialog.exec();

    if(addDialog.added)
    {
        qDebug() << "Adding resource" << addDialog.url << "type:" << addDialog.type;

        // If it starts with http (also valid for https) and contains %1 (where to add the airport code), add it
        if(addDialog.url.startsWith("http") && addDialog.url.contains("%1"))
        {
            config.resources.insert(addDialog.order, {addDialog.url, addDialog.type});

            addResource(addDialog.order, addDialog.url, addDialog.type);

            statusBar()->showMessage(tr("Resource %1 was added.").arg(addDialog.url));
        } else statusBar()->showMessage(tr("Couldn't add the resource. The syntax isn't correct."));
    }
}

void SettingsWindow::on_removeButton_clicked()
{
    // If the table is empty
    if(!ui->resTable->rowCount()) return;

    // Get the selected item
    int index = ui->resTable->selectedItems().first()->row();

    // Get the resource URL
    QString resourceUrl = config.resources.at(index).first();

    qDebug() << "Removing resource" << resourceUrl;

    ui->resTable->removeRow(index);

    config.resources.removeAt(index);

    statusBar()->showMessage(tr("Resource %1 was removed.").arg(resourceUrl));
}

void SettingsWindow::on_upButton_clicked()
{
    // If the table is empty (proceeding will cause CTD)
    if(!ui->resTable->rowCount()) return;

    // Get the selected row index
    int index = ui->resTable->selectedItems().first()->row();

    // Get the selected resource
    QStringList resource = config.resources.at(index);

    // If it can be moved up
    if(index - 1 >= 0)
    {
        qDebug() << "Moving resource" << resource << "up";

        // Remove the resource
        ui->resTable->removeRow(index);

        config.resources.removeAt(index);

        // Move it up
        index--;

        // Add it again
        addResource(index, resource.first(), resource.last());

        config.resources.insert(index, resource);

        statusBar()->showMessage(tr("Resource %1 was moved up.").arg(resource.first()));
    }
}

void SettingsWindow::on_downButton_clicked()
{
    if(!ui->resTable->rowCount()) return;

    int index = ui->resTable->selectedItems().first()->row();

    QStringList resource = config.resources.at(index);

    // If it can be moved down
    if(index + 1 < config.resources.size())
    {
        qDebug() << "Moving resource" << resource << "down";

        ui->resTable->removeRow(index);
        config.resources.removeAt(index);

        // Move it down
        index++;

        addResource(index, resource.first(), resource.last());
        config.resources.insert(index, resource);

        statusBar()->showMessage(tr("Resource %1 was moved down.").arg(resource.first()));
    }
}

void SettingsWindow::on_resetButton_clicked()
{
    // If the user really wants to reset
    if(QMessageBox::question(this, tr("Resetting resources"), tr("Are you sure you want to reset resources?")) == QMessageBox::Yes)
    {
        qDebug() << "Resetting resources";

        config.resources.clear();

        config.resources.append({"http://imageserver.fltplan.com/merge/merge190718/%1.pdf", "0"});
        config.resources.append({"http://www.sia-enna.dz/PDF/AIP/AD/AD2/%1/", "1"});
        config.resources.append({"http://caa.gov.ly/ais/wp-content/uploads/2017/AIP/AD/%1.pdf", "0"});
        config.resources.append({"http://www.caiga.ru/common/AirInter/validaip/aip/ad/ad2/rus/%1/", "1"});
        config.resources.append({"http://vau.aero/navdb/chart/%1.pdf", "0"});
        config.resources.append({"http://sa-ivao.net/charts_file/%1.pdf", "0"});
        config.resources.append({"http://europlanet.de/vaFsP/charts/%1.pdf", "0"});
        config.resources.append({"http://www.fly-sea.com/charts/%1.pdf", "0"});
        config.resources.append({"http://uvairlines.com/admin/resources/charts/%1.pdf", "0"});
        config.resources.append({"https://www.virtualairlines.eu/charts/%1.pdf", "0"});

        setResources();

        statusBar()->showMessage(tr("Resources were reset."));
    }
}

void SettingsWindow::closeEvent(QCloseEvent *bar)
{
    // Set the config data
    config.settingsWinGeo = geometry();

    config.openCharts = ui->openCheck->isChecked();

    config.removeFiles = ui->removeCheck->isChecked();

    config.updatePeriod = getUpdatePeriod();

    config.language = ui->langCombo->currentData().toString();

    bar->accept();

    emit closed();

    qDebug() << "Settings window is closed";
}

SettingsWindow::~SettingsWindow() { delete ui; }
