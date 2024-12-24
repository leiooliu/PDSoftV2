#ifndef HARMONICTABLEMODEL_H
#define HARMONICTABLEMODEL_H

#include <QAbstractTableModel>
#include <QVector>
#include <QVariant>

class HarmonicTableModel:public QAbstractTableModel
{
    Q_OBJECT
public:
    explicit HarmonicTableModel(QObject* parent = nullptr);

    // 设置全部数据
    void setData(const QVector<QVector<QVariant>>& data);

    // 分页相关接口
    int totalRowCount() const;         // 总数据行数
    int pageCount() const;             // 总页数
    int currentPage() const;           // 当前页码
    int rowsPerPage() const;           // 每页显示的行数

    void setRowsPerPage(int rows);     // 设置每页显示行数
    void goToPage(int page);           // 跳转到指定页
    void nextPage();                   // 下一页
    void previousPage();               // 上一页

    // 重写虚函数
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QVariant headerData(int section, Qt::Orientation orientation, int role = Qt::DisplayRole) const override;

    // 更新数据
    void updateData(const QVector<QVariant>& newRowData, int row);
    void insertRow(const QVector<QVariant>& newRowData);
    void removeRow(int row);

private:
    QVector<QVector<QVariant>> tableData; // 原始全部数据

    // 分页相关变量
    int m_rowsPerPage = 100;      // 每页显示行数，默认为10行
    int m_currentPage = 0;       // 当前页码，从0开始
};

#endif // HARMONICTABLEMODEL_H
