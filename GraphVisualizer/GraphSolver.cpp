#include "GraphSolver.h"
#include "VertexItem.h"
#include "Edge.h"
#include <QQueue>
#include <QSet>
#include <QStack>

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