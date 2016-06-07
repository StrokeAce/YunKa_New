#pragma once

enum LableType
{
	NotLabel = -1,	// �Ǳ�ע	
	LabelRectangle,	// ���α�ע
	LabelEllipse,	// ��Բ��ע
	LabelLine,		// ���α�ע	
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


