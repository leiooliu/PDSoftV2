#ifndef ENUMBINDER_H
#define ENUMBINDER_H

#include <QComboBox>
#include <QMap>
#include <QVariant>

template <typename EnumType>
class EnumBinder
{
public:
    EnumBinder(QComboBox *comboBox, const QMap<EnumType, QString> &enumMap);
    EnumType getCurrentEnumValue() const;
    EnumType getEnumValueAtIndex(int index) const;
    void setCurrentEnumValue(EnumType value);
private:
    QComboBox *comboBox;
};

template <typename EnumType>
EnumBinder<EnumType>::EnumBinder(QComboBox *comboBox, const QMap<EnumType, QString> &enumMap)
    : comboBox(comboBox)
{
    for (auto it = enumMap.begin(); it != enumMap.end(); ++it) {
        comboBox->addItem(it.value(), QVariant::fromValue(it.key()));
    }
}

template <typename EnumType>
EnumType EnumBinder<EnumType>::getCurrentEnumValue() const
{
    return static_cast<EnumType>(comboBox->currentData().toInt());
}

template <typename EnumType>
void EnumBinder<EnumType>::setCurrentEnumValue(EnumType value)
{
    int index = comboBox->findData(QVariant::fromValue(value));
    if (index != -1) {
        comboBox->setCurrentIndex(index);
    }
}

template <typename EnumType>
EnumType EnumBinder<EnumType>::getEnumValueAtIndex(int index) const
{
    if (index >= 0 && index < comboBox->count()) {
        return static_cast<EnumType>(comboBox->itemData(index).toInt());
    }
    // 处理非法索引的情况，可以抛出异常或返回一个默认值，视具体需求而定
    throw std::out_of_range("Index out of range");
}

#endif // ENUMBINDER_H
