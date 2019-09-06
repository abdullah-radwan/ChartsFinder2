#include "adddialog.h"
#include "ui_adddialog.h"

AddDialog::AddDialog(QWidget *parent, int resourcesSize) : QDialog(parent), ui(new Ui::AddDialog)
{
    ui->setupUi(this);

    // Set the maximum number for the order spin
    ui->orderSpin->setMaximum(resourcesSize);
}

void AddDialog::on_addButton_clicked()
{
    // Set data
    url = ui->urlEdit->text();

    // Get the type as string to add it
    type =  QString::number(ui->typeCombo->currentIndex());

    // If it's a folder resource and doesn't int with a forward slash
    if(type == "1" && !url.endsWith("/")) url.append("/");

    // -1 to convert to 'computer zero'
    order = ui->orderSpin->value() - 1;

    added = true;

    close();
}

void AddDialog::on_cancelButton_clicked() { close(); }

AddDialog::~AddDialog() { delete ui; }
