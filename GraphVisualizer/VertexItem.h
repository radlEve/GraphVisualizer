#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <QBrush>
#include <QPen>
#include <QList>


class Edge;

class VertexItem : public QGraphicsItem
{
public:
	VertexItem(int id, QPointF position);

	void addEdge(Edge* edge);

	void removeEdgeFromList(Edge* edge);

	QList<Edge*>& getEdges() { return edgeList; }

	void setColor(QColor color);

	bool isConnectedTo(const VertexItem* other) const;

	QRectF boundingRect() const override;

	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

	int getId() const { return m_id; }

protected:
	QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

private:
	int m_id;				//Уникальный номер вершины
	const int m_radius = 20;	//Радиус круга
	QList<Edge*> edgeList;
	QColor m_color;
};

