#include "clockwidget.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    ClockWidget obj;
    obj.show();

    return app.exec();
}
