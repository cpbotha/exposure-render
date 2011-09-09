#pragma once

#include <QtGui>

#include "Preset.h"

class QFocus : public QPresetXML
{
	Q_OBJECT

public:
	QFocus(QObject* pParent = NULL);
	QFocus::QFocus(const QFocus& Other);
	QFocus& QFocus::operator=(const QFocus& Other);

	int				GetFocalDistance(void) const;
	void			SetFocalDistance(const int& FocalDistance);
	void			ReadXML(QDomElement& Parent);
	QDomElement		WriteXML(QDomDocument& DOM, QDomElement& Parent);

signals:
	void Changed(void);

private:
	float			m_FocalDistance;
};
