#include "GraphVisualizer.h"
#include <QGraphicsSceneMouseEvent>

GraphVisualizer::GraphVisualizer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setupScene();
}

GraphVisualizer::~GraphVisualizer()
{}


void GraphVisualizer::setupScene()
{
    scene = new QGraphicsScene(this);

    view = new QGraphicsView(scene);
    view->setRenderHint(QPainter::Antialiasing);

    setCentralWidget(view);

    scene->setSceneRect(0, 0, 800, 600);

    scene->installEventFilter(this);
}

bool GraphVisualizer::eventFilter(QObject* watched, QEvent* event)
{
    if (watched == scene && event->type() == QEvent::GraphicsSceneMousePress) 
    {
        QGraphicsSceneMouseEvent* mouseEvent = static_cast<QGraphicsSceneMouseEvent*>(event);

        if (mouseEvent->button() == Qt::LeftButton) 
        {
            QPointF position = mouseEvent->scenePos();

            QGraphicsItem* item = scene->itemAt(position, view->transform());

            if (!item)
            {
                VertexItem* ver = new VertexItem(nextId++, position);
                scene->addItem(ver);
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}