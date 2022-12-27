#ifndef UI_CHATWIN_H
#define UI_CHATWIN_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QListWidget>
#include <QtWidgets/QMainWindow>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_ChatWin
{
public:
    QWidget *centralwidget;
    QLineEdit *msgText;
    QPushButton *sendMsg;
    QListWidget *msgsHist;

    void setupUi(QMainWindow *ChatWin)
    {
        if (ChatWin->objectName().isEmpty())
            ChatWin->setObjectName(QString::fromUtf8("ChatWin"));
        ChatWin->resize(640, 480);
        ChatWin->setMinimumSize(QSize(640, 320));
        ChatWin->setMaximumSize(QSize(1920, 1280));
        centralwidget = new QWidget(ChatWin);
        centralwidget->setObjectName(QString::fromUtf8("centralwidget"));
        msgText = new QLineEdit(centralwidget);
        msgText->setObjectName(QString::fromUtf8("msgText"));
        msgText->setGeometry(QRect(10, 10, 531, 41));
        sendMsg = new QPushButton(centralwidget);
        sendMsg->setObjectName(QString::fromUtf8("sendMsg"));
        sendMsg->setGeometry(QRect(540, 10, 91, 41));
        msgsHist = new QListWidget(centralwidget);
        msgsHist->setObjectName(QString::fromUtf8("msgsHist"));
        msgsHist->setGeometry(QRect(10, 50, 621, 421));
        ChatWin->setCentralWidget(centralwidget);

        retranslateUi(ChatWin);
        QObject::connect(sendMsg, SIGNAL(clicked()), ChatWin, SLOT(sendMessage()));

        QMetaObject::connectSlotsByName(ChatWin);
    }

    void retranslateUi(QMainWindow *ChatWin)
    {
        ChatWin->setWindowTitle(QCoreApplication::translate("ChatWin", "ChatWin", nullptr));
        sendMsg->setText(QCoreApplication::translate("ChatWin", "Send msg", nullptr));
    } 

};

namespace Ui {
    class ChatWin: public Ui_ChatWin {};
}
QT_END_NAMESPACE

#endif // UI_CHATWIN_H
