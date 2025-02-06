/********************************************************************************
** Form generated from reading UI file 'mainwindow.ui'
**
** Created by: Qt User Interface Compiler version 6.6.3
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_MAINWINDOW_H
#define UI_MAINWINDOW_H

#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtWidgets/QApplication>
#include <QtWidgets/QComboBox>
#include <QtWidgets/QGroupBox>
#include <QtWidgets/QHeaderView>
#include <QtWidgets/QLabel>
#include <QtWidgets/QListView>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QMenu>
#include <QtWidgets/QMenuBar>
#include <QtWidgets/QProgressBar>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSpinBox>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QTabWidget>
#include <QtWidgets/QTableView>
#include <QtWidgets/QTextEdit>
#include <QtWidgets/QWidget>
#include <pdchart.h>

QT_BEGIN_NAMESPACE

class Ui_MainWindow
{
public:
    QAction *actionopen;
    QAction *actionSave;
    QWidget *centralwidget;
    QGroupBox *groupBox;
    QSpinBox *sb_timebase;
    QComboBox *cb_Timebase;
    QGroupBox *groupBox_2;
    QComboBox *cb_Couping;
    QComboBox *cb_Voltage;
    QComboBox *cb_Channel;
    QLabel *label;
    QLabel *label_2;
    QLabel *label_3;
    QGroupBox *groupBox_3;
    QPushButton *pushButton_9;
    QPushButton *pushButton_8;
    QPushButton *pushButton_13;
    QPushButton *pushButton_15;
    QGroupBox *groupBox_4;
    QPushButton *pushButton_3;
    QPushButton *pushButton;
    QPushButton *pushButton_2;
    QGroupBox *groupBox_5;
    QPushButton *pushButton_5;
    QPushButton *pushButton_4;
    QPushButton *pushButton_7;
    QPushButton *pushButton_10;
    QPushButton *pushButton_11;
    QPushButton *pushButton_12;
    QGroupBox *groupBox_6;
    QTextEdit *textEdit;
    QTabWidget *tabWidget;
    QWidget *tab;
    QTableView *tableView2;
    QListView *listView;
    QLabel *label_4;
    QPushButton *pushButton_6;
    QSpinBox *harmonicParam;
    QLabel *label_7;
    QLabel *label_8;
    QLabel *lbl_CacheCount;
    QProgressBar *progressBar;
    QGroupBox *groupBox_7;
    QLabel *label_5;
    QLabel *label_6;
    QLabel *lbl_fvalues;
    QLabel *lbl_vvalues;
    QTabWidget *tabWidget_2;
    QWidget *tab_3;
    PDChart *chartView;
    QWidget *tab_4;
    PDChart *chartView_2;
    QWidget *tab_2;
    PDChart *chartView_3;
    QWidget *tab_5;
    PDChart *chartView_4;
    QPushButton *pushButton_14;
    QMenuBar *menubar;
    QMenu *menu;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *MainWindow)
    {
        if (MainWindow->objectName().isEmpty())
            MainWindow->setObjectName("MainWindow");
        MainWindow->resize(1567, 850);
        MainWindow->setLayoutDirection(Qt::LayoutDirection::LeftToRight);
        MainWindow->setTabShape(QTabWidget::TabShape::Rounded);
        actionopen = new QAction(MainWindow);
        actionopen->setObjectName("actionopen");
        actionSave = new QAction(MainWindow);
        actionSave->setObjectName("actionSave");
        centralwidget = new QWidget(MainWindow);
        centralwidget->setObjectName("centralwidget");
        groupBox = new QGroupBox(centralwidget);
        groupBox->setObjectName("groupBox");
        groupBox->setGeometry(QRect(11, 11, 179, 50));
        QSizePolicy sizePolicy(QSizePolicy::Policy::Preferred, QSizePolicy::Policy::Expanding);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(groupBox->sizePolicy().hasHeightForWidth());
        groupBox->setSizePolicy(sizePolicy);
        groupBox->setMaximumSize(QSize(16777215, 50));
        sb_timebase = new QSpinBox(groupBox);
        sb_timebase->setObjectName("sb_timebase");
        sb_timebase->setGeometry(QRect(10, 20, 61, 21));
        sb_timebase->setMinimum(1);
        sb_timebase->setMaximum(5000);
        sb_timebase->setValue(1);
        cb_Timebase = new QComboBox(groupBox);
        cb_Timebase->setObjectName("cb_Timebase");
        cb_Timebase->setGeometry(QRect(72, 20, 101, 21));
        groupBox_2 = new QGroupBox(centralwidget);
        groupBox_2->setObjectName("groupBox_2");
        groupBox_2->setGeometry(QRect(11, 145, 179, 110));
        groupBox_2->setMaximumSize(QSize(180, 110));
        cb_Couping = new QComboBox(groupBox_2);
        cb_Couping->setObjectName("cb_Couping");
        cb_Couping->setGeometry(QRect(70, 26, 101, 21));
        cb_Voltage = new QComboBox(groupBox_2);
        cb_Voltage->setObjectName("cb_Voltage");
        cb_Voltage->setGeometry(QRect(70, 53, 101, 21));
        cb_Channel = new QComboBox(groupBox_2);
        cb_Channel->setObjectName("cb_Channel");
        cb_Channel->setGeometry(QRect(70, 80, 101, 21));
        label = new QLabel(groupBox_2);
        label->setObjectName("label");
        label->setGeometry(QRect(11, 30, 53, 15));
        label_2 = new QLabel(groupBox_2);
        label_2->setObjectName("label_2");
        label_2->setGeometry(QRect(11, 54, 51, 16));
        label_3 = new QLabel(groupBox_2);
        label_3->setObjectName("label_3");
        label_3->setGeometry(QRect(11, 82, 53, 15));
        groupBox_3 = new QGroupBox(centralwidget);
        groupBox_3->setObjectName("groupBox_3");
        groupBox_3->setGeometry(QRect(11, 261, 179, 80));
        groupBox_3->setMaximumSize(QSize(180, 80));
        pushButton_9 = new QPushButton(groupBox_3);
        pushButton_9->setObjectName("pushButton_9");
        pushButton_9->setGeometry(QRect(99, 21, 75, 23));
        pushButton_8 = new QPushButton(groupBox_3);
        pushButton_8->setObjectName("pushButton_8");
        pushButton_8->setGeometry(QRect(10, 21, 75, 23));
        pushButton_13 = new QPushButton(groupBox_3);
        pushButton_13->setObjectName("pushButton_13");
        pushButton_13->setGeometry(QRect(10, 51, 75, 23));
        pushButton_15 = new QPushButton(groupBox_3);
        pushButton_15->setObjectName("pushButton_15");
        pushButton_15->setGeometry(QRect(99, 51, 75, 23));
        groupBox_4 = new QGroupBox(centralwidget);
        groupBox_4->setObjectName("groupBox_4");
        groupBox_4->setGeometry(QRect(11, 347, 179, 80));
        groupBox_4->setMaximumSize(QSize(16777215, 80));
        pushButton_3 = new QPushButton(groupBox_4);
        pushButton_3->setObjectName("pushButton_3");
        pushButton_3->setGeometry(QRect(10, 50, 81, 23));
        pushButton = new QPushButton(groupBox_4);
        pushButton->setObjectName("pushButton");
        pushButton->setGeometry(QRect(10, 22, 80, 23));
        pushButton_2 = new QPushButton(groupBox_4);
        pushButton_2->setObjectName("pushButton_2");
        pushButton_2->setGeometry(QRect(94, 22, 80, 23));
        groupBox_5 = new QGroupBox(centralwidget);
        groupBox_5->setObjectName("groupBox_5");
        groupBox_5->setGeometry(QRect(11, 433, 179, 120));
        groupBox_5->setMaximumSize(QSize(16777215, 120));
        pushButton_5 = new QPushButton(groupBox_5);
        pushButton_5->setObjectName("pushButton_5");
        pushButton_5->setGeometry(QRect(10, 60, 81, 23));
        pushButton_4 = new QPushButton(groupBox_5);
        pushButton_4->setObjectName("pushButton_4");
        pushButton_4->setGeometry(QRect(92, 60, 81, 23));
        pushButton_7 = new QPushButton(groupBox_5);
        pushButton_7->setObjectName("pushButton_7");
        pushButton_7->setGeometry(QRect(10, 30, 81, 23));
        pushButton_10 = new QPushButton(groupBox_5);
        pushButton_10->setObjectName("pushButton_10");
        pushButton_10->setGeometry(QRect(10, 90, 81, 23));
        pushButton_11 = new QPushButton(groupBox_5);
        pushButton_11->setObjectName("pushButton_11");
        pushButton_11->setGeometry(QRect(92, 90, 81, 23));
        pushButton_12 = new QPushButton(groupBox_5);
        pushButton_12->setObjectName("pushButton_12");
        pushButton_12->setGeometry(QRect(92, 30, 81, 23));
        groupBox_6 = new QGroupBox(centralwidget);
        groupBox_6->setObjectName("groupBox_6");
        groupBox_6->setGeometry(QRect(11, 559, 179, 201));
        textEdit = new QTextEdit(groupBox_6);
        textEdit->setObjectName("textEdit");
        textEdit->setGeometry(QRect(10, 20, 161, 161));
        tabWidget = new QTabWidget(centralwidget);
        tabWidget->setObjectName("tabWidget");
        tabWidget->setGeometry(QRect(1084, 10, 481, 801));
        tab = new QWidget();
        tab->setObjectName("tab");
        tableView2 = new QTableView(tab);
        tableView2->setObjectName("tableView2");
        tableView2->setGeometry(QRect(0, 40, 471, 311));
        listView = new QListView(tab);
        listView->setObjectName("listView");
        listView->setGeometry(QRect(0, 410, 471, 361));
        label_4 = new QLabel(tab);
        label_4->setObjectName("label_4");
        label_4->setGeometry(QRect(4, 387, 53, 15));
        pushButton_6 = new QPushButton(tab);
        pushButton_6->setObjectName("pushButton_6");
        pushButton_6->setGeometry(QRect(0, 352, 471, 31));
        harmonicParam = new QSpinBox(tab);
        harmonicParam->setObjectName("harmonicParam");
        harmonicParam->setGeometry(QRect(73, 10, 101, 22));
        harmonicParam->setMaximum(2000000000);
        harmonicParam->setValue(1000);
        label_7 = new QLabel(tab);
        label_7->setObjectName("label_7");
        label_7->setGeometry(QRect(10, 13, 53, 15));
        label_8 = new QLabel(tab);
        label_8->setObjectName("label_8");
        label_8->setGeometry(QRect(180, 13, 291, 16));
        lbl_CacheCount = new QLabel(tab);
        lbl_CacheCount->setObjectName("lbl_CacheCount");
        lbl_CacheCount->setGeometry(QRect(40, 387, 16, 16));
        progressBar = new QProgressBar(tab);
        progressBar->setObjectName("progressBar");
        progressBar->setGeometry(QRect(70, 384, 401, 23));
        progressBar->setValue(0);
        tabWidget->addTab(tab, QString());
        groupBox_7 = new QGroupBox(centralwidget);
        groupBox_7->setObjectName("groupBox_7");
        groupBox_7->setGeometry(QRect(11, 67, 179, 72));
        groupBox_7->setMaximumSize(QSize(180, 72));
        label_5 = new QLabel(groupBox_7);
        label_5->setObjectName("label_5");
        label_5->setGeometry(QRect(10, 22, 53, 15));
        label_6 = new QLabel(groupBox_7);
        label_6->setObjectName("label_6");
        label_6->setGeometry(QRect(10, 46, 53, 15));
        lbl_fvalues = new QLabel(groupBox_7);
        lbl_fvalues->setObjectName("lbl_fvalues");
        lbl_fvalues->setGeometry(QRect(70, 20, 101, 16));
        lbl_vvalues = new QLabel(groupBox_7);
        lbl_vvalues->setObjectName("lbl_vvalues");
        lbl_vvalues->setGeometry(QRect(71, 46, 101, 16));
        tabWidget_2 = new QTabWidget(centralwidget);
        tabWidget_2->setObjectName("tabWidget_2");
        tabWidget_2->setGeometry(QRect(200, 15, 881, 791));
        tab_3 = new QWidget();
        tab_3->setObjectName("tab_3");
        chartView = new PDChart(tab_3);
        chartView->setObjectName("chartView");
        chartView->setGeometry(QRect(0, 0, 871, 761));
        tabWidget_2->addTab(tab_3, QString());
        tab_4 = new QWidget();
        tab_4->setObjectName("tab_4");
        chartView_2 = new PDChart(tab_4);
        chartView_2->setObjectName("chartView_2");
        chartView_2->setGeometry(QRect(0, 0, 871, 761));
        tabWidget_2->addTab(tab_4, QString());
        tab_2 = new QWidget();
        tab_2->setObjectName("tab_2");
        chartView_3 = new PDChart(tab_2);
        chartView_3->setObjectName("chartView_3");
        chartView_3->setGeometry(QRect(0, 0, 871, 761));
        tabWidget_2->addTab(tab_2, QString());
        tab_5 = new QWidget();
        tab_5->setObjectName("tab_5");
        chartView_4 = new PDChart(tab_5);
        chartView_4->setObjectName("chartView_4");
        chartView_4->setGeometry(QRect(0, 0, 871, 761));
        tabWidget_2->addTab(tab_5, QString());
        pushButton_14 = new QPushButton(centralwidget);
        pushButton_14->setObjectName("pushButton_14");
        pushButton_14->setGeometry(QRect(10, 772, 181, 31));
        MainWindow->setCentralWidget(centralwidget);
        menubar = new QMenuBar(MainWindow);
        menubar->setObjectName("menubar");
        menubar->setGeometry(QRect(0, 0, 1567, 21));
        menu = new QMenu(menubar);
        menu->setObjectName("menu");
        MainWindow->setMenuBar(menubar);
        statusbar = new QStatusBar(MainWindow);
        statusbar->setObjectName("statusbar");
        MainWindow->setStatusBar(statusbar);

        menubar->addAction(menu->menuAction());
        menu->addAction(actionopen);
        menu->addAction(actionSave);

        retranslateUi(MainWindow);

        tabWidget->setCurrentIndex(0);
        tabWidget_2->setCurrentIndex(3);


        QMetaObject::connectSlotsByName(MainWindow);
    } // setupUi

    void retranslateUi(QMainWindow *MainWindow)
    {
        MainWindow->setWindowTitle(QCoreApplication::translate("MainWindow", "PD_Soft_V2", nullptr));
        actionopen->setText(QCoreApplication::translate("MainWindow", "\346\211\223\345\274\200", nullptr));
        actionSave->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230", nullptr));
        groupBox->setTitle(QCoreApplication::translate("MainWindow", "\346\227\266\345\237\272", nullptr));
        groupBox_2->setTitle(QCoreApplication::translate("MainWindow", "\345\217\202\346\225\260", nullptr));
        label->setText(QCoreApplication::translate("MainWindow", "Coupling", nullptr));
        label_2->setText(QCoreApplication::translate("MainWindow", "Voltage", nullptr));
        label_3->setText(QCoreApplication::translate("MainWindow", "Channel", nullptr));
        groupBox_3->setTitle(QCoreApplication::translate("MainWindow", "\351\207\207\351\233\206", nullptr));
        pushButton_9->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242", nullptr));
        pushButton_8->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213", nullptr));
        pushButton_13->setText(QCoreApplication::translate("MainWindow", "\346\270\205\351\231\244\347\274\223\345\255\230", nullptr));
        pushButton_15->setText(QCoreApplication::translate("MainWindow", "\345\220\216\345\217\260\351\207\207\351\233\206", nullptr));
        groupBox_4->setTitle(QCoreApplication::translate("MainWindow", "\346\250\241\346\213\237", nullptr));
        pushButton_3->setText(QCoreApplication::translate("MainWindow", "\345\215\225\345\270\247\346\270\262\346\237\223", nullptr));
        pushButton->setText(QCoreApplication::translate("MainWindow", "\345\274\200\345\247\213\346\250\241\346\213\237\346\225\260\346\215\256", nullptr));
        pushButton_2->setText(QCoreApplication::translate("MainWindow", "\345\201\234\346\255\242\346\250\241\346\213\237\346\225\260\346\215\256", nullptr));
        groupBox_5->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266", nullptr));
        pushButton_5->setText(QCoreApplication::translate("MainWindow", "\345\212\240\350\275\275CVS", nullptr));
        pushButton_4->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\207\272CVS", nullptr));
        pushButton_7->setText(QCoreApplication::translate("MainWindow", "\345\212\240\350\275\275\346\226\207\344\273\266", nullptr));
        pushButton_10->setText(QCoreApplication::translate("MainWindow", "\345\212\240\350\275\275\347\274\223\345\255\230\346\226\207\344\273\266", nullptr));
        pushButton_11->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\207\272\347\274\223\345\255\230\346\226\207\344\273\266", nullptr));
        pushButton_12->setText(QCoreApplication::translate("MainWindow", "\344\277\235\345\255\230\346\226\207\344\273\266", nullptr));
        groupBox_6->setTitle(QCoreApplication::translate("MainWindow", "Debug Log", nullptr));
        label_4->setText(QCoreApplication::translate("MainWindow", "\347\274\223\345\255\230", nullptr));
        pushButton_6->setText(QCoreApplication::translate("MainWindow", "\345\257\274\345\207\272\350\260\220\346\263\242\350\256\241\347\256\227\347\273\223\346\236\234\346\225\260\346\215\256", nullptr));
        label_7->setText(QCoreApplication::translate("MainWindow", "\345\237\272\346\263\242\351\242\221\347\216\207", nullptr));
        label_8->setText(QCoreApplication::translate("MainWindow", "Hz   (0\344\270\272\350\275\257\344\273\266\346\243\200\346\265\213\345\237\272\347\241\200\351\242\221\347\216\207\357\274\214\345\246\202\344\270\215\345\207\206\347\241\256\345\217\257\350\207\252\345\256\232\344\271\211)", nullptr));
        lbl_CacheCount->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        tabWidget->setTabText(tabWidget->indexOf(tab), QCoreApplication::translate("MainWindow", "\350\260\220\346\263\242", nullptr));
        groupBox_7->setTitle(QCoreApplication::translate("MainWindow", "\344\277\241\345\217\267", nullptr));
        label_5->setText(QCoreApplication::translate("MainWindow", "\346\265\213\350\257\225", nullptr));
        label_6->setText(QCoreApplication::translate("MainWindow", "\344\277\241\345\217\267\345\271\205\345\200\274", nullptr));
        lbl_fvalues->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        lbl_vvalues->setText(QCoreApplication::translate("MainWindow", "0", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_3), QCoreApplication::translate("MainWindow", "Time", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_4), QCoreApplication::translate("MainWindow", "Frequency", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_2), QCoreApplication::translate("MainWindow", "HarmonicRatio", nullptr));
        tabWidget_2->setTabText(tabWidget_2->indexOf(tab_5), QCoreApplication::translate("MainWindow", "HarmonicMagnitude", nullptr));
        pushButton_14->setText(QCoreApplication::translate("MainWindow", "\350\256\276\347\275\256", nullptr));
        menu->setTitle(QCoreApplication::translate("MainWindow", "\346\226\207\344\273\266", nullptr));
    } // retranslateUi

};

namespace Ui {
    class MainWindow: public Ui_MainWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_MAINWINDOW_H
