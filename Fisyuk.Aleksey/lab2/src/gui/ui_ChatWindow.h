/********************************************************************************
** Form generated from reading UI file 'ChatWindow.ui'
**
** Created by: Qt User Interface Compiler version 5.15.2
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_CHATWINDOW_H
#define UI_CHATWINDOW_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QSplitter>
#include <QtWidgets/QStatusBar>
#include <QtWidgets/QVBoxLayout>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChatWindow
{
public:
    QWidget *centralwidget;
    QGridLayout *gridLayout;
    QVBoxLayout *verticalLayout;
    QListWidget *chatWidget;
    QSplitter *splitter;
    QLineEdit *inputWidget;
    QPushButton *sendMessage;
    QStatusBar *statusbar;

    void setupUi(QMainWindow *ChatWindow)
    {
        if (ChatWindow->objectName().isEmpty())
            ChatWindow->setObjectName(QString::fromUtf8("ChatWindow"));
        ChatWindow->resize(400, 800);
        QSizePolicy sizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
        sizePolicy.setHorizontalStretch(0);
        sizePolicy.setVerticalStretch(0);
        sizePolicy.setHeightForWidth(ChatWindow->sizePolicy().hasHeightForWidth());
        ChatWindow->setSizePolicy(sizePolicy);
        ChatWindow->setMinimumSize(QSize(0, 0));
        ChatWindow->setMaximumSize(QSize(16777215, 16777215));
        QPalette palette;
        QBrush brush(QColor(0, 187, 255, 60));
        brush.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Active, QPalette::Base, brush);
        palette.setBrush(QPalette::Inactive, QPalette::Base, brush);
        QBrush brush1(QColor(240, 240, 240, 255));
        brush1.setStyle(Qt::SolidPattern);
        palette.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        ChatWindow->setPalette(palette);
        centralwidget = new QWidget(ChatWindow);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        gridLayout = new QGridLayout(centralwidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        verticalLayout = new QVBoxLayout();
        verticalLayout->setObjectName(QString::fromUtf8("verticalLayout"));
        chatWidget = new QListWidget(centralwidget);
        chatWidget->setObjectName(QString::fromUtf8("chatWidget"));

        verticalLayout->addWidget(chatWidget);

        splitter = new QSplitter(centralwidget);
        splitter->setObjectName(QString::fromUtf8("splitter"));
        splitter->setOrientation(Qt::Horizontal);
        inputWidget = new QLineEdit(splitter);
        inputWidget->setObjectName(QString::fromUtf8("inputWidget"));
        QPalette palette1;
        QBrush brush2(QColor(8, 156, 255, 30));
        brush2.setStyle(Qt::SolidPattern);
        palette1.setBrush(QPalette::Active, QPalette::Base, brush2);
        palette1.setBrush(QPalette::Inactive, QPalette::Base, brush2);
        palette1.setBrush(QPalette::Disabled, QPalette::Base, brush1);
        inputWidget->setPalette(palette1);
        splitter->addWidget(inputWidget);
        sendMessage = new QPushButton(splitter);
        sendMessage->setObjectName(QString::fromUtf8("sendMessage"));
        sendMessage->setMinimumSize(QSize(180, 30));
        sendMessage->setMaximumSize(QSize(180, 30));
        splitter->addWidget(sendMessage);

        verticalLayout->addWidget(splitter);


        gridLayout->addLayout(verticalLayout, 0, 0, 1, 1);

        ChatWindow->setCentralWidget(centralwidget);
        statusbar = new QStatusBar(ChatWindow);
        statusbar->setObjectName(QString::fromUtf8("statusbar"));
        ChatWindow->setStatusBar(statusbar);

        retranslateUi(ChatWindow);
        QObject::connect(sendMessage, SIGNAL(clicked()), ChatWindow, SLOT(send()));

        QMetaObject::connectSlotsByName(ChatWindow);
    } // setupUi

    void retranslateUi(QMainWindow *ChatWindow)
    {
        ChatWindow->setWindowTitle(QCoreApplication::translate("ChatWindow", "ChatWindow", nullptr));
        sendMessage->setText(QCoreApplication::translate("ChatWindow", "send", nullptr));
    } // retranslateUi

};

namespace Ui {
    class ChatWindow: public Ui_ChatWindow {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_CHATWINDOW_H
