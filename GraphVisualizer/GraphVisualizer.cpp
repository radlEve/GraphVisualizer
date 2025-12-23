#include "GraphVisualizer.h"
#include <QGraphicsSceneMouseEvent>
#include <QStyle>
#include <QToolBar>

#include "Edge.h"

GraphVisualizer::GraphVisualizer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    setupScene();
    setupUiCustom();
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

    // Создаем таймер
    autoPlayTimer = new QTimer(this);
    // Говорим таймеру: "Когда тикнешь, вызови onNextStep"
    connect(autoPlayTimer, &QTimer::timeout, this, &GraphVisualizer::onNextStep);
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

void GraphVisualizer::executeStep()
{
    if (currentSteps.isEmpty()) {
        actNextStep->setEnabled(false); // Если шаги кончились, гасим кнопку
        actAutoPlay->setEnabled(false);

        // Останавливаем таймер, если он работал
        if (autoPlayTimer->isActive()) {
            autoPlayTimer->stop();
            actAutoPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay));
        }
        return;
    }

    // Берем первый шаг из очереди
    AlgorithmStep step = currentSteps.dequeue();

    if (step.type == StepType::ResetColors) {
        // Сброс всех цветов
        for (QGraphicsItem* item : scene->items()) {
            if (VertexItem* v = dynamic_cast<VertexItem*>(item)) {
                v->setColor(Qt::white);
            }
            if (Edge* e = dynamic_cast<Edge*>(item)) {
                e->setColor(Qt::black);
            }
            // Ребрам цвет пока не меняли (они всегда черные, но можно добавить метод setColor и в Edge)
        }
    }
    else if (step.type == StepType::HighlightNode) {
        VertexItem* v = static_cast<VertexItem*>(step.item);
        if (v) v->setColor(step.color);
    }
    else if (step.type == StepType::HighlightEdge) {
        Edge* e = static_cast<Edge*>(step.item);
        // Тут нужен метод setColor в Edge (если его нет - ребро останется черным)
        if (e) e->setColor(step.color); 
    }
}

void GraphVisualizer::onNextStep()
{
    executeStep();
}

void GraphVisualizer::onRunBFS()
{
    // 1. Собираем данные со сцены
    QList<VertexItem*> vertices;
    QList<Edge*> edges;

    for (QGraphicsItem* item : scene->items()) {
        if (VertexItem* v = dynamic_cast<VertexItem*>(item)) vertices.append(v);
        else if (Edge* e = dynamic_cast<Edge*>(item)) edges.append(e);
    }

    if (vertices.isEmpty()) return;

    // 2. Загружаем в решатель
    solver.setGraphData(vertices, edges);

    // 3. Запускаем алгоритм (стартуем, например, с вершины с минимальным ID)
    // Либо можно сделать выбор стартовой вершины кликом
    int startId = vertices.first()->getId();

    currentSteps = solver.runBFS(startId);

    if (!currentSteps.isEmpty()) {
        actNextStep->setEnabled(true);
        actAutoPlay->setEnabled(true);
        // Можно сразу выполнить первый шаг (сброс цветов), чтобы пользователь видел реакцию
        executeStep();
    }
    // Тут можно или сразу все выполнить (циклом), или ждать нажатия кнопки "Next"
    // Пример мгновенного выполнения:
    /*
    while (!currentSteps.isEmpty()) {
        executeStep();
    }
    */
}

void GraphVisualizer::setupUiCustom()
{
    // Создаем тулбар
    toolbar = addToolBar("Main Toolbar");
    toolbar->setMovable(false); // Зафиксировать, чтобы не таскали

    QIcon iconRun = style()->standardIcon(QStyle::SP_MediaPlay);
    actAutoPlay = toolbar->addAction(iconRun, "Авто-запуск", this, &GraphVisualizer::onAutoPlay);
    actAutoPlay->setEnabled(false); // Пока алгоритм не выбран, запускать нечего

    toolbar->addSeparator();

    // 1. Кнопка ОЧИСТИТЬ (иконка "Мусорка")
    // Используем стандартные иконки Qt, чтобы не искать картинки
    QIcon iconTrash = style()->standardIcon(QStyle::SP_TrashIcon);
    actClear = toolbar->addAction(iconTrash, "Очистить граф", this, &GraphVisualizer::onClear);

    toolbar->addSeparator(); // Вертикальная черта

    // 2. Кнопка BFS (иконка "Компьютер" или "Play")
    QIcon iconPlay = style()->standardIcon(QStyle::SP_ComputerIcon);
    actRunBFS = toolbar->addAction(iconPlay, "Запуск BFS", this, &GraphVisualizer::onRunBFS);

    // 3. Кнопка NEXT (иконка "Стрелка вправо")
    QIcon iconNext = style()->standardIcon(QStyle::SP_ArrowRight);
    actNextStep = toolbar->addAction(iconNext, "Шаг вперед", this, &GraphVisualizer::onNextStep);

    // Изначально кнопка "Далее" может быть неактивна, пока не запущен алгоритм
    actNextStep->setEnabled(false);

}

void GraphVisualizer::onClear()
{
    // Очищаем сцену
    scene->clear();

    // Сбрасываем внутренние переменные
    solver = GraphSolver(); // Новый пустой решатель
    currentSteps.clear();
    firstVertex = nullptr;
    nextId = 1; // Сбрасываем счетчик ID

    // Блокируем кнопку "Далее"
    actNextStep->setEnabled(false);
    autoPlayTimer->stop();
}

void GraphVisualizer::onAutoPlay()
{
    if (autoPlayTimer->isActive()) {
        // Если таймер уже идет -> ставим на паузу
        autoPlayTimer->stop();
        actAutoPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPlay)); // Меняем иконку на Play
        actAutoPlay->setText("Продолжить");
    }
    else {
        // Если стоит -> запускаем (каждые 300 мс)
        if (!currentSteps.isEmpty()) {
            autoPlayTimer->start(300); // 300 мс задержка между шагами
            actAutoPlay->setIcon(style()->standardIcon(QStyle::SP_MediaPause)); // Меняем иконку на Pause
            actAutoPlay->setText("Пауза");
        }
    }
}