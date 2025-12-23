#pragma once

#include <QGraphicsItem>
#include <QPainter>

#include "VertexItem.h"

class VertexItem;		//сообщаем компилятору, что такой класс есть (чтобы избежать циклического include)

class Edge : public QGraphicsItem
{
public:
	Edge(VertexItem* source, VertexItem* dest);

	void adjust();

	VertexItem* sourceNode() const{ return source; }
	VertexItem* destNode() const{ return dest; }

	QRectF boundingRect() const override;
	void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

private:
	VertexItem* source, * dest;
	QPointF sourcePoint;
	QPointF destPoint;
	qreal arrowSize = 10;
};

