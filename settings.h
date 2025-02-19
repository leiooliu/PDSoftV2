#ifndef SETTINGS_H
#define SETTINGS_H

#include <QDialog>
#include <configloader.h>
#include <TimeBaseLoader.h>
#include <ps2000aApi.h>
#include <enumbinder.h>
namespace Ui {
class Settings;
}

class Settings : public QDialog
{
    Q_OBJECT

public:
    explicit Settings(QWidget *parent = nullptr);
    ~Settings();
    void SetConfig(ConfigSetting *setting,QVector<TimeBase> timeBaseList);
private slots:
    void on_buttonBox_accepted();

private:
    Ui::Settings *ui;
    ConfigSetting configSettings;
    QVector<TimeBase> timeBaseList;
    EnumBinder<PS2000A_RANGE> *binderVoltage;
    EnumBinder<enPS2000AChannel> *binderChannel;
    EnumBinder<enPS2000ACoupling> *binderCoupling;

signals:
    void setFinished();
};

#endif // SETTINGS_H
