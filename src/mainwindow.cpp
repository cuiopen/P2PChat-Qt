#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QDateTime>
#include <QSettings>
#include <QMessageBox>
#include <QFileDialog>
#include <QDesktopServices>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    connect(ui->actionEnglish,&QAction::triggered,this,&MainWindow::setLanguage);
    connect(ui->actionSimplifiedChinese,&QAction::triggered,this,&MainWindow::setLanguage);
    connect(ui->actionTraditionalChinese,&QAction::triggered,this,&MainWindow::setLanguage);

    connect(ui->actionHelp,&QAction::triggered,this,&MainWindow::getHelp);
    connect(ui->actionAbout,&QAction::triggered,this,&MainWindow::getHelp);

    connect(ui->btnLogin,SIGNAL(clicked()),this,SLOT(click_btnLogin()));
    connect(ui->btnLogout,SIGNAL(clicked(bool)),this,SLOT(click_btnLogout()));
    connect(ui->btnListen,SIGNAL(clicked(bool)),this,SLOT(click_btnListen()));
    connect(ui->btnSendFile,SIGNAL(clicked(bool)),this,SLOT(click_btnSendFile()));
    connect(ui->btnChooseFile,SIGNAL(clicked(bool)),this,SLOT(click_btnChooseFile()));
    connect(ui->btnSendMessage,SIGNAL(clicked()),this,SLOT(click_btnSendMessage()));

    ui->labIPAdress->setText(Tools::getLocalIP());
    ui->edtFinalIP->setText(DEFAULT_FILE_IP);
    ui->edtFinalPort->setText(QString::number(DEFAULT_FILE_PORT));
    ui->ProgressBar->hide();

    setLocalUserEnable(false);
    setLocalFileEnable(false);

    QFont font;
    font.setPixelSize(DEFAULT_MESSAGE_FONT_SIZE);
    ui->browserMessage->setFont(font);

    file = new fileWorker;
    chat = new chatWorker;

    connect(file,SIGNAL(messageShowReady(chatWorker::MessageType,QString,QString)),
            this,SLOT(showMessage(chatWorker::MessageType,QString,QString)) );
    connect(file,SIGNAL(progressBarUpdateReady(fileWorker::UpdateType,qint64)),
            this,SLOT(updateProgressBar(fileWorker::UpdateType,qint64)) );
    connect(chat,SIGNAL(messageShowReady(chatWorker::MessageType,QString,QString)),
            this,SLOT(showMessage(chatWorker::MessageType,QString,QString)) );
    connect(chat,SIGNAL(onlineUsersUpdateReady(QSet<QString>)),
            this,SLOT(updateOnlineUsers(QSet<QString>)) );

}

MainWindow::~MainWindow()
{
    chat->sendJson(chatWorker::Logout,ui->edtName->text()); // 销毁对象前先退出
    delete ui;
}
void MainWindow::setLanguage()
{
    QSettings settings("ypingcn","p2pchat-qt");

    if(QObject::sender() == ui->actionEnglish)
        settings.setValue("p2pchat-qt-lang","eng");
    else if(QObject::sender() == ui->actionSimplifiedChinese)
        settings.setValue("p2pchat-qt-lang","zh-cn");
    else if(QObject::sender() == ui->actionTraditionalChinese)
        settings.setValue("p2pchat-qt-lang","zh-tw");

    QMessageBox::information(this,tr("Language"),tr("Restart the app to switch language"),QMessageBox::Yes);
}

void MainWindow::getHelp()
{
    QDialog * help = new QDialog(this);
    QVBoxLayout * helpLayout = new QVBoxLayout(help);

    if(QObject::sender() == ui->actionAbout)
    {
        help->setWindowTitle(tr("About"));
        QLabel * content = new QLabel();
        QString website = "<a href=\"https://github.com/ypingcn/P2PChat-Qt\">" + tr("click to know more.") + "</a>";
        content->setText(tr("A simple program designed for LAN chat.")+website);
        content->setOpenExternalLinks(true);
        helpLayout->addWidget(content);
    }
    else if(QObject::sender() == ui->actionHelp)
    {
        help->setWindowTitle(tr("Help"));
        QTextEdit * content = new QTextBrowser();
        help->resize(600,380);
        content->append(tr("P2PChat-Qt Help"));
        content->append(tr("\n --- Nick Name Help"));
        content->append(tr("Valid character include : numbers , alphabet ( lower-case and upper-case ) , underline and simplified chinese character."));
        content->append(tr("\n --- Mask Help"));
        content->append(tr("Mask help you to chat in different range."));
        content->append(tr("#255.255.255.255 option# local usage , Presentation mode"));
        content->append(tr("#192.168.1.1 option# LAN usage , if two clients whose IP begins with 192.168 , are in the same LAN"));
        content->append(tr("\n --- IP Args Help"));
        content->append(tr("two number mean IP address and port number."));
        content->append(tr("You should input an vaild IP address in the first blank ( four numbers, separated by dots and ranged between 0 and 255 )"));
        content->append(tr("#127.0.0.1 is Presentation mode, you should input the destination IP instead.( As OnlineUsers shows )"));
        content->append(tr("Number in the second blank should be a number ranged between 1 and 65535"));
        content->append(tr("Port number don't recommand to change unless the two clients use the same port for file transmisson."));
        content->append(tr("'Listen' means I am ready to get the file."));
        helpLayout->addWidget(content);
    }

    help->exec();
}

