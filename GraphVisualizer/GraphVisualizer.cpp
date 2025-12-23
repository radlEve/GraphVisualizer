#include "GraphVisualizer.h"
#include <QGraphicsSceneMouseEvent>
#include <QStyle>
#include <QToolBar>
#include <QMenu>
#include <QAction>  

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

void GraphVisualizer::removeEdge(Edge* e)
{
    if (!e) return;

    // 1. Сообщаем вершинам, что этого ребра больше нет
    if (e->sourceNode()) e->sourceNode()->removeEdgeFromList(e);
    if (e->destNode()) e->destNode()->removeEdgeFromList(e);

    // 2. Удаляем визуально со сцены
    scene->removeItem(e);

    // 3. Удаляем из памяти
    delete e;
}

void GraphVisualizer::removeVertex(VertexItem* v)
{
    if (!v) return;

    // 1. Сначала удаляем ВСЕ ребра, связанные с этой вершиной
    // Делаем копию списка, так как будем удалять из оригинала в процессе
    QList<Edge*> edgesToRemove = v->getEdges();

    for (Edge* edge : edgesToRemove) {
        removeEdge(edge);
    }

    // 2. Удаляем саму вершину со сцены
    scene->removeItem(v);

    // 3. Удаляем из памяти
    delete v;
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

void GraphVisualizer::startBFS(int startId)
{
    // 1. Сбрасываем старое (на всякий случай)
    actNextStep->setEnabled(false);
    actAutoPlay->setEnabled(false);
    if (autoPlayTimer->isActive()) onAutoPlay();
    // Или просто:
    solver = GraphSolver();
    currentSteps.clear();

    // 2. Собираем данные со сцены
    QList<VertexItem*> vertices;
    QList<Edge*> edges;

    for (QGraphicsItem* item : scene->items()) {
        if (VertexItem* v = dynamic_cast<VertexItem*>(item)) vertices.append(v);
        else if (Edge* e = dynamic_cast<Edge*>(item)) edges.append(e);
    }

    if (vertices.isEmpty()) return;

    // 3. Загружаем и запускаем
    solver.setGraphData(vertices, edges);

    // Используем переданный ID
    currentSteps = solver.runBFS(startId);

    // 4. Активируем интерфейс
    if (!currentSteps.isEmpty()) {
        actNextStep->setEnabled(true);
        actAutoPlay->setEnabled(true);
        executeStep(); // Сразу выполняем первый шаг (сброс цветов)
    }
}

void GraphVisualizer::startDFS(int startId)
{
    // 1. Сброс
    solver = GraphSolver();
    currentSteps.clear();

    // 2. Сбор данных
    QList<VertexItem*> vertices;
    QList<Edge*> edges;
    for (QGraphicsItem* item : scene->items()) {
        if (VertexItem* v = dynamic_cast<VertexItem*>(item)) vertices.append(v);
        else if (Edge* e = dynamic_cast<Edge*>(item)) edges.append(e);
    }
    if (vertices.isEmpty()) return;

    // 3. Загрузка
    solver.setGraphData(vertices, edges);

    // 4. ЗАПУСК ИМЕННО DFS
    currentSteps = solver.runDFS(startId);

    // 5. Интерфейс
    if (!currentSteps.isEmpty()) {
        actNextStep->setEnabled(true);
        actAutoPlay->setEnabled(true);
        executeStep();
    }
}

void GraphVisualizer::startDijkstra(int startId)
{
    // 1. Сброс
    solver = GraphSolver();
    currentSteps.clear();

    // 2. Сбор данных
    QList<VertexItem*> vertices;
    QList<Edge*> edges;
    for (QGraphicsItem* item : scene->items()) {
        if (VertexItem* v = dynamic_cast<VertexItem*>(item)) vertices.append(v);
        else if (Edge* e = dynamic_cast<Edge*>(item)) edges.append(e);
    }
    if (vertices.isEmpty()) return;

    // 3. Загрузка
    solver.setGraphData(vertices, edges);

    // 4. ЗАПУСК
    currentSteps = solver.runDijkstra(startId);

    // 5. Интерфейс
    if (!currentSteps.isEmpty()) {
        actNextStep->setEnabled(true);
        actAutoPlay->setEnabled(true);
        executeStep();
    }
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

    // 2. Кнопка NEXT (иконка "Стрелка вправо")
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

void GraphVisualizer::contextMenuEvent(QContextMenuEvent* event)
{
    // Получаем позицию клика в координатах сцены
    // (хитрый момент: event->pos() это координаты окна, надо перевести во View, потом в Scene)
    QPoint viewPos = view->mapFrom(this, event->pos());
    QPointF scenePos = view->mapToScene(viewPos);

    // Смотрим, есть ли что-то под курсором
    QGraphicsItem* item = scene->itemAt(scenePos, view->transform());

    QMenu menu(this);

    // === СЛУЧАЙ 1: Кликнули на ВЕРШИНУ ===
    if (VertexItem* v = dynamic_cast<VertexItem*>(item)) {
        // Действие 1: Запустить BFS отсюда
        if (VertexItem* v = dynamic_cast<VertexItem*>(item)) {

            QAction* actBFS = menu.addAction("Запустить BFS отсюда");

            // Теперь лямбда просто вызывает наш готовый метод
            connect(actBFS, &QAction::triggered, [this, v]() {
                startBFS(v->getId()); });

            QAction* actDFS = menu.addAction("Запустить DFS отсюда");
            connect(actDFS, &QAction::triggered, [this, v]() {
                startDFS(v->getId());
                });

            QAction* actDijkstra = menu.addAction("Запустить Дейкстру");
            connect(actDijkstra, &QAction::triggered, [this, v]() {
                startDijkstra(v->getId());
                });
        }

        menu.addSeparator();

        // Действие 2: Удалить вершину
        QAction* actDel = menu.addAction("Удалить вершину");
        connect(actDel, &QAction::triggered, [this, v]() {
            removeVertex(v);
            });
    }

    // === СЛУЧАЙ 2: Кликнули на РЕБРО ===
    else if (Edge* e = dynamic_cast<Edge*>(item)) {
        // Действие 1: Изменить вес
        QAction* actWeight = menu.addAction("Изменить вес");
        connect(actWeight, &QAction::triggered, [this, e]() {
            // Просто вызываем тот же код, что и при двойном клике
            // Но так как mouseDoubleClickEvent требует событие, проще сымитировать или вынести логику.
            // Для простоты пока оставим редактирование веса только по двойному клику,
            // или скопируй логику QInputDialog сюда.
            });

        // Действие 2: Удалить ребро
        QAction* actDel = menu.addAction("Удалить ребро");
        connect(actDel, &QAction::triggered, [this, e]() {
            removeEdge(e);
            });
    }

    // === СЛУЧАЙ 3: Кликнули в ПУСТОТУ ===
    else {
        QAction* actClear = menu.addAction("Очистить всё");
        connect(actClear, &QAction::triggered, this, &GraphVisualizer::onClear);
    }

    // Показываем меню в точке клика
    menu.exec(event->globalPos());
}