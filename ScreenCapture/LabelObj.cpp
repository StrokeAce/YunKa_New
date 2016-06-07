#include "stdafx.h"
#include "ScreenCapture.h"
#include "LabelObj.h"


LabelObj::LabelObj(LableType type, CPoint startPos, CPoint endPos)
{
	m_type = type;
	m_startPos = startPos;
	m_endPos = endPos;
}

LabelObj::~LabelObj()
{
}

LableType LabelObj::GetType()
{
	return m_type;
}

void LabelObj::GetPos( CPoint& startPos,CPoint& endPos )
{
	startPos = m_startPos;
	endPos = m_endPos;
}

LineObj::LineObj( CPoint startPos,CPoint endPos): LabelObj(LabelLine,startPos, endPos)
{	
}

RectangleObj::RectangleObj(CPoint startPos,CPoint endPos): LabelObj(LabelRectangle,startPos, endPos)
{
}

EllipseObj::EllipseObj(CPoint startPos,CPoint endPos): LabelObj(LabelEllipse,startPos, endPos)
{
	
}