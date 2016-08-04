#ifndef _UI_USER_LIST_H_
#define _UI_USER_LIST_H_

#pragma once


inline double CalculateDelay(double state) {
	return pow(state, 2);
}

class UserListUI : public CListUI
{
public:
	enum { SCROLL_TIMERID = 10 };

	struct NodeData
	{
		int _level;
		unsigned long  _uid;
		bool _expand;
		CDuiString _text;
		string     _sid;
		CListLabelElementUI* _pListElement;
	};

	class Node
	{
		typedef std::vector <Node*>	Children;
		Children	_children;
		Node*		_parent;
		NodeData    _data;

	private:
		void set_parent(Node* parent) { _parent = parent; }

	public:
		Node() : _parent(NULL) {}
		explicit Node(NodeData t) : _data(t), _parent(NULL) {}
		Node(NodeData t, Node* parent) : _data(t), _parent(parent) {}
		~Node()
		{
			for (int i = 0; i < num_children(); i++)
				delete _children[i];
		}
		NodeData& data() { return _data; }
		int num_children() const { return _children.size(); }
		Node* child(int i)	{ return _children[i]; }
		Node* parent() { return (_parent); }
		bool has_children() const { return num_children() > 0; }
		void add_child(Node* child)
		{
			child->set_parent(this);
			_children.push_back(child);
		}
		void remove_child(Node* child)
		{
			Children::iterator iter = _children.begin();
			for (; iter < _children.end(); ++iter)
			{
				if (*iter == child)
				{
					_children.erase(iter);
					return;
				}
			}
		}
		Node* get_last_child()
		{
			if (has_children())
			{
				return child(num_children() - 1)->get_last_child();
			}
			else return this;
		}
	};

	UserListUI() : _root(NULL), m_dwDelayDeltaY(0), m_dwDelayNum(0), m_dwDelayLeft(0)
	{
		SetItemShowHtml(true);

		_root = new Node;
		_root->data()._uid = 0;
		_root->data()._level = -1;
		_root->data()._expand = true;
		_root->data()._pListElement = NULL;
		_root->data()._sid = "";

		m_listBKImage = _T("");
		m_listName = _T("");
	}

	~UserListUI() { if (_root) delete _root; }

	bool Add(CControlUI* pControl)
	{
		if (!pControl) return false;
		if (_tcscmp(pControl->GetClass(), _T("ListLabelElementUI")) != 0) return false;

		return CListUI::Add(pControl);
	}

	bool AddAt(CControlUI* pControl, int iIndex)
	{
		if (!pControl) return false;
		if (_tcscmp(pControl->GetClass(), _T("ListLabelElementUI")) != 0) return false;

		return CListUI::AddAt(pControl, iIndex);
	}

	bool Remove(CControlUI* pControl)
	{
		if (!pControl) return false;
		if (_tcscmp(pControl->GetClass(), _T("ListLabelElementUI")) != 0) return false;

		if (reinterpret_cast<Node*>(static_cast<CListLabelElementUI*>(pControl->GetInterface(_T("ListLabelElement")))->GetTag()) == NULL)
			return CListUI::Remove(pControl);
		else
			return RemoveNode(reinterpret_cast<Node*>(static_cast<CListLabelElementUI*>(pControl->GetInterface(_T("ListLabelElement")))->GetTag()));
	}

	bool RemoveAt(int iIndex)
	{
		CControlUI* pControl = GetItemAt(iIndex);
		if (!pControl) return false;
		if (_tcscmp(pControl->GetClass(), _T("ListLabelElementUI")) != 0) return false;

		if (reinterpret_cast<Node*>(static_cast<CListLabelElementUI*>(pControl->GetInterface(_T("ListLabelElement")))->GetTag()) == NULL)
			return CListUI::RemoveAt(iIndex);
		else
			return RemoveNode(reinterpret_cast<Node*>(static_cast<CListLabelElementUI*>(pControl->GetInterface(_T("ListLabelElement")))->GetTag()));
	}

