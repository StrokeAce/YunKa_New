// DrawLabel.cpp : 实现文件
//

#include "stdafx.h"
#include "Draw.h"


DrawLabel::DrawLabel()
{
}

DrawLabel::~DrawLabel()
{
}

void DrawLabel::DrawRect(CDC* dc,int left, int top, int width, int height)
{
	dc->Draw3dRect(left,top,width,height,RGB(22,177,234),RGB(22,177,234));
}

void DrawLabel::DrawText(CDC* dc, CString str,CRect rc)
{
	CFont font;
	font.CreateFont(-12,0,0,0,400,0,0,0,134,3,2,1,2,TEXT("宋体"));
	dc->SelectObject((HFONT)font);
	dc->SetBkMode(TRANSPARENT);
	dc->SetTextColor(RGB(255,255,255));	
	dc->DrawText(str,rc,DT_LEFT|DT_SINGLELINE|DT_VCENTER|DT_END_ELLIPSIS);
}

void DrawLabel::DrawObj(CDC* dc,LabelObj* obj)
{
	LableType type = obj->GetType();
	if(type == LabelLine)
	{
		CPoint startPos;
		CPoint endPos;
		((LineObj*)obj)->GetPos(startPos, endPos);

		HPEN pen = CreatePen(PS_DOT, 5, RGB(255,0,0));
		dc->SelectObject(pen);
		
		dc->MoveTo(endPos.x, endPos.y);
		dc->LineTo(startPos.x, startPos.y);

		::DeleteObject(pen);
	}
	else if (type == LabelRectangle)
	{ 
		CPoint startPos;
		CPoint endPos;
		((RectangleObj*)obj)->GetPos(startPos, endPos);

		HPEN pen = CreatePen(PS_DOT, 4, RGB(255,0,0));
		dc->SelectObject(pen);

		HBRUSH brush = (HBRUSH)GetStockObject(NULL_BRUSH);
		dc->SelectObject(brush);

		dc->Rectangle(startPos.x, startPos.y, endPos.x, endPos.y);

		::DeleteObject(pen);
		::DeleteObject(brush);
	}
	else if (type == LabelEllipse)
	{
		CPoint startPos;
		CPoint endPos;
		((EllipseObj*)obj)->GetPos(startPos, endPos);

		HPEN pen = CreatePen(PS_DOT, 4, RGB(255,0,0));
		dc->SelectObject(pen);

		HBRUSH brush = (HBRUSH)GetStockObject(NULL_BRUSH);
		dc->SelectObject(brush);

		dc->Ellipse(startPos.x, startPos.y, endPos.x, endPos.y);

		::DeleteObject(pen);
		::DeleteObject(brush);
	}
}

void DrawLabel::DrawLine( CDC* dc, POINT start, POINT end)
{
	HPEN pen = CreatePen(PS_DOT, 4, 0xFF3030);
	dc->SelectObject(pen);

	dc->MoveTo(end.x, end.y);
	dc->LineTo(start.x, start.y);

	end.x = start.x;
	end.y = start.y;

	::DeleteObject(pen);
}
