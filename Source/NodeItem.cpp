
// Precompiled headers
#include "Stable.h"

#include "NodeItem.h"
#include "TransferFunction.h"
#include "TransferFunctionCanvas.h"

#define NODE_POSITION_EPSILON 0.01f

float	QNodeItem::m_Radius			= 4.0f;
QBrush	QNodeItem::m_BrushNormal	= QBrush(QColor::fromHsl(200, 100, 150));
QBrush	QNodeItem::m_BrushHighlight	= QBrush(QColor::fromHsl(0, 100, 150));
QBrush	QNodeItem::m_BrushDisabled	= QBrush(QColor::fromHsl(0, 0, 100));

QPen	QNodeItem::m_PenNormal		= QPen(QBrush(QColor::fromHsl(0, 100, 150)), 1.3);
QPen	QNodeItem::m_PenHighlight	= QPen(QBrush(QColor::fromHsl(100, 100, 150)), 1.8);
QPen	QNodeItem::m_PenDisabled	= QPen(QBrush(QColor::fromHsl(0, 0, 150)), 1.3);

QNodeItem::QNodeItem(QTransferFunctionCanvas* pTransferFunctionCanvas, QNode* pNode) :
	QGraphicsEllipseItem(pTransferFunctionCanvas),
	m_pTransferFunctionCanvas(pTransferFunctionCanvas),
	m_pNode(pNode),
	m_Cursor(),
	m_LastPos(),
	m_CachePen(),
	m_CacheBrush(),
	m_SuspendUpdate(false)
{
	// Styling
	setBrush(QNodeItem::m_BrushNormal);
	setPen(QNodeItem::m_PenNormal);

	// Make item movable
	setFlag(QGraphicsItem::ItemIsMovable);
	setFlag(QGraphicsItem::ItemSendsGeometryChanges);
	setFlag(QGraphicsItem::ItemIsSelectable);

	// Tooltip
	UpdateTooltip();
};

QVariant QNodeItem::itemChange(GraphicsItemChange Change, const QVariant& Value)
{
	QPointF NewScenePoint = Value.toPointF();
 
	if (!m_SuspendUpdate && Change == QGraphicsItem::ItemPositionChange)
	{
		QPointF NodeRangeMin = m_pTransferFunctionCanvas->TransferFunctionToScene(QPointF(m_pNode->GetMinX(), m_pNode->GetMinY()));
		QPointF NodeRangeMax = m_pTransferFunctionCanvas->TransferFunctionToScene(QPointF(m_pNode->GetMaxX(), m_pNode->GetMaxY()));

		NewScenePoint.setX(qMin(NodeRangeMax.x() - NODE_POSITION_EPSILON, qMax(NewScenePoint.x(), NodeRangeMin.x() + NODE_POSITION_EPSILON)));
		NewScenePoint.setY(qMin(NodeRangeMin.y(), qMax(NewScenePoint.y(), NodeRangeMax.y())));

		return NewScenePoint;
	}

	if (!m_SuspendUpdate && Change == QGraphicsItem::ItemPositionHasChanged)
	{
		QPointF NewTfPoint = m_pTransferFunctionCanvas->SceneToTransferFunction(NewScenePoint);

		m_pTransferFunctionCanvas->m_AllowUpdateNodes = false;

		m_pNode->SetIntensity(NewTfPoint.x());
		m_pNode->SetOpacity(NewTfPoint.y());

		m_pTransferFunctionCanvas->m_AllowUpdateNodes = true;

		return NewScenePoint;
	}

    return QGraphicsItem::itemChange(Change, Value);
}

void QNodeItem::paint(QPainter* pPainter, const QStyleOptionGraphicsItem* pOption, QWidget* pWidget)
{
	if (isEnabled())
	{
		if (isUnderMouse() || isSelected())
		{
			setBrush(m_BrushHighlight);
			setPen(m_PenHighlight);
		}
		else
		{
			setBrush(m_BrushNormal);
			setPen(m_PenNormal);
		}
	}
	else
	{
		setBrush(m_BrushDisabled);
		setPen(m_PenDisabled);
	}

	QGraphicsEllipseItem::paint(pPainter, pOption, pWidget);
}

void QNodeItem::mousePressEvent(QGraphicsSceneMouseEvent* pEvent)
{
	QGraphicsItem::mousePressEvent(pEvent);

	if (pEvent->button() == Qt::LeftButton)
	{
		if (gTransferFunction.GetNodes().indexOf(*m_pNode) == 0 || gTransferFunction.GetNodes().indexOf(*m_pNode) == gTransferFunction.GetNodes().size() - 1)
		{
			m_Cursor.setShape(Qt::SizeVerCursor);
			setCursor(m_Cursor);
		}
		else
		{
			m_Cursor.setShape(Qt::SizeAllCursor);
			setCursor(m_Cursor);
		}
	}
}

void QNodeItem::mouseReleaseEvent(QGraphicsSceneMouseEvent* pEvent)
{
	QGraphicsEllipseItem::mouseReleaseEvent(pEvent);

	// Change the cursor shape to normal
	m_Cursor.setShape(Qt::ArrowCursor);
	setCursor(m_Cursor);
}

void QNodeItem::mouseMoveEvent(QGraphicsSceneMouseEvent* pEvent)
{
	QGraphicsItem::mouseMoveEvent(pEvent);
}

void QNodeItem::setPos(const QPointF& Pos)
{
	QGraphicsEllipseItem::setPos(Pos);

	QRectF EllipseRect;

	EllipseRect.setTopLeft(QPointF(-m_Radius, -m_Radius));
	EllipseRect.setWidth(2.0f * m_Radius);
	EllipseRect.setHeight(2.0f * m_Radius);

	setRect(EllipseRect);
}

void QNodeItem::UpdateTooltip(void)
{
	QString ToolTipString;

	const QString R = QString::number(m_pNode->GetDiffuse().red());
	const QString G = QString::number(m_pNode->GetDiffuse().green());
	const QString B = QString::number(m_pNode->GetDiffuse().blue());

	ToolTipString.append("<table>");
		ToolTipString.append("<tr>");
			ToolTipString.append("<td>Node</td><td>:</td>");
			ToolTipString.append("<td>" + QString::number(1) + "</td>");
		ToolTipString.append("</tr>");
		ToolTipString.append("<tr>");
			ToolTipString.append("<td>Position</td><td> : </td>");
			ToolTipString.append("<td>" + QString::number(m_pNode->GetIntensity()) + "</td>");
		ToolTipString.append("</tr>");
		ToolTipString.append("<tr>");
			ToolTipString.append("<td>Opacity</td><td> : </td>");
			ToolTipString.append("<td>" + QString::number(m_pNode->GetOpacity()) + "</td>");
		ToolTipString.append("</tr>");
		ToolTipString.append("<tr>");
			ToolTipString.append("<td>Color</td><td> : </td>");
			ToolTipString.append("<td style='color:rgb(" + R + ", " + G + ", " + B + ")'><b>");
				ToolTipString.append("<style type='text/css'>backgournd {color:red;}</style>");
				ToolTipString.append("[");
					ToolTipString.append(R + ", ");
					ToolTipString.append(G + ", ");
					ToolTipString.append(B);
				ToolTipString.append("]");
			ToolTipString.append("</td></b>");
		ToolTipString.append("</tr>");
	ToolTipString.append("</table>");

	// Update the tooltip
	setToolTip(ToolTipString);
}