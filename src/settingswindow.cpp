#include "settingswindow.h"
#include "ui_settingswindow.h"
#include "adddialog.h"
#include <QFileDialog>
#include <QMessageBox>
#include <QCloseEvent>

SettingsWindow::SettingsWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);
}

SettingsWindow::SettingsWindow(QWidget *parent, Config::configStruct config) :
    QMainWindow(parent),
    ui(new Ui::SettingsWindow)
{
    ui->setupUi(this);

    this->config = config;
}

void SettingsWindow::showEvent(QShowEvent *ev)
{
    QMainWindow::showEvent(ev);

    ui->openCheck->setChecked(config.openCharts);

    ui->removeCheck->setChecked(config.removeFiles);

    ui->pathLabel->setText("Charts folder: " + config.path);

    setResources();
}

void SettingsWindow::on_pathButton_clicked()
{
    QString path = QFileDialog::getExistingDirectory(this, "Select charts folder");

    if(!path.isEmpty())
    {
        config.path = path + "/";
        ui->pathLabel->setText("Charts folder: " + config.path);
    }
}

void SettingsWindow::setResources()
{
    ui->resTable->reset();

    ui->resTable->setRowCount(config.resources.size());

    for(int counter = 0; counter < config.resources.size(); counter++)
    {
        ui->resTable->setItem(counter, 0, new QTableWidgetItem(config.resources.at(counter).first()));
        ui->resTable->setItem(counter, 1, new QTableWidgetItem(config.resources.at(counter).last()));

        QCoreApplication::processEvents();
    }

    ui->resTable->resizeColumnsToContents();
}

void SettingsWindow::on_addButton_clicked()
{
    AddDialog addDialog(this, config.resources.size() + 1);

    addDialog.exec();

    if(addDialog.isAdded)
    {
        if(addDialog.url.startsWith("http") && addDialog.url.contains("%1"))
        {
            config.resources.insert(addDialog.order, {addDialog.url, addDialog.type});
            setResources();

            statusBar()->showMessage("Resource added");
        } else statusBar()->showMessage("Can't add the resource. Syntax isn't correct.");
    }
}

void SettingsWindow::on_removeButton_clicked()
{
    int index = ui->resTable->row(ui->resTable->selectedItems().first());

    ui->resTable->removeRow(index);
    config.resources.removeAt(index);

    statusBar()->showMessage("Resource " + config.resources.at(index).first() + " was removed.");
}

void SettingsWindow::on_upButton_clicked()
{
    int index = ui->resTable->row(ui->resTable->selectedItems().first());

    QStringList resource = config.resources.at(index);

    if(index - 1 >= 0)
    {
        config.resources.removeAt(index);

        index--;

        config.resources.insert(index, resource);

        setResources();

        statusBar()->showMessage("Resource " + config.resources.at(index).first() + " was moved up.");
     }
}

void SettingsWindow::on_downButton_clicked()
{
    int index = ui->resTable->row(ui->resTable->selectedItems().first());

    QStringList resource = config.resources.at(index);

    if(index + 1 < config.resources.size())
    {
        config.resources.removeAt(index);

        index++;

        config.resources.insert(index, resource);

        setResources();

        statusBar()->showMessage("Resource " + config.resources.at(index).first() + " was moved down.");
     }
}

void SettingsWindow::on_resetButton_clicked()
{
    if(QMessageBox::question(this, "Reset resources", "Are you sure you want to reset resources?") == QMessageBox::Yes)
    {
        config.resources.clear();

        config.resources.append({"http://imageserver.fltplan.com/merge/merge190228/%1.pdf","Normal"});
        config.resources.append({"http://www.sia-enna.dz/PDF/AIP/AD/AD2/%1/","Folder"});
        config.resources.append({"http://www.caiga.ru/common/AirInter/validaip/aip/ad/ad2/rus/%1/","Folder"});
        config.resources.append({"http://vau.aero/navdb/chart/%1.pdf","Normal"});
        config.resources.append({"http://sa-ivao.net/charts_file/%1.pdf","Normal"});
        config.resources.append({"http://europlanet.de/vaFsP/charts/%1.pdf","Normal"});
        config.resources.append({"http://www.fly-sea.com/charts/%1.pdf","Normal"});
        config.resources.append({"http://uvairlines.com/admin/resources/charts/%1.pdf","Normal"});
        config.resources.append({"https://www.virtualairlines.eu/charts/%1.pdf","Normal"});

        setResources();

        statusBar()->showMessage("Resources were reset.");
    }
}

void SettingsWindow::closeEvent(QCloseEvent *bar)
{
    config.openCharts = ui->openCheck->isChecked();
    config.removeFiles = ui->removeCheck->isChecked();

    emit closed();

    bar->accept();
}

SettingsWindow::~SettingsWindow() { delete ui; }
