#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GraphVisualizer.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include "VertexItem.h"
#include "GraphSolver.h"
#include <QQueue>
#include <QTimer> // Для автоматического воспроизведения (опционально)

class GraphVisualizer : public QMainWindow
{
    Q_OBJECT

public:
    GraphVisualizer(QWidget *parent = nullptr);
    ~GraphVisualizer();

public slots:
    void onClear(); // Слот очистки
    // Слоты для кнопок
    void onRunBFS();
    void onAutoPlay();
    void onNextStep();

private:
    Ui::GraphVisualizerClass ui;

    void setupScene();

    QGraphicsScene* scene;
    QGraphicsView* view;
    int nextId = 1;         //cчетчик для номеров вершин

    bool eventFilter(QObject* watched, QEvent* event) override;

    VertexItem* firstVertex = nullptr;

    GraphSolver solver;
    QQueue<AlgorithmStep> currentSteps; // Очередь шагов, которые надо выполнить

    // Метод выполнения одного шага
    void executeStep();

    QToolBar* toolbar;
    QAction* actClear;
    QAction* actRunBFS;
    QAction* actNextStep;

    QTimer* autoPlayTimer; // Таймер для анимации
    QAction* actAutoPlay;  // Кнопка Play/Pause

    // Инициализация интерфейса
    void setupUiCustom();
};

//метки времени

