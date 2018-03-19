#include "clockwidget.h"
#include "ui_clockwidget.h"

#include <QDebug>
#include <QTime>
#include <QMouseEvent>
#include <QPainter>
#include <qmath.h>
#include <QtXml>
#include <QAction>
#include <QPixmap>

ClockWidget::ClockWidget(QWidget *parent):
    QWidget(parent),
    ui(new Ui::ClockWidget)
{
    //Init text
    textHide = (tr("Згорнути"));
    textShow = (tr("Розгорнути"));
    textExit = (tr("Вихід"));

    //Clock
    ClockTime ct = {-1, 0.0f};
    ctH = ct; ctM = ct; ctS = ct;
    this->pHours = new QPainter();
    this->pMinutes = new QPainter();
    this->pSeconds = new QPainter();
    this->startTimer(500);
    ui->setupUi(this);

    this->setWindowFlags(Qt::Window|Qt::FramelessWindowHint);
    this->setAttribute(Qt::WA_TranslucentBackground);

    QPixmap pixmapOk = QPixmap(":/icons/iconOk.png");
    this->setWindowIcon(pixmapOk);

    //Context Menu
    this->contextMenu = new QMenu(this);
    this->contextMenu->addAction(textHide);
    this->contextMenu->addSeparator();
    this->contextMenu->addAction(textExit);
    connect(this->contextMenu, SIGNAL(triggered(QAction*)), SLOT(slotActivated(QAction*)));

    //Context Tray Menu
    this->contextTrayMenu = new QMenu(this);
    QAction *viewWindow = new QAction(textShow, this);
    QAction *quitAction = new QAction(textExit, this);
    contextTrayMenu->addAction(viewWindow);
    contextTrayMenu->addSeparator();
    contextTrayMenu->addAction(quitAction);
    connect(viewWindow, SIGNAL(triggered()), this, SLOT(showNormal()));
    connect(quitAction, SIGNAL(triggered()), this, SLOT(close()));

    //Context Tray Icon
    trayIcon = new QSystemTrayIcon(this);
    QPixmap pixmapFail = QPixmap(":/icons/iconFail.png");
    trayIcon->setIcon(pixmapFail);
    trayIcon->setToolTip("QtClock" "\n" "--:--:--");
    trayIcon->setContextMenu(contextTrayMenu);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)), this, SLOT(slotTrayActivated(QSystemTrayIcon::ActivationReason)));
}

ClockWidget::~ClockWidget()
{
    delete ui;
}

void ClockWidget::drawing(QPainter **painter, QString snum, double angle)
{
    char hms;//h,m,s
    QFont font;
    QPointF center;
    int radius;
    if (*painter == pHours) {
        center.setX(static_cast<qreal>(this->width()/2));
        center.setY(static_cast<qreal>(this->height()/2));
        radius = 110;
        hms = 'h';
    }
    else {
        if (*painter == pMinutes || *painter == pSeconds) {
            QPointF cPoint(this->width()/2, this->height()/2);
            double rad_angle = qDegreesToRadians(angle);
            qreal x, y;
            if (*painter == pMinutes) {
                x = qCos(rad_angle)*140+cPoint.x();
                y = qSin(rad_angle)*140+cPoint.y();
                radius = 50;
                hms = 'm';
            }
            else if (*painter == pSeconds) {
                x = qCos(rad_angle)*125+cPoint.x();
                y = qSin(rad_angle)*125+cPoint.y();
                radius = 25;
                hms = 's';
            }
            QPointF point = {x, y};
            center = point;
        }
    }
    (*painter)->setPen(Qt::NoPen);
    (*painter)->drawEllipse(center, radius, radius);
    font.setBold(true);
    font.setItalic(true);
    font.setFamily("Arial");
    switch (hms) {
        case 'h': font.setPointSize(80); goto exit;
        case 'm': font.setPointSize(30); goto exit;
        case 's': font.setPointSize(15); goto exit;
    }
    exit:
    ;
    (*painter)->setFont(font);
    (*painter)->setPen(Qt::white);
    (*painter)->drawText(center.x()-radius, center.y()-radius, 2*radius, 2*radius, Qt::AlignCenter, snum);
    return;
}

template <typename T>
void ClockWidget::drawingHours(T position, double angle)
{
    QString snum = QString::number(position);
    if (position == -1)
        snum = "--";
    pHours->begin(this);
    setDefault(&pHours);
    drawing(&pHours, snum, angle);
    pHours->end();
    return;
}

template <typename T>
void ClockWidget::drawingMinutes(T position, double angle)
{
    if (position == -1)
        return;
    QString snum = QString::number(position);
    pMinutes->begin(this);
    setDefault(&pMinutes);
    drawing(&pMinutes, snum, angle);
    pMinutes->end();
    return;
}