	void RemoveAll()
	{
		CListUI::RemoveAll();
		for (int i = 0; i < _root->num_children(); ++i)
		{
			Node* child = _root->child(i);
			RemoveNode(child);
		}
		delete _root;
		_root = new Node;

		_root->data()._uid = -1;
		_root->data()._level = -1;
		_root->data()._expand = true;
		_root->data()._pListElement = NULL;
		_root->data()._sid = "";
	}
	void SetVisible(bool bVisible = true)
	{
		if (m_bVisible == bVisible) return;
		CControlUI::SetVisible(bVisible);
	}

	void SetInternVisible(bool bVisible = true)
	{
		CControlUI::SetInternVisible(bVisible);
	}

	void DoEvent(TEventUI& event)
	{
		if (!IsMouseEnabled() && event.Type > UIEVENT__MOUSEBEGIN && event.Type < UIEVENT__MOUSEEND) {
			if (m_pParent != NULL) m_pParent->DoEvent(event);
			else CVerticalLayoutUI::DoEvent(event);
			return;
		}

		if (event.Type == UIEVENT_TIMER && event.wParam == SCROLL_TIMERID) {
			if (m_dwDelayLeft > 0) {
				--m_dwDelayLeft;
				SIZE sz = GetScrollPos();
				LONG lDeltaY = (LONG)(CalculateDelay((double)m_dwDelayLeft / m_dwDelayNum) * m_dwDelayDeltaY);
				if ((lDeltaY > 0 && sz.cy != 0) || (lDeltaY < 0 && sz.cy != GetScrollRange().cy)) {
					sz.cy -= lDeltaY;
					SetScrollPos(sz);
					return;
				}
			}
			m_dwDelayDeltaY = 0;
			m_dwDelayNum = 0;
			m_dwDelayLeft = 0;
			m_pManager->KillTimer(this, SCROLL_TIMERID);
			return;
		}
		if (event.Type == UIEVENT_SCROLLWHEEL) {
			LONG lDeltaY = 0;
			if (m_dwDelayNum > 0) lDeltaY = (LONG)(CalculateDelay((double)m_dwDelayLeft / m_dwDelayNum) * m_dwDelayDeltaY);
			switch (LOWORD(event.wParam)) {
			case SB_LINEUP:
				if (m_dwDelayDeltaY >= 0) m_dwDelayDeltaY = lDeltaY + 8;
				else m_dwDelayDeltaY = lDeltaY + 12;
				break;
			case SB_LINEDOWN:
				if (m_dwDelayDeltaY <= 0) m_dwDelayDeltaY = lDeltaY - 8;
				else m_dwDelayDeltaY = lDeltaY - 12;
				break;
			}
			if (m_dwDelayDeltaY > 100) m_dwDelayDeltaY = 100;
			else if (m_dwDelayDeltaY < -100) m_dwDelayDeltaY = -100;
			m_dwDelayNum = (DWORD)sqrt((double)abs(m_dwDelayDeltaY)) * 5;
			m_dwDelayLeft = m_dwDelayNum;
			m_pManager->SetTimer(this, SCROLL_TIMERID, 50U);
			return;
		}

		CListUI::DoEvent(event);
	}

	Node* GetRoot() { return _root; }

