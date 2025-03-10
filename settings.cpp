#include "settings.h"
#include "ui_settings.h"
#include <enummap.h>
Settings::Settings(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Settings)
{
    ui->setupUi(this);
    tableModel = new TimebaseModel();
}

void Settings::SetConfig(ConfigSetting *setting,QVector<TimeBase> timeBaseList){
    qDebug()<< setting->sampleCount;
    ui->autoOpenDevice->setChecked(setting->autoOpenDevice);
    ui->autoSaveRawData->setChecked(setting->autoSaveRawData);
    ui->autoSaveFolder->setText(setting->autoSaveFolder);
    ui->harmonicCalculateCount->setText(QString::number(setting->harmonicCalculateCount));
    ui->autoLoadDelay->setText(QString::number(setting->autoLoadDelay));
    ui->autoRenderFrequency->setChecked(setting->autoRenderFrequency);
    ui->autoCalculateHarmonicResult->setChecked(setting->autoCalculateHarmonicResult);
    ui->autoCalculateSingalFreq->setChecked(setting->autoCalculateSingalFreq);

    if(ui->cb_coupling->count() > 0){
        ui->cb_coupling->clear();
    }

    if(ui->cb_voltage->count() > 0){
        ui->cb_voltage->clear();
    }

    if(ui->cb_channel->count() > 0){
        ui->cb_channel->clear();
    }

    if(ui->cb_timebase->count() > 0){
        ui->cb_timebase->clear();
    }

    binderVoltage =  EnumMap::getVoltageBuilder(ui->cb_voltage);
    binderChannel = EnumMap::getChannelBuilder(ui->cb_channel);
    binderCoupling = EnumMap::getCouplingBuilder(ui->cb_coupling);

    QVector<QVector<QVariant>> results;

    for(const TimeBase &tb : timeBaseList){
        ui->cb_timebase->addItem(tb.scope);

        QString scope = tb.scope;
        int timebasevalue = tb.timebasevalue;

        results.append({
           scope,
           QString::number(timebasevalue)
        });
    }

    tableModel->setData(results);
    ui->tableView->setModel(tableModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    ui->cb_timebase->setCurrentIndex(setting->defaultTimeBase);
    ui->cb_voltage->setCurrentIndex(setting->defaultVoltage);
    ui->cb_channel->setCurrentIndex(setting->defaultChannel);
    ui->cb_coupling->setCurrentIndex(setting->defaultCoupling);

    configSettings = *setting;
}

Settings::~Settings()
{
    delete ui;
}

void Settings::on_buttonBox_accepted()
{
    configSettings.autoOpenDevice = ui->autoOpenDevice->checkState();
    configSettings.autoSaveFolder = ui->autoSaveFolder->text();
    configSettings.harmonicCalculateCount = ui->harmonicCalculateCount->text().toInt();
    configSettings.autoLoadDelay = ui->autoLoadDelay->text().toInt();
    configSettings.autoSaveRawData = ui->autoSaveRawData->checkState();
    configSettings.autoOpenDevice = ui->autoOpenDevice->checkState();
    configSettings.autoRenderFrequency = ui->autoRenderFrequency->checkState();
    configSettings.autoCalculateHarmonicResult = ui->autoCalculateHarmonicResult->checkState();
    configSettings.autoCalculateSingalFreq = ui->autoCalculateSingalFreq->checkState();

    configSettings.defaultCoupling =  binderCoupling->getCurrentEnumValue();
    configSettings.defaultChannel =  binderChannel->getCurrentEnumValue();
    configSettings.defaultVoltage =  binderVoltage->getCurrentEnumValue();
    configSettings.defaultTimeBase = ui->cb_timebase->currentIndex();

    ConfigLoader::saveConfigSettingsToJson(configSettings ,"Config.Default.json");
    if(ui->isReload->checkState())
        emit setFinished();
}

