#ifndef VIEW_ZOOM_H
#define VIEW_ZOOM_H

#include <QObject>
#include <QGraphicsView>

class View_zoom : public QObject
{
    Q_OBJECT
public:
    explicit View_zoom(QObject *parent = nullptr);
    View_zoom(QGraphicsView* view);
    void zoom_point(double factor);
    void set_modifiers(Qt::KeyboardModifiers modifiers);
    void set_zoom_factor_base(double value);

private:
    QGraphicsView* _view;
    Qt::KeyboardModifiers _modifiers;
    double _zoom_factor_base;
    QPointF target_scene_pos, target_viewport_pos;
    bool eventFilter(QObject* object, QEvent* event);

signals:
    void zoomed();

};

#endif // VIEW_ZOOM_H
