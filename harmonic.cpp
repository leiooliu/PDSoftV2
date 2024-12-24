#include "harmonic.h"
#include "ui_harmonic.h"
#include "QString"
#include "QMessageBox"
#include "filemanager.h"
#include "QFileDialog.h"
#include <chrono>
harmonic::harmonic(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::harmonic)
{
    ui->setupUi(this);
    //加载时基配置
    timeBaseList = TimeBaseLoader::loadFromJson("TimeBase.json");

    // 处理数据，这里可以更新 UI 或进行其他操作
    timeChart = new RenderTimeChart(ui->timeChart);

    //绑定tableView
    tableModel = new HarmonicTableModel();

    analyzer = new FFTAnalyzer(2000);
}

harmonic::~harmonic()
{
    delete ui;
}

//开始后台采集
void harmonic::on_pushButton_clicked()
{
    // 创建 SegmentHandle 线程对象
    segmentHandle = new SegmentHandle(PS2000A_5V, PS2000A_DC, PS2000A_CHANNEL_A,
                                      ui->le_segmentcount->text().toInt(), ui->le_samplecount->text().toInt(), 1, this);

    // 连接信号和槽
    connect(segmentHandle, &SegmentHandle::progressUpdated, this, &harmonic::updateProgress);
    connect(segmentHandle, &SegmentHandle::dataReady, this, &harmonic::onDataReady);
    connect(segmentHandle, &SegmentHandle::finished, segmentHandle, &QObject::deleteLater);  // 清理资源

    // 启动线程
    segmentHandle->start();
}

void harmonic::updateProgress(int percentage)
{
    ui->pBar->setValue(percentage);  // 更新进度条
}

void harmonic::onDataReady(const QVector<QPointF> &data){
    static bool alreadyProcessed = false;
    bufferedData = data;
    if (alreadyProcessed) {
        return; // 如果已处理数据，则直接返回，防止重复绘制
    }
    alreadyProcessed = true;
    timeChart->render(bufferedData ,timeBaseList.at(17).unit);
    // 连接信号和槽
    connect(timeChart, &RenderTimeChart::renderFinished, this, &harmonic::renderTimeChartFinash);
    timeChart->start();
    alreadyProcessed = false; // 允许下次处理
}
//时域图表渲染完成
void harmonic::renderTimeChartFinash(){
    //QMessageBox::information(nullptr, "Complate", "图表渲染完成");
}

//保存数据
void harmonic::on_pushButton_4_clicked()
{
    if(bufferedData.size() > 0){
        // 获取文件保存路径
        QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.pd);;All Files (*)"));
        if (!filePath.isEmpty())
        {
            // 确保文件扩展名是.txt
            if (!filePath.endsWith(".pd", Qt::CaseInsensitive))
                filePath.append(".pd");

            bool success = FileManager::serializeToBinary(filePath ,bufferedData);
            //bool success = fileHandle.serializeToFile(filePath ,bufferedData);
            if(success){
                QMessageBox::information(nullptr, "SAVE", "文件保存成功");
            }else{
                QMessageBox::critical(this, tr("Error"), tr("Failed to open file for writing."));
            }
        }
    }else{
        QMessageBox::information(nullptr, "error", "数据缓存为空，请重新采集");
    }
}

//计算谐波数据
void harmonic::on_pushButton_5_clicked()
{
    // 记录起始时间
    // auto start = std::chrono::high_resolution_clock::now();
    double baseFreq = ui->spinBox->value();
    analyzer->analyze(&bufferedData,baseFreq);
    connect(analyzer ,&FFTAnalyzer::dataReady,this,&harmonic::harmonicRunReady);
    connect(analyzer ,&FFTAnalyzer::progressUpdated,this,&harmonic::updateProgress);
    analyzer->run();
    // // 记录结束时间
    // auto end = std::chrono::high_resolution_clock::now();
    // // 计算时间差，并转换为毫秒
    // std::chrono::duration<double, std::milli> elapsed = end - start;
    // QMessageBox::information(nullptr, "", "谐波计算时间：" + QString::number(elapsed.count()));
}

void harmonic::harmonicRunReady(const QVector<QVector<QVariant>> result){
    tableModel->setData(result);
    ui->tableView->setModel(tableModel);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->hide();
}

//上一页
void harmonic::on_pushButton_2_clicked()
{
    tableModel->previousPage();
}

//下一页
void harmonic::on_pushButton_3_clicked()
{
    tableModel->nextPage();
}

//加载数据
void harmonic::on_pushButton_7_clicked()
{
    bufferedData.clear();
    // 打开文件选择对话框
    QString fileName = QFileDialog::getOpenFileName(this, "Open PD File", "", "pd Files (*.pd)");
    if (!fileName.isEmpty())
    {
        bufferedData = FileManager::deserializeFromBinary(fileName);
        if(bufferedData.size() > 0){
            timeChart->render(bufferedData ,timeBaseList.at(17).unit);
            // 连接信号和槽
            connect(timeChart, &RenderTimeChart::renderFinished, this, &harmonic::renderTimeChartFinash);
            timeChart->start();
        }else{
            QMessageBox::critical(this, tr("Error"), tr("文件打开错误。"));
        }
    }
}

//导出表格数据到csv
void harmonic::on_pushButton_8_clicked()
{
    // 获取文件保存路径
    QString filePath = QFileDialog::getSaveFileName(this, tr("Save File"), "", tr("Text Files (*.csv);;All Files (*)"));
    if (!filePath.isEmpty())
    {
        //analyzer->exportTableToCSV()
        analyzer->exportTableToCSV(ui->tableView, filePath);
    }

}

//清空表格
void harmonic::on_pushButton_9_clicked()
{
    ui->tableView->setModel(nullptr);
}

