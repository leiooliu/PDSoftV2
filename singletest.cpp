#include "singletest.h"
#include "ui_singletest.h"

singletest::singletest(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::singletest)
{
    ui->setupUi(this);
}

void singletest::setData(double &ns ,double &minVolts ,double &maxVolts){
    ui->dsb_ns->setValue(ns);
    ui->dsb_minVolts->setValue(minVolts);
    ui->dsb_maxVolts->setValue(maxVolts);
}

singletest::~singletest()
{
    delete ui;
}

//取消
void singletest::on_pushButton_2_clicked()
{
    this->close();
}

//确定
void singletest::on_pushButton_clicked()
{
    emit setReady(ui->dsb_ns->value() ,ui->dsb_minVolts->value() ,ui->dsb_maxVolts->value());
    this->close();
}

