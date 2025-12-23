#include "GraphSolver.h"
#include "VertexItem.h"
#include "Edge.h"
#include <QQueue>
#include <QSet>
#include <QStack>
#include <limits>
#include <algorithm>

GraphSolver::GraphSolver()
{
}

void GraphSolver::setGraphData(QList<VertexItem*> vertices, QList<Edge*> edges)
{
    m_vertices = vertices;
    m_edges = edges;
}

VertexItem* GraphSolver::findNodeById(int id)
{
    for (VertexItem* v : m_vertices) {
        if (v->getId() == id) return v;
    }
    return nullptr;
}

QList<Edge*> GraphSolver::getConnectedEdges(VertexItem* node)
{
    QList<Edge*> result;
    for (Edge* edge : m_edges) {
        // Если ребро соединено с нашей вершиной
        if (edge->sourceNode() == node || edge->destNode() == node) {
            result.append(edge);
        }
    }
    return result;
}

// === РЕАЛИЗАЦИЯ BFS (Поиск в ширину) ===
QQueue<AlgorithmStep> GraphSolver::runBFS(int startNodeId)
{
    QQueue<AlgorithmStep> steps;

    // 1. Сбрасываем цвета в начале
    steps.enqueue({ ResetColors, nullptr, Qt::white });

    VertexItem* startNode = findNodeById(startNodeId);
    if (!startNode) return steps;

    // Стандартные структуры для BFS
    QQueue<VertexItem*> queue;
    QSet<VertexItem*> visited;

    // Начальная инициализация
    queue.enqueue(startNode);
    visited.insert(startNode);

    // Добавляем шаг анимации: "Покрасить старт в зеленый"
    steps.enqueue({ HighlightNode, startNode, Qt::green });

    while (!queue.empty()) {
        VertexItem* current = queue.dequeue();

        // Получаем соседей
        QList<Edge*> connectedEdges = getConnectedEdges(current);

        for (Edge* edge : connectedEdges) {
            // Определяем соседа (так как ребро может смотреть в любую сторону)
            VertexItem* neighbor = (edge->sourceNode() == current) ? edge->destNode() : edge->sourceNode();

            if (!visited.contains(neighbor)) {
                visited.insert(neighbor);
                queue.enqueue(neighbor);

                // АНИМАЦИЯ:
                // 1. Красим ребро, по которому прошли (желтым)
                steps.enqueue({ HighlightEdge, edge, Qt::yellow });
                // 2. Красим найденного соседа (желтым, типа "в обработке")
                steps.enqueue({ HighlightNode, neighbor, Qt::yellow });
            }
        }

        // Когда закончили обработку вершины, красим её в "посещенный" (серый)
        // (кроме стартовой, пусть останется зеленой для красоты)
        if (current != startNode) {
            steps.enqueue({ HighlightNode, current, Qt::lightGray });
        }
    }

    return steps;
}

// === РЕАЛИЗАЦИЯ DFS (Поиск в глубину) ===
QQueue<AlgorithmStep> GraphSolver::runDFS(int startNodeId)
{
    QQueue<AlgorithmStep> steps;
    steps.enqueue({ ResetColors, nullptr, Qt::white });

    VertexItem* startNode = findNodeById(startNodeId);
    if (!startNode) return steps;

    // Используем СТЕК вместо Очереди
    QStack<VertexItem*> stack;
    QSet<VertexItem*> visited;

    stack.push(startNode);

    while (!stack.isEmpty()) {
        VertexItem* current = stack.pop();

        // В DFS мы помечаем вершину посещенной, когда ДОСТАЕМ её из стека
        if (!visited.contains(current)) {
            visited.insert(current);

            // Анимация: текущая вершина обрабатывается
            // Если это старт - зеленый, иначе - желтый (или оранжевый для отличия от BFS)
            QColor color = (current == startNode) ? Qt::green : Qt::yellow;
            steps.enqueue({ HighlightNode, current, color });

            // Получаем соседей
            QList<Edge*> connectedEdges = getConnectedEdges(current);

            // Важный момент: чтобы идти "слева направо", в стек кладут в обратном порядке.
            // Но для визуализации это не критично.

            for (Edge* edge : connectedEdges) {
                VertexItem* neighbor = (edge->sourceNode() == current) ? edge->destNode() : edge->sourceNode();

                if (!visited.contains(neighbor)) {
                    stack.push(neighbor);

                    // Анимация: подсветим ребро, которое "нашли", но еще не прошли
                    // Сделаем его, например, синим, чтобы отличать от пройденного
                    steps.enqueue({ HighlightEdge, edge, Qt::cyan });
                }
            }
        }
    }

    return steps;
}

