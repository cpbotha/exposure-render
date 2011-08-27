
#include "TransferFunctionView.h"
#include "TransferFunction.h"
#include "NodeItem.h"

// Compare two nodes
bool CompareNodes(QNode* pNodeA, QNode* pNodeB)
{
	return pNodeA->GetPosition() < pNodeB->GetPosition();
}

// Compare two node items
bool CompareNodeItems(QNodeItem* pNodeItemA, QNodeItem* pNodeItemB)
{
	return pNodeItemA->m_pNode->GetPosition() < pNodeItemB->m_pNode->GetPosition();
}

QTransferFunctionView::QTransferFunctionView(QWidget* pParent, QTransferFunction* pTransferFunction) :
	QGraphicsView(pParent),
	m_pGraphicsScene(NULL),
	m_pTransferFunction(pTransferFunction),
	m_pPolygon(NULL),
	m_pOutline(NULL),
	m_pCanvas(NULL),
	m_Margin(20.0f)
{
	// Dimensions
//	setFixedHeight(170);

	// Styling
	setFrameShadow(Sunken);
	setFrameShape(NoFrame);

	// Never show scrollbars
	setVerticalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);
	setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy::ScrollBarAlwaysOff);

	// Status and tooltip
	setStatusTip("Transfer function editor");
	setToolTip("Transfer function editor");

	// Create scene and apply
	m_pGraphicsScene = new QGraphicsScene(this);
	setScene(m_pGraphicsScene);

	// Turn antialiasing on
//	setRenderHint(QPainter::Antialiasing);

	// Setup connections
	connect(m_pTransferFunction, SIGNAL(FunctionChanged()), this, SLOT(Update()));
	connect(m_pTransferFunction, SIGNAL(NodeAdd(QNode*)), this, SLOT(OnNodeAdd(QNode*)));

	// Respond to changes in node selection
	connect(m_pTransferFunction, SIGNAL(SelectionChanged(QNode*)), this, SLOT(OnNodeSelectionChanged(QNode*)));

	// Add polygon graphics item
	m_pPolygon = new QGraphicsPolygonItem;

	// Ensure the item is drawn in the right order
	m_pPolygon->setZValue(500);

	// Add the polygon
	m_pGraphicsScene->addItem(m_pPolygon);

	// Create rectangle for outline
	m_pOutline = new QGraphicsRectItem;
	m_pOutline->setRect(rect());
	m_pOutline->setBrush(QBrush(Qt::BrushStyle::NoBrush));
	m_pOutline->setPen(QPen(Qt::darkGray));

	// Ensure the item is drawn in the right order
	m_pOutline->setZValue(1000);

	// Add the rectangle
//	m_pGraphicsScene->addItem(m_pOutline);

	// Create canvas
	m_pCanvas = new QGraphicsRectItem;
	m_pCanvas->setRect(rect());
	m_pCanvas->setBrush(QBrush(QColor(190, 190, 190)));
	m_pCanvas->setPen(QPen(Qt::darkGray));

	// Add the rectangle
	m_pGraphicsScene->addItem(m_pCanvas);

	// Render
	Update();
	UpdateNodes();

	// Configure pen
	m_GridPenHorizontal.setStyle(Qt::PenStyle::DashLine);
	m_GridPenHorizontal.setColor(QColor(75, 75, 75, 120));
//	m_GridPenHorizontal.setWidthF(1.0f);

	m_Text = new QGraphicsTextItem();
	m_Text->setPos(m_EditRect.bottomLeft());
	m_Text->setTextWidth(m_EditRect.width());
	m_Text->setHtml("<center>Density</center>");

//	m_pGraphicsScene->addItem(m_Text);
}

void QTransferFunctionView::drawBackground(QPainter* pPainter, const QRectF& Rectangle)
{
	
	QGraphicsView::drawBackground(pPainter, Rectangle);
	
	setBackgroundBrush(QBrush(QColor(240, 240, 240)));
}

void QTransferFunctionView::resizeEvent(QResizeEvent* pResizeEvent)
{
	QGraphicsView::resizeEvent(pResizeEvent);

	Update();
	UpdateGrid();
}

