#include "mainwindow.h"
#include <QApplication>
#include <QTranslator>
#include <QSettings>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile qss(":/theme/default.qss"); // 加载主题
    qss.open(QFile::ReadOnly);
    qApp->setStyleSheet(qss.readAll());
    qss.close();

    QSettings settings("ypingcn","p2pchat-qt");
    if(!settings.contains("p2pchat-qt-lang"))
        settings.setValue("p2pchat-qt-lang","zh-cn");

    QTranslator Translator; // 加载翻译文件
    if(QSysInfo::kernelType() == "linux")
        Translator.load("/usr/bin/p2pchat-qt/local/"+settings.value("p2pchat-qt-lang").toString()+".qm");
    else
        Translator.load(settings.value("p2pchat-qt-lang").toString()+".qm");
    a.installTranslator(&Translator);

    MainWindow w;
    w.show();

    return a.exec();
}
