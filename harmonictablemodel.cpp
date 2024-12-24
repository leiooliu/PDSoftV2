#include "harmonictablemodel.h"
#include <QDebug>

HarmonicTableModel::HarmonicTableModel(QObject* parent)
    : QAbstractTableModel(parent) {}

void HarmonicTableModel::setData(const QVector<QVector<QVariant>>& data) {
    // 禁止发出不必要的信号
    blockSignals(true);

    beginResetModel(); // 通知视图即将更新
    tableData = data;  // 设置新数据
    endResetModel();   // 通知视图数据已更新

    // 恢复信号发送
    blockSignals(false);
}

int HarmonicTableModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);

    // 当前页的实际行数
    int start = m_currentPage * m_rowsPerPage;
    int end = qMin(start + m_rowsPerPage, tableData.size());
    return end - start;
}

int HarmonicTableModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return tableData.isEmpty() ? 0 : tableData[0].size(); // 列数等于内层 QVector 的大小
}

QVariant HarmonicTableModel::data(const QModelIndex& index, int role) const {
    if (!index.isValid()) {
        return QVariant();
    }

    int start = m_currentPage * m_rowsPerPage; // 当前页起始行号
    int realRow = start + index.row();        // 映射到真实行号

    if (realRow >= tableData.size()) {
        return QVariant(); // 超出范围返回无效值
    }

    if (role == Qt::DisplayRole) {
        return tableData[realRow][index.column()];
    } else if (role == Qt::TextAlignmentRole) {
        return Qt::AlignCenter; // 默认居中对齐
    }
    return QVariant();
}

QVariant HarmonicTableModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        // 设置列头
        switch (section) {
        case 0: return "谐波阶数";
        case 1: return "谐波频率(Hz)";
        case 2: return "谐波能量";
        case 3: return "频率占比(%)";
        default: return QVariant();
        }
    } else if (orientation == Qt::Vertical) {
        // 设置行号
        return section + 1; // 行号从1开始
    }

    return QVariant();
}

void HarmonicTableModel::updateData(const QVector<QVariant>& newRowData, int row) {
    if (row >= 0 && row < tableData.size()) {
        tableData[row] = newRowData;
        // 更新特定行的数据
        QModelIndex topLeft = index(row, 0);
        QModelIndex bottomRight = index(row, columnCount() - 1);
        emit dataChanged(topLeft, bottomRight);
    }
}

void HarmonicTableModel::insertRow(const QVector<QVariant>& newRowData) {
    beginInsertRows(QModelIndex(), tableData.size(), tableData.size());
    tableData.append(newRowData);
    endInsertRows();
}

void HarmonicTableModel::removeRow(int row) {
    if (row >= 0 && row < tableData.size()) {
        beginRemoveRows(QModelIndex(), row, row);
        tableData.removeAt(row);
        endRemoveRows();
    }
}

int HarmonicTableModel::totalRowCount() const {
    return tableData.size();
}

int HarmonicTableModel::pageCount() const {
    return (totalRowCount() + m_rowsPerPage - 1) / m_rowsPerPage; // 向上取整
}

int HarmonicTableModel::currentPage() const {
    return m_currentPage;
}

int HarmonicTableModel::rowsPerPage() const {
    return m_rowsPerPage;
}

void HarmonicTableModel::setRowsPerPage(int rows) {
    m_rowsPerPage = rows;
    m_currentPage = 0;  // 重置到第一页
    emit layoutChanged(); // 通知视图刷新
}

void HarmonicTableModel::goToPage(int page) {
    if (page >= 0 && page < pageCount()) {
        m_currentPage = page;
        emit layoutChanged(); // 通知视图刷新
    }
}

void HarmonicTableModel::nextPage() {
    if (m_currentPage + 1 < pageCount()) {
        ++m_currentPage;
        emit layoutChanged();
    }
}

void HarmonicTableModel::previousPage() {
    if (m_currentPage > 0) {
        --m_currentPage;
        emit layoutChanged();
    }
}
