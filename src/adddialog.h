#ifndef ADDDIALOG_H
#define ADDDIALOG_H

#include <QDialog>

QT_BEGIN_NAMESPACE
namespace Ui {
class AddDialog;
}
QT_END_NAMESPACE

class AddDialog : public QDialog
{
    Q_OBJECT

public:
    AddDialog(QWidget *parent, int resourcesSize);
    ~AddDialog();

    QString url, type, suffix;
    int order;

private slots:
    void on_addButton_clicked();
    void on_cancelButton_clicked();

private:
    Ui::AddDialog *ui;
};

#endif // ADDDIALOG_H