template <typename T>
void ClockWidget::drawingSeconds(T position, double angle)
{
    if (position == -1)
        return;
    QString snum = QString::number(position);
    pSeconds->begin(this);
    setDefault(&pSeconds);
    drawing(&pSeconds, snum, angle);
    pSeconds->end();
    return;
}

void ClockWidget::setDefault(QPainter **painter)
{
    (*painter)->setRenderHint(QPainter::Antialiasing, true);
    (*painter)->setOpacity(0.70);
    (*painter)->setBrush(Qt::gray);
    return;
}

//when left mouse button is pressed and the cursor moves
void ClockWidget::mouseMoveEvent(QMouseEvent *event)
{
    if (movePnt.x()>=0 && movePnt.y()>=0 && event->buttons() && Qt::LeftButton) {
        QPoint difference = event->pos() - movePnt;
        QPoint newposition = this->pos() + difference;
        this->move(newposition);
    }
}

//when left mouse button is pressed
void ClockWidget::mousePressEvent(QMouseEvent *event)
{
    movePnt = event->pos();
}

//when left mouse button is released
void ClockWidget::mouseReleaseEvent(QMouseEvent *)
{
    movePnt = QPoint(-1, -1);
}

void ClockWidget::ClockUpdate()
{
    QDomDocument domDoc;
    QFile clock(":/xml/clock.xml");
    if (clock.open(QIODevice::ReadOnly)) {
        if (domDoc.setContent(&clock)) {
            QDomElement domElement = domDoc.documentElement();
            QDomNode domNode = domElement.firstChild();
            while (!domNode.isNull()) {
                if (domNode.isElement()) {
                    QDomElement domElement = domNode.toElement();
                    if (domElement.attribute("position", "") == QString::number(QTime::currentTime().hour())) {
                        ctH.position = domElement.attribute("position", "").toInt();
                        ctH.angle = domElement.text().toDouble();
                    }
                    if (domElement.attribute("position", "") == QString::number(QTime::currentTime().minute())) {
                        ctM.position = domElement.attribute("position", "").toInt();
                        ctM.angle = domElement.text().toDouble();
                    }
                    if (domElement.attribute("position", "") == QString::number(QTime::currentTime().second())) {
                        ctS.position = domElement.attribute("position", "").toInt();
                        ctS.angle = domElement.text().toDouble();
                    }
                }
                domNode = domNode.nextSibling();
            }
        }
        clock.close();
    }
}

void ClockWidget::paintEvent(QPaintEvent *)
{
    drawingHours(ctH.position, ctH.angle);
    drawingMinutes(ctM.position, ctM.angle);
    drawingSeconds(ctS.position, ctS.angle);
}

void ClockWidget::timerEvent(QTimerEvent *)
{
    static int pause(5);
    if (!pause) {
        this->ClockUpdate();
        this->update(0, 0, this->width(), this->height());
    }
    else
        --pause;

    QString msgtime("");
    msgtime += (ctH.position!=-1)?QString::number(ctH.position).length()>1?QString::number(ctH.position):"0"+QString::number(ctH.position):"--";
    msgtime += ":";
    msgtime += (ctM.position!=-1)?QString::number(ctM.position).length()>1?QString::number(ctM.position):"0"+QString::number(ctM.position):"--";
    msgtime += ":";
    msgtime += (ctS.position!=-1)?QString::number(ctS.position).length()>1?QString::number(ctS.position):"0"+QString::number(ctS.position):"--";

    trayIcon->setToolTip("QtClock" "\n" + msgtime);
}

void ClockWidget::contextMenuEvent(QContextMenuEvent *contextMenu)
{
    this->contextMenu->exec(contextMenu->globalPos());
}

void ClockWidget::changeEvent(QEvent *event)
{
    QWidget::changeEvent(event);
    if (event->type() == QEvent::WindowStateChange) {
        if (this->isMinimized()) {
            this->trayIcon->show();
            this->hide();
            this->showTrayMessage();
        }
    }
    if (this->isVisible() && trayIcon->isVisible()) {
        this->trayIcon->hide();
        this->raise();
        this->activateWindow();
    }
}

void ClockWidget::showTrayMessage()
{
    QSystemTrayIcon::MessageIcon icon = QSystemTrayIcon::MessageIcon(QSystemTrayIcon::Information);
    trayIcon->showMessage("QtClock",
                          trUtf8("Додаток згорнуто в трей. Для того, щоб "
                                 "розгорнути вікно додатка, клацніть по іконці додатка в трей"),
                          icon,
                          2000);
}

void ClockWidget::slotActivated(QAction *action)
{
    if (action->text()==textHide) {
        this->showMinimized();
        this->hide();
    }
    else if (action->text()==textExit)
        this->close();
    else
        return;
}

void ClockWidget::slotTrayActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason)
    {
        case QSystemTrayIcon::Context:
            break;
        default:
            this->showNormal();
    }
}
