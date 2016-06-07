// ScreenCapDlg.h : 头文件
//

#pragma once
#include "PngImage.h"
#include "Resource.h"
#include "LabelObj.h"
#include "Draw.h"
#include <vector>
#include <GdiPlus.h>
#include <Gdiplusgraphics.h>
using namespace Gdiplus;
#pragma comment(lib, "gdiplus.lib")

using namespace std;

enum OprStatus
{
	Inital,		// 初始状态
	Capturing,	// 正在截图状态
	Captured,	// 选择完截图区域状态
	Drag,		// 拖拽状态
	Drawing,	// 正在绘制标注
	Draw		// 准备绘制标注状态
};

enum LineType
{
	LeftLine,	// 矩形框的左边线
	RightLine,	// 矩形框的右边线
	TopLine,	// 矩形框的上边线
	BottomLine	// 矩形框的下边线
};

enum ToolType
{
	NotTool=4,	// 未选中工具栏项(仅包含下面这四项的工具栏项的枚举)
	ToolDrop,	// 撤销
	ToolSaveAs,	// 另存为
	ToolCancel,	// 取消
	ToolConfirm	// 完成
};

typedef struct FloatPoint{
	float x;
	float y;
}FP;

typedef struct Line{
	FloatPoint Start;
	FloatPoint End;
}LE;

class ScreenCapDlg : public CDialog
{
protected:
	HWND				m_hwnd;				// 聊天窗口的句柄
	CString				m_path;				// 截图保存的路径
	int					m_saveType;			// 截图保存类型 0 文件形式保存 1 剪切板保存 2 两种方式都保存
	vector<LabelObj*>	m_labels;			// 已绘制的标注
	CPngImage2			m_pngMask;			// 绘制灰色透明背景的图片对象
	CPngImage2			m_pngDot;			// 绘制截图框可拉伸点的图片对象
	CPngImage2			m_pngAction;		// 绘制工具栏的图片对象
	CPngImage2			m_pngActionClick;	// 绘制被选中的工具栏的图片对象
	CBitmap				*m_backGround;		// 背景位图
	int					m_iCapture;			// 标记值,不太好描述
	bool				m_bLeftDown;		// 左键Down下标记
	CPoint				m_ptLButtonDown;	// 左键Down下的坐标值
	CRect				m_rcSel[9];			// m_rcSel[0],记录左上角点坐标和截图框的宽高。
											// m_rcSel[1]-m_rcSel[8]分别记录截图框上8个点，这些点处可以进行截图框的伸缩操作
	HCURSOR				m_curSel[9];		// m_curSel[0],记录鼠标在截图区以内鼠标箭头的样式。
											// m_curSel[1]-m_curSel[8] 分别记录鼠标在8个点上对应的鼠标箭头的样式
	CRect				m_rcAction[9];		// 工具栏上每项的区域坐标记录
	int					m_width;			// 屏幕宽
	int					m_height;			// 屏幕高
	CPoint				m_ptStopMove;		// 截图框停止移动时鼠标箭头的x,y坐标
	CPoint				m_ptDragPos;		// 拖拽截图最开始时，鼠标在截图区的位置
	CPoint				m_ptStartDraw;		// 是否可以开始绘制标注
	DrawLabel			m_draw;				// 标注绘制对象
	int					m_xScreen;			// 屏幕X方向的分辨率
	int					m_yScreen;			// 屏幕Y方向的分辨率
	CBitmap*			m_pBitmap;			// 背景位图
	HCURSOR				m_hCursor[3];		// 光标
	CPoint				m_startPt;			// 截取矩形左上角
	BOOL				m_bStartDraw;		// 是否可以开始绘制
	BOOL				m_bIsNotDrawing;	// 是否还没开始绘制
	OprStatus			m_status;			// 当前操作状态	
	CPoint				m_ptLineEndPos;		// 画线时结束时的点坐标
	LableType			m_eDrawType;		// 当前绘制的图形类型
	ToolType			m_eToolType;		// 当前被选中的工具栏项

public:
	ScreenCapDlg(CWnd* pParent = NULL);
	~ScreenCapDlg();
	enum { IDD = IDD_DIALOG_SCREENCAP };
	void			SetCapInfo(HWND hWnd, int saveType, LPSTR path);

protected:
	virtual void	DoDataExchange(CDataExchange* pDX);
	DECLARE_MESSAGE_MAP()

	virtual BOOL	OnInitDialog();
	afx_msg void	OnPaint();	
	void			PaintWindow();

public:
	afx_msg BOOL	OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message);
	afx_msg void	OnLButtonDown(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonUp(UINT nFlags, CPoint point);
	afx_msg void	OnLButtonDblClk(UINT nFlags, CPoint point);
	afx_msg void	OnRButtonDown(UINT nFlags, CPoint point);
	afx_msg void	OnOK();
	afx_msg void	OnCancel();
	afx_msg void	OnMouseMove(UINT nFlags, CPoint point);
	virtual BOOL	PreTranslateMessage(MSG* pMsg);

private:	
	// 绘制整个界面
	void			DrawFrame();
	// 绘制截图区域
	void			DrawCapturedFrame(CDC *DC, CDC* bufferDC);
	// 绘制工具栏
	void			DrawToolBar(CDC *DC,CDC* bufferDC);
	// 拷贝屏幕位图
	HBITMAP			CopyScreenToBitmap(LPRECT lpRect,BOOL bSave =FALSE );
	// 保存截图到文件
	void			SaveTo(CString strSaveFile="",CString ext=".png");
	// 选取截图区域时的绘制
	void			CapturingDraw(CPoint point);
	// captured状态时，鼠标移动箭头样式的控制
	void			CursorChangeWhenCaptured(CPoint point);
	// draw状态时，鼠标移动箭头样式的控制
	void			CursorChangeWhenDraw(CPoint point);
	// 拖拽时的绘制
	void			DragDraw(CPoint point);
	// drawing状态时的绘制
	void			LabelDraw(CPoint point);
	// 拖拽截图框移动时，相关变量的控制
	void			MoveToSide(int moveX,int moveY, CPoint point);
	// 获取两个线段的交点，根据斜率计算
	FloatPoint*		GetLineIntersection(Line RefLine1, Line RefLine2,LineType type);
	// 计算标注图形绘制所需的结束点
	CPoint			GetEndPos(CPoint point);
	// 判断当前鼠标箭头是否在工具栏区域
	int				IsInToolArea(CPoint point);
	// 获取截图框坐标，可能需要转换
	void			GetCapturedCoord(int& x, int& y, int& w, int& h);
	// 获取exe的执行路径
	CString			GetModuleDir();
	// 获取当前时间
	CString			OnGetTodayDate();
};
