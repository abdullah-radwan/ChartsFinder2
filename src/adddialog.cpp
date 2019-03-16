#include "adddialog.h"
#include "ui_adddialog.h"

AddDialog::AddDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddDialog)
{
    ui->setupUi(this);
}

AddDialog::AddDialog(QWidget *parent, int resourcesSize) :
    QDialog(parent),
    ui(new Ui::AddDialog)
{
    ui->setupUi(this);
    ui->orderSpin->setMaximum(resourcesSize);
}

void AddDialog::on_addButton_clicked()
{
    url = ui->urlEdit->text();
    type = ui->typeCombo->currentText();
    if(type == "Folder" && !url.endsWith("/")) type.append("/");
    order = ui->orderSpin->value() - 1;
    isAdded = true;

    close();
}

void AddDialog::on_cancelButton_clicked() { close(); }

AddDialog::~AddDialog() { delete ui; }
