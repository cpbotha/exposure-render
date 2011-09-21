
// Precompiled headers
#include "Stable.h"

#include "Aperture.h"

QAperture::QAperture(QObject* pParent /*= NULL*/) :
	QPresetXML(pParent),
	m_Size(0.05f)
{
}

QAperture::QAperture(const QAperture& Other)
{
	*this = Other;
}

QAperture& QAperture::operator=(const QAperture& Other)
{
	QPresetXML::operator=(Other);

	m_Size = Other.m_Size;

	emit Changed(*this);

	return *this;
}

float QAperture::GetSize(void) const
{
	return m_Size;
}

void QAperture::SetSize(const float& Size)
{
	m_Size = Size;

	emit Changed(*this);
}

void QAperture::Reset(void)
{
	m_Size = 0.05f;

	emit Changed(*this);
}

void QAperture::ReadXML(QDomElement& Parent)
{
	m_Size = Parent.firstChildElement("Size").attribute("Value").toFloat();
}

QDomElement QAperture::WriteXML(QDomDocument& DOM, QDomElement& Parent)
{
	// Aperture
	QDomElement Aperture = DOM.createElement("Aperture");
	Parent.appendChild(Aperture);

	// Size
	QDomElement Size = DOM.createElement("Size");
	Size.setAttribute("Value", m_Size);
	Aperture.appendChild(Size);

	return Aperture;
}