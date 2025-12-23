#pragma once

#include <QObject>
#include <QList>
#include <QColor>
#include <QQueue>
#include <QMap>

// Предварительное объявление, чтобы Solver знал, с чем работает
class VertexItem;
class Edge;

// Тип действия, которое нужно совершить в интерфейсе
enum StepType {
    HighlightNode, // Подсветить вершину
    HighlightEdge, // Подсветить ребро
    ResetColors    // Сбросить цвета (начало алгоритма)
};

// Структура одного шага анимации
struct AlgorithmStep {
    StepType type;
    void* item;    // Указатель на объект (VertexItem* или Edge*)
    QColor color;  // В какой цвет красить
};

class GraphSolver
{
public:
    GraphSolver();

    // Загрузка графа в "мозг" (получаем списки объектов со сцены)
    void setGraphData(QList<VertexItem*> vertices, QList<Edge*> edges);

    // --- Алгоритмы ---
    // Они возвращают очередь шагов, которые нужно проиграть
    QQueue<AlgorithmStep> runBFS(int startNodeId);

private:
    // Внутреннее хранилище ссылок на объекты
    QList<VertexItem*> m_vertices;
    QList<Edge*> m_edges;

    // Вспомогательный метод: найти вершину по ID
    VertexItem* findNodeById(int id);

    // Вспомогательный метод: найти ребра, выходящие из вершины
    QList<Edge*> getConnectedEdges(VertexItem* node);
};