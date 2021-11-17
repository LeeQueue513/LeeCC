#include "showpic.h"
#include "ui_showpic.h"
#include "mainwindow.h"

showPic::showPic(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::showPic)
{
    ui->setupUi(this);
    this->InitUI();
}

showPic::~showPic()
{
    delete ui;
}

void showPic::InitUI()
{
    //设置背景
    QPixmap background(BackgroundPath);
    QPalette palette;
    palette.setBrush(QPalette::Background,background);
    this->setPalette(palette);

    this->myView = new View_zoom(ui->graphicsView);
    myView->set_modifiers(Qt::NoModifier);
    ui->graphicsView->setRenderHint(QPainter::Antialiasing); //设置反走样

    this->scene = new QGraphicsScene;
    //scene->setSceneRect(0,0,SCENE_WIDTH,SCENE_HEIGHT);
    scene->addPixmap(QPixmap("graph_Tree.png"));
    ui->graphicsView->setScene(scene);
    ui->graphicsView->setDragMode(QGraphicsView::ScrollHandDrag); //设置场景
}

void showPic::Show_new(bool is_tree)
{
    this->scene->clear();
    if(is_tree)
    {
        this->setWindowTitle("语法分析树");
        scene->addPixmap(QPixmap("graph_Tree.png"));
    }
    else
    {
        this->setWindowTitle("DFA");
        scene->addPixmap(QPixmap("graph_DFA.png"));
    }
    ui->graphicsView->setScene(scene);
    this->show();
}