QQueue<AlgorithmStep> GraphSolver::runDijkstra(int startNodeId)
{
    QQueue<AlgorithmStep> steps;
    steps.enqueue({ ResetColors, nullptr, Qt::white });

    VertexItem* startNode = findNodeById(startNodeId);
    if (!startNode) return steps;

    // 1. Инициализация
    // Храним текущие минимальные расстояния
    QMap<VertexItem*, int> distances;
    // Храним "откуда мы пришли" (чтобы потом восстановить путь, если нужно)
    // Но для раскраски нам достаточно знать, что ребро входит в кратчайшее дерево
    QMap<VertexItem*, Edge*> parentEdge;

    QList<VertexItem*> unvisitedNodes = m_vertices;

    // Заполняем бесконечностью
    for (VertexItem* v : m_vertices) {
        distances[v] = std::numeric_limits<int>::max(); // Бесконечность
    }
    distances[startNode] = 0;

    while (!unvisitedNodes.isEmpty()) {
        // 2. Ищем вершину с минимальным расстоянием среди непосещенных
        VertexItem* current = nullptr;
        int minDist = std::numeric_limits<int>::max();

        for (VertexItem* node : unvisitedNodes) {
            if (distances[node] < minDist) {
                minDist = distances[node];
                current = node;
            }
        }

        // Если все оставшиеся вершины недостижимы (бесконечность) - выходим
        if (current == nullptr || minDist == std::numeric_limits<int>::max()) {
            break;
        }

        // Удаляем из непосещенных
        unvisitedNodes.removeOne(current);

        // Анимация: "Мы закрепили эту вершину" (Зеленый - финал)
        steps.enqueue({ HighlightNode, current, Qt::green });

        // Если мы пришли в эту вершину по какому-то ребру, красим это ребро в "хороший" путь
        if (parentEdge.contains(current)) {
            steps.enqueue({ HighlightEdge, parentEdge[current], Qt::green });
        }

        // 3. Релаксация (обновление соседей)
        QList<Edge*> edges = getConnectedEdges(current);

        for (Edge* edge : edges) {
            VertexItem* neighbor = (edge->sourceNode() == current) ? edge->destNode() : edge->sourceNode();

            // Если сосед еще не посещен
            if (unvisitedNodes.contains(neighbor)) {
                // Анимация: "Проверяем ребро" (Желтый)
                steps.enqueue({ HighlightEdge, edge, Qt::yellow });

                int weight = edge->getWeight();
                int newDist = distances[current] + weight;

                // Если нашли путь короче
                if (newDist < distances[neighbor]) {
                    distances[neighbor] = newDist;
                    parentEdge[neighbor] = edge;

                    // Анимация: "Нашли путь лучше!" (Сосед мигает оранжевым)
                    steps.enqueue({ HighlightNode, neighbor, Qt::darkYellow });
                }
                else {
                    // Анимация: "Путь не лучше, возвращаем ребро в черный/серый"
                    // (Опционально, можно не делать, или красить в серый как "отброшенное")
                    steps.enqueue({ HighlightEdge, edge, Qt::lightGray });
                }
            }
        }
    }

    return steps;
}

