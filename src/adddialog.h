#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>

namespace Ui {
class AddDialog;
}

class AddDialog : public QDialog
{
    Q_OBJECT

public:
    AddDialog(QWidget *parent, int resourcesSize);

    ~AddDialog();

    QString url, type;

    int order;

    bool added = false;

private slots:
    void on_addButton_clicked();

    void on_cancelButton_clicked();

private:
    Ui::AddDialog *ui;
};

#endif // ADDDIALOG_H
