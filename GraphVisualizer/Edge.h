#pragma once

#include <QGraphicsItem>
#include <QPainter>
#include <QGraphicsSceneMouseEvent>

#include "VertexItem.h"

class VertexItem;		//сообщаем компилятору, что такой класс есть (чтобы избежать циклического include)

class Edge : public QGraphicsItem
{
public:
	Edge(VertexItem* source, VertexItem* dest);

	void adjust();

	VertexItem* sourceNode() const{ return source; }
	VertexItem* destNode() const{ return dest; }

	void setWeight(int w);
	int getWeight() const { return m_weight; }

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

protected:
	void mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event) override;

private:
	VertexItem* source, * dest;
	QPointF sourcePoint;
	QPointF destPoint;

	qreal arrowSize = 10;
	int m_weight;
};

