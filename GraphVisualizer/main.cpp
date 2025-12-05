#include "GraphVisualizer.h"
#include <QtWidgets/QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);
    GraphVisualizer window;
    window.show();
    return app.exec();
}
