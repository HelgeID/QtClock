#ifndef CLOCKWIDGET_H
#define CLOCKWIDGET_H

#include <QWidget>
#include <QMenu>
#include <QSystemTrayIcon>

namespace Ui {
    class ClockWidget;
}

class ClockWidget: public QWidget
{
    Q_OBJECT

    struct ClockTime
    {
        int position;
        double angle;
    } ctH, ctM, ctS;

public:
    explicit ClockWidget(QWidget *parent = 0);
    ~ClockWidget();

private:
    QString textHide;
    QString textShow;
    QString textExit;
    Ui::ClockWidget *ui;
    QPainter *pHours;
    QPainter *pMinutes;
    QPainter *pSeconds;
    void drawing(QPainter **, QString, double);
    template <typename T> void drawingHours(T, double);    //(position, angle)
    template <typename T> void drawingMinutes(T, double);  //(position, angle)
    template <typename T> void drawingSeconds(T, double);  //(position, angle)
    void setDefault(QPainter **);

    QPoint movePnt;
    void mouseMoveEvent(QMouseEvent *);
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);

    /* Main Update */
    void ClockUpdate();

    void paintEvent(QPaintEvent *);
    void timerEvent(QTimerEvent *);

    QMenu *contextMenu, *contextTrayMenu;
    void contextMenuEvent(QContextMenuEvent *);
    void changeEvent(QEvent *);

    QSystemTrayIcon *trayIcon;
    void showTrayMessage();

private slots:
    void slotActivated(QAction *);
    void slotTrayActivated(QSystemTrayIcon::ActivationReason);
};

#endif // CLOCKWIDGET_H
