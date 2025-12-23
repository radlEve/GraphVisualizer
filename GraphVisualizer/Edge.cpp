#include "Edge.h"
#include "VertexItem.h"

#include <QPen>

Edge::Edge(VertexItem* source, VertexItem* dest) 
	: source(source), dest(dest)
{
	setZValue(-1);		//чтобы ребра были ЗА вершинами, а не перекрывали их

	adjust();
}

void Edge::adjust()
{
	if (!source || !dest) return;

	prepareGeometryChange();
	
	sourcePoint = source->pos();
	destPoint = dest->pos();
}

QRectF Edge::boundingRect() const
{
	if (!source || !dest) return QRectF();
	qreal penWidth = 2;
	qreal extra = penWidth / 2.0;
	return QRectF(sourcePoint, QSizeF(destPoint.x() - sourcePoint.x(), destPoint.y() - sourcePoint.y())).normalized().adjusted(-extra, -extra, extra, extra);
}

void Edge::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	if (!source || !dest) return;

	QLineF line(sourcePoint, destPoint);

	if (qFuzzyCompare(line.length(), qreal(0.))) return;

	painter->setPen(QPen(Qt::black, 2, Qt::SolidLine, Qt::RoundCap, Qt::RoundJoin));
	painter->drawLine(line);
}