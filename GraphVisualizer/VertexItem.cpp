#include "VertexItem.h"
#include "Edge.h"


VertexItem::VertexItem(int id, QPointF position)
	: m_id(id)
{
	setFlags(ItemIsMovable | ItemIsSelectable | ItemSendsGeometryChanges);
	setPos(position);

	m_color = Qt::white;
}

QRectF VertexItem::boundingRect() const
{
	return QRectF(-m_radius - 2, -m_radius - 2, (m_radius * 2) + 4, (m_radius * 2) + 4);
}

void VertexItem::paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget)
{
	Q_UNUSED(option);	//макросы, чтобы компилятор не ругался, т.к. мы их не используем
	Q_UNUSED(widget);	//но сигнатура метода фикисирована
	
	painter->setBrush(m_color);
	painter->setPen(QPen(Qt::black, 2));

	if (isSelected()) {
		painter->setPen(QPen(Qt::red, 2));
	}

	painter->drawEllipse(-m_radius, -m_radius, m_radius * 2, m_radius * 2);

	painter->drawText(boundingRect(), Qt::AlignCenter, QString::number(m_id));	//ID будет в центре круга
}

QVariant VertexItem::itemChange(GraphicsItemChange change, const QVariant& value)
{
	if (change == ItemPositionChange && scene()) {
		for (Edge* edge : edgeList) {
			edge->adjust();
		}
	}
	return QGraphicsItem::itemChange(change, value);
}

void VertexItem::addEdge(Edge* edge)
{
	edgeList << edge;
	edge->adjust();
}

void VertexItem::setColor(QColor color)
{
	m_color = color;
	update();
}

bool VertexItem::isConnectedTo(const VertexItem* other) const
{
	for (Edge* edge : edgeList) {
		// Ребро соединяет нас с кем-то. Проверяем, с кем.
		if (edge->sourceNode() == other || edge->destNode() == other) {
			return true; // Связь уже есть
		}
	}
	return false; // Связи нет
}