#include "timebasemodel.h"

TimebaseModel::TimebaseModel(QObject* parent)
    : QAbstractTableModel(parent) {}

void TimebaseModel::setData(const QVector<QVector<QVariant>>& data) {
    // 禁止发出不必要的信号
    blockSignals(true);

    beginResetModel(); // 通知视图即将更新
    tableData = data;  // 设置新数据
    endResetModel();   // 通知视图数据已更新

    // 恢复信号发送
    blockSignals(false);
}

int TimebaseModel::rowCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);

    // 当前页的实际行数
    int start = m_currentPage * m_rowsPerPage;
    int end = qMin(start + m_rowsPerPage, tableData.size());
    return end - start;
}

int TimebaseModel::columnCount(const QModelIndex& parent) const {
    Q_UNUSED(parent);
    return tableData.isEmpty() ? 0 : tableData[0].size(); // 列数等于内层 QVector 的大小
}

QVariant TimebaseModel::data(const QModelIndex& index, int role) const {
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

QVariant TimebaseModel::headerData(int section, Qt::Orientation orientation, int role) const {
    if (role != Qt::DisplayRole) {
        return QVariant();
    }

    if (orientation == Qt::Horizontal) {
        // 设置列头
        switch (section) {
        case 0: return "示波器";
        case 1: return "时基参数";
        default: return QVariant();
        }
    } else if (orientation == Qt::Vertical) {
        // 设置行号
        return section + 1; // 行号从1开始
    }

    return QVariant();
}

void TimebaseModel::updateData(const QVector<QVariant>& newRowData, int row) {
    if (row >= 0 && row < tableData.size()) {
        tableData[row] = newRowData;
        // 更新特定行的数据
        QModelIndex topLeft = index(row, 0);
        QModelIndex bottomRight = index(row, columnCount() - 1);
        emit dataChanged(topLeft, bottomRight);
    }
}

void TimebaseModel::insertRow(const QVector<QVariant>& newRowData) {
    beginInsertRows(QModelIndex(), tableData.size(), tableData.size());
    tableData.append(newRowData);
    endInsertRows();
}

void TimebaseModel::removeRow(int row) {
    if (row >= 0 && row < tableData.size()) {
        beginRemoveRows(QModelIndex(), row, row);
        tableData.removeAt(row);
        endRemoveRows();
    }
}

int TimebaseModel::totalRowCount() const {
    return tableData.size();
}

int TimebaseModel::pageCount() const {
    return (totalRowCount() + m_rowsPerPage - 1) / m_rowsPerPage; // 向上取整
}

int TimebaseModel::currentPage() const {
    return m_currentPage;
}

int TimebaseModel::rowsPerPage() const {
    return m_rowsPerPage;
}

void TimebaseModel::setRowsPerPage(int rows) {
    m_rowsPerPage = rows;
    m_currentPage = 0;  // 重置到第一页
    emit layoutChanged(); // 通知视图刷新
}

void TimebaseModel::goToPage(int page) {
    if (page >= 0 && page < pageCount()) {
        m_currentPage = page;
        emit layoutChanged(); // 通知视图刷新
    }
}

void TimebaseModel::nextPage() {
    if (m_currentPage + 1 < pageCount()) {
        ++m_currentPage;
        emit layoutChanged();
    }
}

void TimebaseModel::previousPage() {
    if (m_currentPage > 0) {
        --m_currentPage;
        emit layoutChanged();
    }
}