void QTransferFunctionView::mousePressEvent(QMouseEvent* pEvent)
{
	QGraphicsView::mousePressEvent(pEvent);

	QNodeItem* pNodeGraphics = dynamic_cast<QNodeItem*>(itemAt(pEvent->pos()));

	if (!pNodeGraphics)
	{
		QPointF TfPoint = SceneToTf(m_pCanvas->mapFromScene(pEvent->posF()));

		int R = (int)(((float)rand() / (float)RAND_MAX) * 255.0f);
		int G = (int)(((float)rand() / (float)RAND_MAX) * 255.0f);
		int B = (int)(((float)rand() / (float)RAND_MAX) * 255.0f);

		// Create new transfer function node
		QNode* pNode = new QNode(m_pTransferFunction, TfPoint.x(), TfPoint.y(), QColor(R, G, B, 255));

		// Add to node list
		m_pTransferFunction->AddNode(pNode);

		// Select it immediately
		SetSelectedNode(pNode);
	}
	else
	{
		SetSelectedNode(pNodeGraphics->m_pNode);
	}
}

void QTransferFunctionView::UpdateCanvas(void)
{
	m_EditRect = rect();
	m_EditRect.adjust(m_Margin, m_Margin, -m_Margin, -m_Margin);

	// Update canvas
	m_pCanvas->setRect(m_EditRect);
}

void QTransferFunctionView::UpdateGrid(void)
{
	// Horizontal grid lines
	const float DeltaY = 0.1f * m_EditRect.height();

	// Remove old horizontal grid lines
	foreach(QGraphicsLineItem* pLine, m_GridLinesHorizontal)
		m_pGraphicsScene->removeItem(pLine);

	// Clear the edges list
	m_GridLinesHorizontal.clear();

	for (int i = 1; i < 10; i++)
	{
		QGraphicsLineItem* pLine = new QGraphicsLineItem(QLineF(QPointF(m_EditRect.left(), m_EditRect.top() + i * DeltaY), QPointF(m_EditRect.right(), m_EditRect.top() + i * DeltaY)));

		pLine->setPen(m_GridPenHorizontal);

		m_pGraphicsScene->addItem(pLine);
		m_GridLinesHorizontal.append(pLine);
	}
}

void QTransferFunctionView::UpdateHistogram(void)
{
}

void QTransferFunctionView::UpdateEdges(void)
{
	// Remove old edges
	foreach(QGraphicsLineItem* pLine, m_Edges)
		m_pGraphicsScene->removeItem(pLine);

	// Clear the edges list
	m_Edges.clear();

	for (int i = 1; i < m_Nodes.size(); i++)
	{
		QPointF PtFrom(m_Nodes[i - 1]->rect().center());
		QPointF PtTo(m_Nodes[i]->rect().center());

		QGraphicsLineItem* pLine = new QGraphicsLineItem(QLineF(PtFrom, PtTo));
		
		// Set the pen
		pLine->setPen(QPen(QColor(110, 110, 100), 0.5));

		// Ensure the item is drawn in the right order
		pLine->setZValue(800);

		m_Edges.append(pLine);
		m_pGraphicsScene->addItem(pLine);
	}
}

void QTransferFunctionView::UpdateNodes(void)
{
	// Set the gradient stops
	foreach(QNodeItem* pNodeGraphics, m_Nodes)
	{
		QPointF SceneCenter = TfToScene(QPointF(pNodeGraphics->m_pNode->GetX(), pNodeGraphics->m_pNode->GetY()));

//		Center.setX(m_EditRect.left() + m_EditRect.width() * pNodeGraphics->m_pNode->GetNormalizedX());
//		Center.setY(m_EditRect.top() + m_EditRect.height() - (pNodeGraphics->m_pNode->GetOpacity() * m_EditRect.height()));
		
//		pNodeGraphics->SetCenter(SceneCenter);
	}

	m_pOutline->setRect(m_EditRect);
}

void QTransferFunctionView::UpdateNodeRanges(void)
{
	if (m_pTransferFunction->m_Nodes.size() < 2)
		return;

	for (int i = 0; i < m_pTransferFunction->m_Nodes.size(); i++)
	{
		QNode* pNode = m_pTransferFunction->m_Nodes[i];

		if (pNode == m_pTransferFunction->m_Nodes.front())
		{
			pNode->m_MinX = 0.0f;
			pNode->m_MaxX = 0.0f;
		}
		else if (pNode == m_pTransferFunction->m_Nodes.back())
		{
			pNode->m_MinX = m_pTransferFunction->m_RangeMax;
			pNode->m_MaxX = m_pTransferFunction->m_RangeMax;
		}
		else
		{
			QNode* pNodeLeft	= m_pTransferFunction->m_Nodes[i - 1];
			QNode* pNodeRight	= m_pTransferFunction->m_Nodes[i + 1];

			pNode->m_MinX = pNodeLeft->GetPosition();
			pNode->m_MaxX = pNodeRight->GetPosition();
		}
	}
}

