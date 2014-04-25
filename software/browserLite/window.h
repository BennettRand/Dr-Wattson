#ifndef WINDOW_H
#define WINDOW_H

#include <QWidget>
#include <QKeyEvent>

namespace Ui {
class Widget;
}

class Widget : public QWidget
{
    Q_OBJECT

public:
    explicit Widget(QWidget *parent = 0);
    ~Widget();

private:
    Ui::Widget *ui;
    void keyPressEvent(QKeyEvent *event);
    bool fullscreen;
};

#endif // WINDOW_H
