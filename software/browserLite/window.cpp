#include "window.h"
#include "ui_window.h"

Widget::Widget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Widget)
{
    ui->setupUi(this);
    if (qApp->arguments().size() > 1)
        ui->webView->setUrl(QUrl(qApp->arguments().last()));
    else
        ui->webView->setUrl(QUrl("http://localhost"));
    fullscreen = true;
}

Widget::~Widget()
{
    delete ui;
}

void Widget::keyPressEvent(QKeyEvent *event) {
    event->accept();
    if (event->key() == Qt::Key_Backspace)
        ui->webView->back();
    else if (event->key() == Qt::Key_F5)
        ui->webView->reload();
    else if (event->key() == Qt::Key_F11) {
        if (fullscreen) {
            showNormal();
            fullscreen = false;
        }
        else {
            showFullScreen();
            fullscreen = true;
        }
    }
}
