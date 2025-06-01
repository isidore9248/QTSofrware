#include "MySoftware.h"
#include <QtWidgets/QApplication>
#include "USARTAss.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    USARTAss window;
    window.show();
    return app.exec();
}