void MainWindow::showMessage(chatWorker::MessageType type, QString hint, QString content)
{
    QDateTime now = QDateTime::currentDateTime();
    if(type == chatWorker::Login || type == chatWorker::Logout || type == chatWorker::System)
    {
        ui->browserMessage->setTextColor(QColor(190,190,190));
        ui->browserMessage->append(hint + now.toString("  hh:mm:ss"));
        ui->browserMessage->append(content);
    }
    else if(type == chatWorker::Chat)
    {
        ui->browserMessage->setTextColor(QColor(70,130,180));
        ui->browserMessage->append(hint + now.toString("  hh:mm:ss"));
        ui->browserMessage->setTextColor(QColor(0,0,0));
        ui->browserMessage->append(content);
    }
}

void MainWindow::updateProgressBar(fileWorker::UpdateType type,qint64 number)
{
    if(type == fileWorker::Show)
        ui->ProgressBar->show();
    else if(type == fileWorker::Hide)
        ui->ProgressBar->hide();
    else if(type == fileWorker::SetValue)
        ui->ProgressBar->setValue(number);
    else if(type == fileWorker::SetMax)
        ui->ProgressBar->setMaximum(number);
}

void MainWindow::updateOnlineUsers(QSet<QString> set)
{
    ui->listOnlineUser->clear();
    ui->listOnlineUser->insertItem(0,tr("Online Users:"));
    int i = 1;
    for(auto it = set.begin();it!= set.end() ;it++)
        ui->listOnlineUser->insertItem(i++,*it);
}

void MainWindow::setLocalUserEnable(bool status)
{
    /* --- 根据用户状态设置按钮可用性 ---*/
    ui->edtName->setEnabled(!status);
    ui->edtMessage->setEnabled(status);
    ui->btnSendMessage->setEnabled(status);
    ui->btnLogin->setEnabled(!status);
    ui->btnLogout->setEnabled(status);
    ui->boxMask->setEnabled(!status);
}

void MainWindow::setLocalFileEnable(bool status)
{
    /* --- 根据文件传输可用性设置按钮可用性以及监听状态 ---*/
    ui->edtFinalIP->setEnabled(status);
    ui->edtFinalPort->setEnabled(status);
    ui->btnChooseFile->setEnabled(status);
    ui->btnListen->setEnabled(status);
    ui->btnListen->setText(tr("Listen"));
}

void MainWindow::click_btnSendMessage()
{
    if(ui->edtMessage->toPlainText().isEmpty())
        QMessageBox::information(this,tr("No message"),tr("No message"),QMessageBox::Yes);
    else
    {
        chat->setMask(ui->boxMask->currentText());
        chat->sendJson(chatWorker::Chat,ui->edtName->text(),ui->edtMessage->toPlainText());
        ui->edtMessage->clear();
    }
}

void MainWindow::click_btnLogin()
{
    if(!Tools::vaildNickName(ui->edtName->text()))
        QMessageBox::information(this,tr("Invaild Nickname!"),tr("Invaild Nickname!"),QMessageBox::Yes);
    else if(Tools::getLocalIP() == QString::null)
        QMessageBox::question(this,tr("Offline"),tr("Check your Network to login"),QMessageBox::Yes);
    else
    {
        setLocalUserEnable(true);
        setLocalFileEnable(true);
        ui->btnListen->setEnabled(true);
        ui->labIPAdress->setText(Tools::getLocalIP());
        chat->setMask(ui->boxMask->currentText());
        chat->sendJson(chatWorker::Login,ui->edtName->text());
        chat->setStatus(chatWorker::Online);
        chat->setUserName(ui->edtName->text());
    }
}

void MainWindow::click_btnLogout()
{
    if(Tools::getLocalIP() == QString::null)
        QMessageBox::question(this,tr("Offline"),tr("Check your Network to login"),QMessageBox::Yes);
    else
    {
        setLocalUserEnable(false);
        setLocalFileEnable(false);

        chat->setMask(ui->boxMask->currentText());
        chat->sendJson(chatWorker::Logout,ui->edtName->text());
        chat->setStatus(chatWorker::Offline);
    }

}

void MainWindow::click_btnChooseFile()
{
    QString path = QFileDialog::getOpenFileName(this, tr("Choose File"), ".", tr("All File(*.*)"));
    if(!path.isEmpty())
    {
        if( file->setSendFile(path) )
            ui->btnSendFile->setEnabled(true);
    }
}

void MainWindow::click_btnListen()
{
    fileWorker::ListenType listenType = file->status();

    if(listenType == fileWorker::Unlisten)
    {
        file->setArgs(ui->edtFinalIP->text(), ui->edtFinalPort->text());
        if( file->startListen() )
        {
            ui->btnListen->setText(tr("UnListen"));
            showMessage(chatWorker::System,tr("System"),tr(" -- File Port Listening"));
        }
        else
        {
            showMessage(chatWorker::System,tr("System"),tr(" -- File Port Listening Fail"));
        }

    }
    else if(listenType == fileWorker::Listen)
    {
        ui->btnListen->setText(tr("Listen"));
        file->stopWorker();
        showMessage(chatWorker::System,tr("System"),tr(" -- File Port Listening Closed"));
    }
}

void MainWindow::click_btnSendFile()
{
    file->setArgs(ui->edtFinalIP->text(),ui->edtFinalPort->text());
    file->startSend();
}