QQueue<AlgorithmStep> GraphSolver::runConnectedComponents()
{
    QQueue<AlgorithmStep> steps;
    steps.enqueue({ ResetColors, nullptr, Qt::white });

    QSet<VertexItem*> visited;

    // Список цветов для разных групп (можно добавить больше)
    QList<QColor> palette = {
        Qt::red, Qt::blue, Qt::green, Qt::magenta, Qt::darkCyan, Qt::darkYellow
    };
    int colorIndex = 0;

    // Пробегаем по ВСЕМ вершинам графа
    for (VertexItem* node : m_vertices) {

        // Если вершину еще не посещали - значит, мы нашли НОВЫЙ островок
        if (!visited.contains(node)) {

            // Выбираем цвет. Если цветов мало, начинаем сначала (циклично)
            QColor currentColor = palette[colorIndex % palette.size()];
            colorIndex++;

            // Запускаем локальный BFS/DFS, чтобы найти всех жителей этого острова
            // Используем стек или очередь локально
            QQueue<VertexItem*> queue;
            queue.enqueue(node);
            visited.insert(node);

            steps.enqueue({ HighlightNode, node, currentColor });

            while (!queue.isEmpty()) {
                VertexItem* current = queue.dequeue();

                // Ищем соседей
                QList<Edge*> edges = getConnectedEdges(current);
                for (Edge* edge : edges) {
                    VertexItem* neighbor = (edge->sourceNode() == current) ? edge->destNode() : edge->sourceNode();

                    if (!visited.contains(neighbor)) {
                        visited.insert(neighbor);
                        queue.enqueue(neighbor);

                        // Красим соседа и ребро в цвет текущей группы
                        steps.enqueue({ HighlightEdge, edge, currentColor });
                        steps.enqueue({ HighlightNode, neighbor, currentColor });
                    }
                    // Если сосед уже посещен, но ребро еще черное - покрасим его тоже (для красоты)
                    else {
                        // Дополнительная проверка, чтобы не перекрашивать ребра других групп
                        // (упрощенно красим всё внутри группы)
                        steps.enqueue({ HighlightEdge, edge, currentColor });
                    }
                }
            }
        }
    }

    return steps;
}

// --- ВСПОМОГАТЕЛЬНЫЕ ФУНКЦИИ DSU ---
VertexItem* findSet(VertexItem* v, QMap<VertexItem*, VertexItem*>& parent) {
    if (v == parent[v])
        return v;
    return parent[v] = findSet(parent[v], parent);
}

void unionSets(VertexItem* a, VertexItem* b, QMap<VertexItem*, VertexItem*>& parent) {
    a = findSet(a, parent);
    b = findSet(b, parent);
    if (a != b)
        parent[b] = a;
}

QQueue<AlgorithmStep> GraphSolver::runKruskal()
{
    QQueue<AlgorithmStep> steps;
    steps.enqueue({ ResetColors, nullptr, Qt::white });

    // 1. Инициализация DSU
    // Изначально каждая вершина - сама себе начальник (отдельное множество)
    QMap<VertexItem*, VertexItem*> parent;
    for (VertexItem* v : m_vertices) {
        parent[v] = v;
    }

    // 2. Сортировка ребер
    // Копируем список ребер, чтобы не менять порядок на сцене
    QList<Edge*> sortedEdges = m_edges;

    // Сортируем лямбда-функцией по весу
    std::sort(sortedEdges.begin(), sortedEdges.end(), [](Edge* a, Edge* b) {
        return a->getWeight() < b->getWeight();
        });

    // 3. Главный цикл
    int edgesCount = 0; // Сколько ребер мы взяли (для остановки, когда V-1)

    for (Edge* edge : sortedEdges) {

        // Анимация: "Рассматриваем текущее ребро" (Желтый)
        steps.enqueue({ HighlightEdge, edge, Qt::yellow });

        VertexItem* u = edge->sourceNode();
        VertexItem* v = edge->destNode();

        // Проверяем, в одной ли они группе
        if (findSet(u, parent) != findSet(v, parent)) {
            // Если в РАЗНЫХ -> Берем ребро!
            unionSets(u, v, parent);

            // Анимация: "Берем!" (Зеленый)
            steps.enqueue({ HighlightEdge, edge, Qt::green });
            // Подсветим вершины тоже, чтобы было видно, что они теперь в дереве
            steps.enqueue({ HighlightNode, u, Qt::green });
            steps.enqueue({ HighlightNode, v, Qt::green });

            edgesCount++;
        }
        else {
            // Если в ОДНОЙ -> Это цикл, выкидываем
            // Анимация: "Не берем!" (Красный или Серый)
            steps.enqueue({ HighlightEdge, edge, Qt::red }); // Красный нагляднее: "Опасность, цикл!"
        }
    }

    return steps;
}