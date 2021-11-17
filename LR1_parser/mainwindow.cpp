#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "common.h"
#include <fstream>
#include <QProcess>
using namespace std;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    this->InitUI();
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::InitUI()
{
    this->setWindowTitle("LR1_parser");
    //设置背景
    QPixmap background(BackgroundPath);
    QPalette palette;
    palette.setBrush(QPalette::Background,background);
    this->setPalette(palette);

    QFile file_show(FilePath);
    file_show.open(QIODevice::ReadOnly);
    this->ui->textBrowser_help->setHtml(file_show.readAll());
    file_show.close();
}

void MainWindow::text_in()
{
    QFile file(R"(C:\QT_Projects\LR1_parser\source.txt)");
    if(!file.open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QMessageBox::critical(NULL, "提示", "无法创建添加文件");
            return ;
    }
    QTextStream out(&file);
    out << this->ui->textEdit->toPlainText();
    file.close();
}

void MainWindow::on_action_parse_triggered()
{
    lr1.clear_all(); //清空上一次的数据
    this->text_in(); //读入本次输入的数据
    ofstream T_out;
    T_out.open(R"(C:\QT_Projects\LR1_parser\graph_Tree.dot)");

    lr1.init(R"(C:\QT_Projects\LR1_parser\grammar.txt)");

    Token R_token;

    State R_state = lr1.parser(R"(C:\QT_Projects\LR1_parser\source.txt)", R_token);
    if(R_state == State::OK)
    {
        this->ui->textBrowser_state->clear();
        QString str;
        str = "<font color=\"#00BB00\">运行成功！</font>";
        this->ui->textBrowser_state->setText(str);
    }
    else
    {
        this->ui->textBrowser_state->clear();
        QString str_err;
        str_err.append("<font color=\"#FF0000\">");
        str_err.append("语法错误！运行失败！出错词法符号为：");
        str_err.append(QString::fromStdString(R_token.value));
        str_err.append("出错位置：");
        str_err.append(QString::number(R_token.line));
        str_err.append("行，");
        str_err.append(QString::number(R_token.col));
        str_err.append("列。");
        str_err.append("</font>");
        this->ui->textBrowser_state->setText(str_err);
        return;
    }
    lr1.printTree(T_out);

    QString m = "C:/Program Files (x86)/Graphviz2.38/bin/dot.exe";
    QStringList arguments;
    arguments << "C:/QT_Projects/LR1_parser/graph_Tree.dot" << "-Tpng" << "-o" << "graph_Tree.png";
    QProcess* pro = new QProcess;
    pro->execute(m, arguments);

    T_out.close();

    QPixmap img;
    img.load("graph_Tree.png");

    this->show_it.Show_new(true);

}

void MainWindow::on_action_DFA_triggered()
{
    lr1.clear_all(); //清空上一次的数据
    this->text_in(); //读入本次输入的数据
    ofstream D_out;
    D_out.open(R"(C:\QT_Projects\LR1_parser\graph_DFA.dot)");

    State R_state = lr1.init(R"(C:\QT_Projects\LR1_parser\source.txt)");

    if(R_state == State::OK)
    {
        this->ui->textBrowser_state->clear();
        QString str;
        str = "<font color=\"#00BB00\">运行成功！</font>";
        this->ui->textBrowser_state->setText(str);
    }
    else
    {
        this->ui->textBrowser_state->clear();
        QString str;
        str = "<font color=\"#FF0000\">文法载入错误！</font>";
        this->ui->textBrowser_state->setText(str);
        return;
    }

    lr1.printVP_DFA(D_out);

    QString m = "C:/Program Files (x86)/Graphviz2.38/bin/dot.exe";
    QStringList arguments;
    arguments << "C:/QT_Projects/LR1_parser/graph_DFA.dot" << "-Tpng" << "-o" << "graph_DFA.png";
    QProcess* pro = new QProcess;
    pro->execute(m, arguments);

    D_out.close();

    QPixmap img;
    img.load("graph_DFA.png");

    this->show_it.Show_new(false);

}
