#include "adddialog.h"
#include "ui_adddialog.h"

AddDialog::AddDialog(QWidget *parent, int resourcesSize) : QDialog(parent), ui(new Ui::AddDialog)
{
    ui->setupUi(this);

    // Set the maximum number for the order spin
    ui->orderSpin->setMaximum(resourcesSize + 1);

    ui->addButton->setFocus();
}

void AddDialog::on_addButton_clicked()
{
    url = ui->urlEdit->text();

    ui->statusLabel->show();

    if (url.isEmpty()) {
        ui->statusLabel->setText(tr("The resource URL can't be empty."));
        return;
    // If it doesn't starts with http (also valid for https) or doesn't contains %1 (the ICAO code placeholder)
    } else if (!url.startsWith("http") || !url.contains("%1")) {
        ui->statusLabel->setText(tr("The resource URL format is incorrect."));
        return;
    }

    suffix = ui->suffixEdit->text();

    if (suffix.isEmpty()) {
        ui->statusLabel->setText(tr("The resource files suffix can't be empty."));
        return;
    } else if (suffix.startsWith(".")) {
        suffix.remove(0, 1); // Remove the dot
    }

    suffix = suffix.toLower();

    type = QString::number(ui->typeCombo->currentIndex());

    // If it's a folder resource and doesn't end with a forward slash
    if (type == "1" && !url.endsWith("/"))
        url.append("/");

    // -1 to convert to 'computer zero'
    order = ui->orderSpin->value() - 1;

    accept();
}

void AddDialog::on_cancelButton_clicked()
{
    reject();
}

AddDialog::~AddDialog()
{
    delete ui;
}
