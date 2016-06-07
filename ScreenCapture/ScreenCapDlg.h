// ScreenCapDlg.h : ͷ�ļ�
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
	Inital,		// ��ʼ״̬
	Capturing,	// ���ڽ�ͼ״̬
	Captured,	// ѡ�����ͼ����״̬
	Drag,		// ��ק״̬
	Drawing,	// ���ڻ��Ʊ�ע
	Draw		// ׼�����Ʊ�ע״̬
};

enum LineType
{
	LeftLine,	// ���ο�������
	RightLine,	// ���ο���ұ���
	TopLine,	// ���ο���ϱ���
	BottomLine	// ���ο���±���
};

enum ToolType
{
	NotTool=4,	// δѡ�й�������(����������������Ĺ��������ö��)
	ToolDrop,	// ����
	ToolSaveAs,	// ���Ϊ
	ToolCancel,	// ȡ��
	ToolConfirm	// ���
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
	HWND				m_hwnd;				// ���촰�ڵľ��
	CString				m_path;				// ��ͼ�����·��
	int					m_saveType;			// ��ͼ�������� 0 �ļ���ʽ���� 1 ���а屣�� 2 ���ַ�ʽ������
	vector<LabelObj*>	m_labels;			// �ѻ��Ƶı�ע
	CPngImage2			m_pngMask;			// ���ƻ�ɫ͸��������ͼƬ����
	CPngImage2			m_pngDot;			// ���ƽ�ͼ���������ͼƬ����
	CPngImage2			m_pngAction;		// ���ƹ�������ͼƬ����
	CPngImage2			m_pngActionClick;	// ���Ʊ�ѡ�еĹ�������ͼƬ����
	CBitmap				*m_backGround;		// ����λͼ
	int					m_iCapture;			// ���ֵ,��̫������
	bool				m_bLeftDown;		// ���Down�±��
	CPoint				m_ptLButtonDown;	// ���Down�µ�����ֵ
	CRect				m_rcSel[9];			// m_rcSel[0],��¼���Ͻǵ�����ͽ�ͼ��Ŀ�ߡ�
											// m_rcSel[1]-m_rcSel[8]�ֱ��¼��ͼ����8���㣬��Щ�㴦���Խ��н�ͼ�����������
	HCURSOR				m_curSel[9];		// m_curSel[0],��¼����ڽ�ͼ����������ͷ����ʽ��
											// m_curSel[1]-m_curSel[8] �ֱ��¼�����8�����϶�Ӧ������ͷ����ʽ
	CRect				m_rcAction[9];		// ��������ÿ������������¼
	int					m_width;			// ��Ļ��
	int					m_height;			// ��Ļ��
	CPoint				m_ptStopMove;		// ��ͼ��ֹͣ�ƶ�ʱ����ͷ��x,y����
	CPoint				m_ptDragPos;		// ��ק��ͼ�ʼʱ������ڽ�ͼ����λ��
	CPoint				m_ptStartDraw;		// �Ƿ���Կ�ʼ���Ʊ�ע
	DrawLabel			m_draw;				// ��ע���ƶ���
	int					m_xScreen;			// ��ĻX����ķֱ���
	int					m_yScreen;			// ��ĻY����ķֱ���
	CBitmap*			m_pBitmap;			// ����λͼ
	HCURSOR				m_hCursor[3];		// ���
	CPoint				m_startPt;			// ��ȡ�������Ͻ�
	BOOL				m_bStartDraw;		// �Ƿ���Կ�ʼ����
	BOOL				m_bIsNotDrawing;	// �Ƿ�û��ʼ����
	OprStatus			m_status;			// ��ǰ����״̬	
	CPoint				m_ptLineEndPos;		// ����ʱ����ʱ�ĵ�����
	LableType			m_eDrawType;		// ��ǰ���Ƶ�ͼ������
	ToolType			m_eToolType;		// ��ǰ��ѡ�еĹ�������

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
	// ������������
	void			DrawFrame();
	// ���ƽ�ͼ����
	void			DrawCapturedFrame(CDC *DC, CDC* bufferDC);
	// ���ƹ�����
	void			DrawToolBar(CDC *DC,CDC* bufferDC);
	// ������Ļλͼ
	HBITMAP			CopyScreenToBitmap(LPRECT lpRect,BOOL bSave =FALSE );
	// �����ͼ���ļ�
	void			SaveTo(CString strSaveFile="",CString ext=".png");
	// ѡȡ��ͼ����ʱ�Ļ���
	void			CapturingDraw(CPoint point);
	// captured״̬ʱ������ƶ���ͷ��ʽ�Ŀ���
	void			CursorChangeWhenCaptured(CPoint point);
	// draw״̬ʱ������ƶ���ͷ��ʽ�Ŀ���
	void			CursorChangeWhenDraw(CPoint point);
	// ��קʱ�Ļ���
	void			DragDraw(CPoint point);
	// drawing״̬ʱ�Ļ���
	void			LabelDraw(CPoint point);
	// ��ק��ͼ���ƶ�ʱ����ر����Ŀ���
	void			MoveToSide(int moveX,int moveY, CPoint point);
	// ��ȡ�����߶εĽ��㣬����б�ʼ���
	FloatPoint*		GetLineIntersection(Line RefLine1, Line RefLine2,LineType type);
	// �����עͼ�λ�������Ľ�����
	CPoint			GetEndPos(CPoint point);
	// �жϵ�ǰ����ͷ�Ƿ��ڹ���������
	int				IsInToolArea(CPoint point);
	// ��ȡ��ͼ�����꣬������Ҫת��
	void			GetCapturedCoord(int& x, int& y, int& w, int& h);
	// ��ȡexe��ִ��·��
	CString			GetModuleDir();
	// ��ȡ��ǰʱ��
	CString			OnGetTodayDate();
};