void QTransferFunctionView::UpdateGradient(void)
{
	m_LinearGradient.setStart(m_EditRect.left(), m_EditRect.top());
	m_LinearGradient.setFinalStop(m_EditRect.right(), m_EditRect.top());

	QGradientStops GradientStops;

	// Set the gradient stops
	foreach(QNode* pNode, m_pTransferFunction->m_Nodes)
	{
		QColor Color = pNode->GetColor();

		// Clamp node opacity to obtain valid alpha for display
		float Alpha = qMin(1.0f, qMax(0.0f, pNode->GetOpacity()));

		Color.setAlphaF(0.9f);

		// Add a new gradient stop
		GradientStops.append(QGradientStop(pNode->GetNormalizedX(), Color));
	}

	m_LinearGradient.setStops(GradientStops);
}

void QTransferFunctionView::UpdatePolygon(void)
{
	QPolygonF Polygon;

	// Set the gradient stops
	for (int i = 0; i < m_Nodes.size(); i++)
	{
		QNodeItem* pNodeGraphics = m_Nodes[i];

		// Compute polygon point in scene coordinates
		QPointF ScenePoint = TfToScene(QPointF(pNodeGraphics->m_pNode->GetX(), pNodeGraphics->m_pNode->GetY()));

		if (pNodeGraphics == m_Nodes.front())
		{
			QPointF CenterCopy = ScenePoint;

			CenterCopy.setY(m_EditRect.bottom());

			Polygon.append(CenterCopy);
		}

		Polygon.append(ScenePoint);

		if (pNodeGraphics == m_Nodes.back())
		{
			QPointF CenterCopy = ScenePoint;

			CenterCopy.setY(m_EditRect.bottom());

			Polygon.append(CenterCopy);
		}
	}

	m_pPolygon->setPolygon(Polygon);
	m_pPolygon->setBrush(QBrush(m_LinearGradient));
	m_pPolygon->setPen(QPen(Qt::PenStyle::NoPen));
}

void QTransferFunctionView::Update(void)
{
	setSceneRect(rect());

	UpdateCanvas();
	UpdateHistogram();
	UpdateNodes();
	UpdateNodeRanges();
	UpdateEdges();
	UpdateGradient();
	UpdatePolygon();
}

void QTransferFunctionView::OnNodeAdd(QNode* pTransferFunctionNode)
{
	QNodeItem* pNodeItem = new QNodeItem(NULL, pTransferFunctionNode, this);
	
	// Ensure the item is drawn in the right order
	pNodeItem->setZValue(900);

	m_pGraphicsScene->addItem(pNodeItem);

	m_Nodes.append(pNodeItem);
	
	qSort(m_Nodes.begin(), m_Nodes.end(), CompareNodeItems);
	qSort(m_pTransferFunction->m_Nodes.begin(), m_pTransferFunction->m_Nodes.end(), CompareNodes);

	UpdateNodes();
}

QPointF QTransferFunctionView::SceneToTf(const QPointF& ScenePoint)
{
	const float NormalizedX = (ScenePoint.x() - (float)rect().left()) / (float)rect().width();
	const float NormalizedY = 1.0f - ((ScenePoint.y() - (float)rect().top()) / (float)rect().height());

	const float TfX = m_pTransferFunction->m_RangeMin + NormalizedX * m_pTransferFunction->m_Range;
	const float TfY = NormalizedY;

	return QPointF(TfX, TfY);
}

QPointF QTransferFunctionView::TfToScene(const QPointF& TfPoint)
{
	const float NormalizedX = (TfPoint.x() - m_pTransferFunction->m_RangeMin) / m_pTransferFunction->m_Range;
	const float NormalizedY = 1.0f - TfPoint.y();

	const float SceneX = m_EditRect.left() + NormalizedX * m_EditRect.width();
	const float SceneY = m_EditRect.top() + NormalizedY * m_EditRect.height();

	return QPointF(SceneX, SceneY);
}

void QTransferFunctionView::SetSelectedNode(QNode* pSelectedNode)
{
	m_pTransferFunction->SetSelectedNode(pSelectedNode);
}

void QTransferFunctionView::OnNodeSelectionChanged(QNode* pNode)
{
	if (pNode)
	{
		// Deselect all nodes
		foreach (QNodeItem* pNode, m_Nodes)
			pNode->setSelected(false);

		// Obtain node index
		const int NodeIndex = m_pTransferFunction->GetNodeIndex(pNode);

		// Select the node
		if (NodeIndex >= 0)
		{
			m_Nodes[NodeIndex]->setSelected(true);
			m_Nodes[NodeIndex]->update();
		}
	}
	else
	{
	}
}