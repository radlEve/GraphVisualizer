#pragma once

#include <QtWidgets/QMainWindow>
#include "ui_GraphVisualizer.h"

#include <QGraphicsScene>
#include <QGraphicsView>
#include "VertexItem.h"

class GraphVisualizer : public QMainWindow
{
    Q_OBJECT

public:
    GraphVisualizer(QWidget *parent = nullptr);
    ~GraphVisualizer();

private:
    Ui::GraphVisualizerClass ui;

    void setupScene();

    QGraphicsScene* scene;
    QGraphicsView* view;
    int nextId = 1;         //cчетчик для номеров вершин

    bool eventFilter(QObject* watched, QEvent* event) override;

    VertexItem* firstVertex = nullptr;
};

//метки времени

