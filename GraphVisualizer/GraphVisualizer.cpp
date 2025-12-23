#include "GraphVisualizer.h"
#include <QGraphicsSceneMouseEvent>

#include "Edge.h"

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

    scene->setBackgroundBrush(Qt::white);
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

            if (item) {
                // 1. Если кликнули на существующий объект
                // Пытаемся привести объект к типу VertexItem
                VertexItem* clickedVertex = dynamic_cast<VertexItem*>(item);

                if (clickedVertex) {
                    // Если это Вершина
                    if (firstVertex == nullptr) {
                        // Это первый клик (начало ребра)
                        firstVertex = clickedVertex;
                        firstVertex->setColor(Qt::green); // Подсветим, что начали тянуть
                    }
                    else {
                        // Это второй клик (конец ребра)
                        // Проверка: нельзя соединить вершину саму с собой и нельзя создавать дубликаты
                        if (firstVertex != clickedVertex) {
                            bool edgeExists = false;
                            if (!firstVertex->isConnectedTo(clickedVertex)) {
                                Edge* newEdge = new Edge(firstVertex, clickedVertex);
                                scene->addItem(newEdge);
                                firstVertex->addEdge(newEdge);
                                clickedVertex->addEdge(newEdge);
                            }
                        }

                        // Сбрасываем состояние
                        firstVertex->setColor(Qt::white); // Возвращаем цвет
                        firstVertex = nullptr;
                    }
                    return true;
                }
            }
            else {
                // 2. Если кликнули в пустоту
                // Если мы были в режиме создания ребра (firstVertex выбран), то отменяем
                if (firstVertex) {
                    firstVertex->setColor(Qt::white);
                    firstVertex = nullptr;
                }
                else {
                    // Иначе создаем новую вершину (старый код)
                    VertexItem* ver = new VertexItem(nextId++, position);
                    scene->addItem(ver);
                }
                return true;
            }
        }
    }
    return QMainWindow::eventFilter(watched, event);
}