#ifndef SHOWPIC_H
#define SHOWPIC_H

#define SCENE_WIDTH 900
#define SCENE_HEIGHT 1000

#include <QWidget>
#include <QGraphicsScene>
#include <view_zoom.h>

namespace Ui {
class showPic;
}

class showPic : public QWidget
{
    Q_OBJECT

public:
    explicit showPic(QWidget *parent = nullptr);
    ~showPic();

    void InitUI();
    void Show_new(bool is_tree);

private:
    Ui::showPic *ui;
    QGraphicsScene *scene; //场景
    View_zoom *myView;     //自定义视图，用于鼠标缩放
};

#endif // SHOWPIC_H
