#pragma once

enum LableType
{
	NotLabel = -1,	// 非标注	
	LabelRectangle,	// 矩形标注
	LabelEllipse,	// 椭圆标注
	LabelLine,		// 线形标注	
	LabelPen,
	LabelText
};

class LabelObj
{
protected:
	LableType m_type;
	CPoint m_startPos;
	CPoint m_endPos;
public:
	LabelObj(LableType type, CPoint startPos, CPoint endPos);
	virtual ~LabelObj();
	LableType	GetType();
	void		GetPos( CPoint& startPos,CPoint& endPos );
};

class LineObj : public LabelObj
{
public:
	LineObj(CPoint startPos,CPoint endPos);
	~LineObj(){};
};

class RectangleObj : public LabelObj
{
public:
	RectangleObj(CPoint startPos,CPoint endPos);
	~RectangleObj(){};
};

class EllipseObj : public LabelObj
{
public:
	EllipseObj(CPoint startPos,CPoint endPos);
	~EllipseObj(){};
};