	Node* AddNode(LPCTSTR text, unsigned long uid,string sid = "", Node* parent = NULL)
	{
		if (!parent) parent = _root;

		CListLabelElementUI* pListElement = new CListLabelElementUI;
		Node* node = new Node;
		node->data()._level = parent->data()._level + 1;
		if (node->data()._level == 0) node->data()._expand = true;
		else node->data()._expand = false;
		node->data()._text = text;
		node->data()._uid = uid;
		node->data()._sid = sid;
		node->data()._pListElement = pListElement;
		
		//���ÿռ�߶�
		//pListElement->SetAttribute(L"height", L"30");
		if (parent != _root) {
			if (!(parent->data()._expand && parent->data()._pListElement->IsVisible()))
				pListElement->SetInternVisible(false);
		}

		CDuiString html_text;
		html_text += _T("<x 6>");
		for (int i = 0; i < node->data()._level; ++i) {
			html_text += _T("<x 24>");
		}



		if (m_listName == _T("talklist") || m_listName == _T("onlineSelectList"))
		{
			if (node->data()._level == 0 ) {
				if (node->data()._expand) html_text += _T("<a><i tree_expand.png 2 1></a>");
				else html_text += _T("<a><i tree_expand.png 2 0></a>");
			}

		}

		//if (node->data()._level < 3) {
		//	if (node->data()._expand) html_text += _T("<a><i tree_expand.png 2 1></a>");
		//	else html_text += _T("<a><i tree_expand.png 2 0></a>");
		//}
		html_text += node->data()._text;
		pListElement->SetText(html_text);
		if (node->data()._level == 0)
			pListElement->SetFixedHeight(24);
		else if (node->data()._level == 1) pListElement->SetFixedHeight(24);
		pListElement->SetTag((UINT_PTR)node);
		if (node->data()._level == 0) {
			//pListElement->SetBkImage(_T("file='tree_top.png' corner='2,1,2,1' fade='100'"));
			pListElement->SetBkImage(m_listBKImage.GetData());
		}

		pListElement->SetName(m_listName.GetData());

		int index = 0;
		if (parent->has_children()) {
			Node* prev = parent->get_last_child();
			index = prev->data()._pListElement->GetIndex() + 1;
		}
		else {
			if (parent == _root) index = 0;
			else index = parent->data()._pListElement->GetIndex() + 1;
		}

		if (!CListUI::AddAt(pListElement, index)) {
			delete pListElement;
			delete node;
			node = NULL;
		}
		parent->add_child(node);
		return node;
	}


	void UpdateNodeItem(LPCTSTR text, Node* node)
	{
		CListLabelElementUI* pListElement = node->data()._pListElement;
		//pListElement->setit

	}


	Node* AddNode(LPCTSTR text, unsigned long uid, int dex, string sid="", Node* parent = NULL)
	{
		if (!parent) parent = _root;

		CListLabelElementUI* pListElement = new CListLabelElementUI;
		Node* node = new Node;
		node->data()._level = parent->data()._level + 1;
		if (node->data()._level == 0) node->data()._expand = true;
		else node->data()._expand = false;
		node->data()._text = text;
		node->data()._uid = uid;
		node->data()._pListElement = pListElement;
		node->data()._sid = sid;
		//���ÿռ�߶�
		//pListElement->SetAttribute(L"height", L"30");
		if (parent != _root) {
			if (!(parent->data()._expand && parent->data()._pListElement->IsVisible()))
				pListElement->SetInternVisible(false);
		}

		CDuiString html_text;
		html_text += _T("<x 6>");
		for (int i = 0; i < node->data()._level; ++i) {
			html_text += _T("<x 24>");
		}

		if (m_listName == _T("talklist") || m_listName == _T("onlineSelectList"))
		{
			if (node->data()._level == 0) {
				if (node->data()._expand) html_text += _T("<a><i tree_expand.png 2 1></a>");
				else html_text += _T("<a><i tree_expand.png 2 0></a>");
			}

		}

		//if (node->data()._level < 3) {
		//	if (node->data()._expand) html_text += _T("<a><i tree_expand.png 2 1></a>");
		//	else html_text += _T("<a><i tree_expand.png 2 0></a>");
		//}
		html_text += node->data()._text;
		pListElement->SetText(html_text);
		if (node->data()._level == 0)
			pListElement->SetFixedHeight(24);
		else if (node->data()._level == 1) pListElement->SetFixedHeight(24);
		pListElement->SetTag((UINT_PTR)node);
		if (node->data()._level == 0) {
			//pListElement->SetBkImage(_T("file='tree_top.png' corner='2,1,2,1' fade='100'"));
			pListElement->SetBkImage(m_listBKImage.GetData());
		}
		pListElement->SetName(m_listName.GetData());
		int index = 0;
		
		/*
		if (parent->has_children()) {
			Node* prev = parent->get_last_child();
			index = prev->data()._pListElement->GetIndex() + 1;
		}
		else {
			if (parent == _root) index = dex;
			else index = parent->data()._pListElement->GetIndex() + 1;
		}
		*/

		index = dex;
		if (!CListUI::AddAt(pListElement, index)) {
			delete pListElement;
			delete node;
			node = NULL;
		}
		parent->add_child(node);

		return node;
	}    
	   
	void SelectNode(Node* node)
	{
		if (!node) node = _root;

		node->data()._pListElement->Select(true);
	}

