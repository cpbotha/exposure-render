#pragma once

#include "Preset.h"
#include "Node.h"
#include "Histogram.h"

class QTransferFunction : public QPresetXML
{
    Q_OBJECT

public:
    QTransferFunction(QObject* pParent = NULL, const QString& Name = "Default");
	QTransferFunction(const QTransferFunction& Other);
	QTransferFunction& operator = (const QTransferFunction& Other);			
	
	void				AddNode(const float& Intensity, const float& Opacity, const QColor& Diffuse, const QColor& Specular, const QColor& Emission, const float& Roughness);
	void				AddNode(const QNode& pNode);
	void				RemoveNode(QNode* pNode);
	void				NormalizeIntensity(void);
	void				DeNormalizeIntensity(void);
	void				UpdateNodeRanges(void);
	const QNodeList&	GetNodes(void) const;
	QNode&				GetNode(const int& Index);
	void				SetSelectedNode(QNode* pSelectedNode);
	void				SetSelectedNode(const int& Index);
	QNode*				GetSelectedNode(void);
	void				SelectPreviousNode(void);
	void				SelectNextNode(void);
	int					GetNodeIndex(QNode* pNode);
	float				GetRangeMin(void);
	void				SetRangeMin(const float& RangeMin);
	float				GetRangeMax(void);
	void				SetRangeMax(const float& RangeMax);
	float				GetRange(void);
	void				ReadXML(QDomElement& Parent);
	QDomElement			WriteXML(QDomDocument& DOM, QDomElement& Parent);

	static QTransferFunction	Default(void);

private slots:
	void	OnNodeChanged(QNode* pNode);

signals:
	void	FunctionChanged(void);
	void	SelectionChanged(QNode* pNode);
	
private:
	QNodeList	m_Nodes;
	QNode*		m_pSelectedNode;
	float		m_RangeMin;
	float		m_RangeMax;
	float		m_Range;

	friend class QNode;
};

typedef QList<QTransferFunction> QTransferFunctionList;

// Transfer function singleton
extern QTransferFunction gTransferFunction;