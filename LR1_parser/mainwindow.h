#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#define BackgroundPath ":/data/background.jpg"
#define FilePath ":/data/help.html"

#include <QMainWindow>
#include <QFile>
#include <QMessageBox>
#include <QTextStream>
#include "parser.h"
#include "showpic.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void InitUI();
    void text_in();

public slots:
    void on_action_parse_triggered();
    void on_action_DFA_triggered();

private:
    Ui::MainWindow *ui;
    LR1_Parser lr1;
    showPic show_it;
};
#endif // MAINWINDOW_H