	int GetNodeIndex( Node* node)
	{
		int index = 0;
		
		
		
		if (!node)
		{
			node = _root;
			Node* prev = node->get_last_child();
			index = prev->data()._pListElement->GetIndex() + 1;

			return index;
		}
		
		/*
		if (node->has_children()) {
			Node* prev = node->get_last_child();
			index = prev->data()._pListElement->GetIndex() + 1;
		}
		else {
			if (node == _root) index = dex;
			else index = node->data()._pListElement->GetIndex() + 1;
		}
		*/
		
		if (node->has_children())
		{
			Node* prev = node->get_last_child();
			index = prev->data()._pListElement->GetIndex() +1;
		}

		return index;
	}





	int GetNodeListNumber(Node* node)
	{
	
		int index = 0;

		if (!node)
		{
			node = _root;
		}
			//Node* prev = node->get_last_child();
			//index = prev->data()._pListElement->GetIndex() + 1;


		for (int i = 0; i < node->num_children(); i++)
		{
			Node *ptr = node->child(i);
			
			index += ptr->num_children();
			GetNodeListNumber(ptr);
		}

		return index;	
	
	}

	bool RemoveNode(Node* node)
	{
		vector<Node*> childVertor;

		if (!node || node == _root) 
			return false;

		int num = node->num_children();
		for (int i = 0; i < num; i++) {
			Node* child = node->child(i);
			childVertor.push_back(child);
		}
		for (int i = 0; i < num; i++)
		{
			RemoveNode(childVertor[i]);
		}
		CListUI::Remove(node->data()._pListElement);
		node->parent()->remove_child(node);
		delete node;
		return true;
	}

	bool RemoveNode2(Node* node)
	{
		if (!node || node == _root) return false;
		for (int i = 0; i < node->num_children(); i++) {
			Node* child = node->child(i);
			RemoveNode(child);
		}
		CListUI::Remove(node->data()._pListElement);
		node->parent()->remove_child(node);
		delete node;
		return true;
	}



	void ExpandNode(Node* node, bool expand)
	{
		if (!node || node == _root) return;

		if (node->data()._expand == expand) return;
		node->data()._expand = expand;

		CDuiString html_text;
		html_text += _T("<x 6>");
		for (int i = 0; i < node->data()._level; ++i) {
			html_text += _T("<x 24>");
		}

		if (m_listName == _T("talklist") || m_listName == _T("onlineSelectList"))
		{
			if (node->data()._level == 0) {
				if (node->data()._expand) html_text += _T("<a><i tree_expand.png 2 1></a>");
				else html_text += _T("<a><i tree_expand.png 2 0></a>");
			}
		}
		
		html_text += node->data()._text;
		node->data()._pListElement->SetText(html_text);

		if (!node->data()._pListElement->IsVisible()) return;
		if (!node->has_children()) return;

		Node* begin = node->child(0);
		Node* end = node->get_last_child();
		for (int i = begin->data()._pListElement->GetIndex(); i <= end->data()._pListElement->GetIndex(); ++i) {
			CControlUI* control = GetItemAt(i);
			if (_tcscmp(control->GetClass(), _T("ListLabelElementUI")) == 0) {
				Node* local_parent = ((UserListUI::Node*)control->GetTag())->parent();
				control->SetInternVisible(local_parent->data()._expand && local_parent->data()._pListElement->IsVisible());
			}
		}
		NeedUpdate();
	}

	SIZE GetExpanderSizeX(Node* node) const
	{
		if (!node || node == _root) return CSize();
		if (node->data()._level >= 3) return CSize();

		SIZE szExpander = { 0 };
		szExpander.cx = 6 + 24 * node->data()._level - 4/*�ʵ��Ŵ�һ��*/;
		szExpander.cy = szExpander.cx + 16 + 8/*�ʵ��Ŵ�һ��*/;
		return szExpander;
	}


	void SetListBKImage(WCHAR *str)
	{
		m_listBKImage = str;
	}

	void SetListName(WCHAR *str)
	{
		m_listName = str;
	}

private:
	Node* _root;
	CDuiString m_listName;
	CDuiString m_listBKImage;
	LONG m_dwDelayDeltaY;
	DWORD m_dwDelayNum;
	DWORD m_dwDelayLeft;
};














#endif
