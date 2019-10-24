#include "adddialog.h"
#include "ui_adddialog.h"

AddDialog::AddDialog(QWidget *parent, int resourcesSize) : QDialog(parent), ui(new Ui::AddDialog)
{
    ui->setupUi(this);

    // Set the maximum number for the order spin
    ui->orderSpin->setMaximum(resourcesSize + 1);
}

void AddDialog::on_addButton_clicked()
{
    // Set data
    url = ui->urlEdit->text();

    if(url.isEmpty())
    {
        ui->statusLabel->setText(tr("The resource URL can't be empty."));
        return;
      // If it doesn't starts with http (also valid for https) or doesn't contains %1 (where to add the airport code)
    } else if(!url.startsWith("http") || !url.contains("%1") || url.endsWith(".pdf"))
    {
        ui->statusLabel->setText(tr("The resource URL format is incorrect."));
        return;
    }

    // Get the suffix
    suffix = ui->suffixEdit->text();

    if(suffix.isEmpty())
    {
        ui->statusLabel->setText(tr("The resource files suffix can't be empty."));
        return;
    } else if(!suffix.startsWith(".")) suffix.insert(0, "."); // If it doesn't start with a dot

    suffix = suffix.toLower();

    // Get the type as string to add it
    type = QString::number(ui->typeCombo->currentIndex());

    // If it's a folder resource and doesn't end with a forward slash
    if(type == "1" && !url.endsWith("/")) url.append("/");

    // -1 to convert to 'computer zero'
    order = ui->orderSpin->value() - 1;

    added = true;

    close();
}

void AddDialog::on_cancelButton_clicked() { close(); }

AddDialog::~AddDialog() { delete ui; }
