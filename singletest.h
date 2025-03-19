#ifndef SINGLETEST_H
#define SINGLETEST_H

#include <QDialog>

namespace Ui {
class singletest;
}

class singletest : public QDialog
{
    Q_OBJECT

public:
    explicit singletest(QWidget *parent = nullptr);
    ~singletest();
    void setData(double &ns ,double &minVolts ,double &maxVolts);
signals:
    void setReady(double ns ,double minVolts ,double maxVolts);

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_clicked();

private:
    Ui::singletest *ui;
};

#endif // SINGLETEST_H
