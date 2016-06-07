#pragma once

#include "LabelObj.h"

class DrawLabel
{
public:
	DrawLabel();
	virtual ~DrawLabel();

	void DrawRect(CDC* dc,int left, int top, int width, int height);

	void DrawText(CDC* dc, CString str,CRect rc);

	void DrawLine(CDC* dc, POINT start, POINT end);

	void DrawObj(CDC* dc, LabelObj* obj);
};


