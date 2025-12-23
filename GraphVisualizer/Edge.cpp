#include "Edge.h"
#include "VertexItem.h"

#include <QPen>
#include <QInputDialog>
#include <QtMath>

Edge::Edge(VertexItem* source, VertexItem* dest) 
	: source(source), dest(dest), m_weight(1)
{
	setZValue(-1);		//чтобы ребра были ЗА вершинами, а не перекрывали их

	adjust();
}

void Edge::setWeight(int w)
{
	m_weight = w;
	update(); // Команда перерисовать линию (чтобы цифра обновилась)
}

void Edge::mouseDoubleClickEvent(QGraphicsSceneMouseEvent* event)
{
	bool ok;

	// Так как мы сохранили файл в UTF-8, можно писать текст прямо так:
	QString title = "Редактирование ребра";

	QString label = QString("Вес ребра между вершинами %1 и %2:")
		.arg(source->getId())
		.arg(dest->getId());

	// Вызываем диалог
	int val = QInputDialog::getInt(nullptr, title, label,
		m_weight, 1, 10000, 1, &ok);

	if (ok) {
		setWeight(val);
	}

	QGraphicsItem::mouseDoubleClickEvent(event);
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

	QPointF center = (sourcePoint + destPoint) / 2.0;
	QRectF textRect(center.x() - 10, center.y() - 10, 20, 20);

	QFont font = painter->font();
	font.setBold(true);
	painter->setFont(font);

	painter->fillRect(textRect, QColor(255, 255, 255, 200));
	painter->setPen(Qt::blue);
	painter->drawText(textRect, Qt::AlignCenter, QString::number(m_weight));
}