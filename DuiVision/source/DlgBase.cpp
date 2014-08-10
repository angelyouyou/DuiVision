// DlgBase.cpp
#include "stdafx.h"
#include <winuser.h>
#include <Windows.h>
#include <Windowsx.h>
#include <shlwapi.h>
#include <cmath>
#include "DlgBase.h"

IMPLEMENT_DYNAMIC(CDlgBase, CDialog)

CDlgBase::CDlgBase(UINT nIDTemplate, CWnd* pParent /*=NULL*/)
: CDialog(nIDTemplate, pParent)
{
	m_nMainThreadId = ::GetCurrentThreadId();
	m_nIDTemplate = nIDTemplate;
	m_pParentDuiObject = NULL;
	m_strXmlFile = _T("");
	m_strXmlContent = _T("");
	m_strTitle = _T("");
	m_hIcon = NULL;
	m_bAppWin = FALSE;
	m_MinSize.cx = 0;
	m_MinSize.cy = 0;
	m_bChangeSize = FALSE;
	m_bInit = false;

	m_nFrameTopBottomSpace = 3;
	m_nFrameLeftRightSpace = 3;

	m_nOverRegioX = 100;
	m_nOverRegioY = 100;
	m_sizeBKImage.cx = 100;
	m_sizeBKImage.cy = 100;

	m_bTracking = false;
	m_bIsSetCapture = false;
	m_clrBK = RGB(186, 226, 239);
	m_bDrawImage = FALSE;

	m_bIsLButtonDown = FALSE;
	m_bIsLButtonDblClk = FALSE;
	m_pOldMemBK = NULL;
	m_pControl = NULL;
	m_pFocusControl = NULL;
	m_pWndPopup = NULL;

	m_strFramePicture = _T("");
	m_nFrameSize = 3;
	m_nFrameWLT = 0;
	m_nFrameHLT = 0;
	m_nFrameWRB = 0;
	m_nFrameHRB = 0;

	m_uTimerAnimation = 0;
	m_uTimerAutoClose = 0;

	m_bAutoClose = FALSE;
	m_bAutoHide = FALSE;
	m_uAutoCloseDelayTime = 0;

	m_strBkImg = _T("");
	m_crlBack = RGB(0,0,0);

	m_nTooltipCtrlID = 0;

	m_pTrayHandler = NULL;
	m_strTrayMenuXml = _T("");

	// 初始化图标
	CStringA strTrayIcon = DuiSystem::Instance()->GetSkin("IDB_TRAY_ICON");
	if(!strTrayIcon.IsEmpty())
	{
		if(strTrayIcon.Find(".") != -1)	// 加载图标文件
		{
			CString strIconFile = DuiSystem::GetSkinPath() + CEncodingUtil::AnsiToUnicode(strTrayIcon);
			WORD wIndex = 0;
			m_hIcon = ::ExtractAssociatedIcon(NULL, strIconFile.GetBuffer(0), &wIndex);
		}else	// 加载图标资源
		{
			UINT nResourceID = atoi(strTrayIcon);
			m_hIcon = AfxGetApp()->LoadIcon(nResourceID);
		}
	}
}

CDlgBase::~CDlgBase()
{
	// 结束定时器
	CTimer::KillTimer();

	if (m_hIcon)
	{
		DestroyIcon(m_hIcon);
		m_hIcon = NULL;
	}

	if(m_BKImage.m_hObject != NULL)
	{
		m_BKImage.DeleteObject();
		m_MemBKDC.SelectObject(m_pOldMemBK);
		m_MemBK.DeleteObject();
	}

	if(m_TitleFont.m_hObject != NULL)
	{
		m_TitleFont.DeleteObject();
	}
	m_Rgn.DeleteObject();

	for (size_t i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecControl.at(i);
		if (pControlBase)
		{
			delete pControlBase;
		}		
	}

	for (size_t i = 0; i < m_vecBaseControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseControl.at(i);
		if (pControlBase)
		{
			delete pControlBase;
		}		
	}

	for (size_t i = 0; i < m_vecArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecArea.at(i);
		if (pControlBase)
		{
			delete pControlBase;
		}		
	}

	for (size_t i = 0; i < m_vecBaseArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseArea.at(i);
		if (pControlBase)
		{
			delete pControlBase;
		}		
	}
}

BEGIN_MESSAGE_MAP(CDlgBase, CDialog)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_WM_NCPAINT()
	ON_WM_ERASEBKGND()
	ON_WM_NCHITTEST()
	ON_WM_CREATE()
	ON_WM_SIZE()
	ON_WM_NCCALCSIZE()
	ON_WM_GETMINMAXINFO()
	ON_WM_WINDOWPOSCHANGING()
	ON_WM_NCDESTROY()
	ON_WM_CLOSE()
	ON_WM_NCACTIVATE()
	ON_WM_MOUSEMOVE()
	ON_WM_MOUSEWHEEL()
	ON_MESSAGE(WM_MOUSELEAVE, OnMouseLeave)
	ON_MESSAGE(WM_MOUSEHOVER, OnMouseHover)
	ON_WM_KEYDOWN()
	ON_WM_SYSCOMMAND()
	ON_WM_LBUTTONDOWN()
	ON_WM_LBUTTONUP()
	ON_WM_LBUTTONDBLCLK()
	ON_WM_DROPFILES()
	ON_WM_DESTROY()
	ON_MESSAGE(WM_USER_CLOSEWND, OnUserCloseWindow)
	ON_MESSAGE(WM_SKIN, OnMessageSkin)
	ON_MESSAGE(WM_UI_TASK, OnMessageUITask)
	ON_MESSAGE(WM_SYSTEM_TRAYICON, OnSystemTrayIcon)
	ON_REGISTERED_MESSAGE(WM_CHECK_ITS_ME, OnCheckItsMe)
END_MESSAGE_MAP()

// 设置最小窗口大小
void CDlgBase::SetMinSize(int iWidth, int iHeight)
{
	m_MinSize.cx = iWidth;
	m_MinSize.cy = iHeight;
}

CSize CDlgBase::GetMinSize()
{
	return	m_MinSize;
}

void CDlgBase::OnGetMinMaxInfo(MINMAXINFO* lpMMI)
{
	CDialog::OnGetMinMaxInfo(lpMMI);
	lpMMI->ptMinTrackSize.x = m_MinSize.cx;
	lpMMI->ptMinTrackSize.y = m_MinSize.cy;

	CRect	rc(0, 0, 1024, 768);
	SystemParametersInfo(SPI_GETWORKAREA, 0, &rc, 0);
	lpMMI->ptMaxPosition.x = rc.left;
	lpMMI->ptMaxPosition.y = rc.top;
	lpMMI->ptMaxSize.x = rc.Width();
	lpMMI->ptMaxSize.y = rc.Height();
}

// 根据控件名创建控件实例
CControlBase* CDlgBase::_CreateControlByName(LPCSTR lpszName)
{
	return DuiSystem::CreateControlByName(lpszName, GetSafeHwnd(), this);
}

// 判断当前是否在主线程
void CDlgBase::TestMainThread()
{
    // 当你看到这个东西的时候，我不幸的告诉你，你的其他线程在刷界面
    // 这是一件很危险的事情
    DWORD dwCurThreadID = GetCurrentThreadId();
    BOOL bOK = (m_nMainThreadId == dwCurThreadID); // 当前线程和构造对象时的线程一致
	if(!bOK)
	{
		DuiSystem::LogEvent(LOG_LEVEL_ERROR, _T("TestMainThread failed"));
	}
    ASSERT(bOK);
}

// CDlgBase message handlers

BOOL CDlgBase::OnInitDialog()
{
	CDialog::OnInitDialog();

	SetIcon(m_hIcon, TRUE);			// 设置大图标
	SetIcon(m_hIcon, FALSE);		// 设置小图标

	// 加载窗口的XML文件
	TiXmlDocument xmlDoc;
	TiXmlElement* pDlgElem = NULL;
	//xmlDoc.Parse(strXml, NULL, TIXML_ENCODING_UTF8);

	if(!m_strXmlFile.IsEmpty())	// 加载XML文件
	{
		if(m_strXmlFile.Find(_T("xml:")) == 0)
		{
			m_strXmlFile.Delete(0, 4);
			m_strXmlFile = DuiSystem::GetXmlPath() + m_strXmlFile;
		}
		//CStringA strFile = CT2A(m_strXmlFile, CP_UTF8);
		xmlDoc.LoadFile(CEncodingUtil::UnicodeToAnsi(m_strXmlFile), TIXML_ENCODING_UTF8);
	}else
	if(!m_strXmlContent.IsEmpty())	// 加载XML内容
	{
		xmlDoc.Parse(CT2A(m_strXmlContent, CP_UTF8), NULL, TIXML_ENCODING_UTF8);
	}

	if(!xmlDoc.Error())
	{
		pDlgElem = xmlDoc.FirstChildElement("dlg");//RootElement();
		if(pDlgElem != NULL)
		{
			// 加载dlg节点属性
			Load(pDlgElem);
			// 更新窗口大小
			SetMinSize(m_MinSize.cx, m_MinSize.cy);
			SetRect(CRect(0, 0, m_MinSize.cx, m_MinSize.cy));
		}
	}

	// 初始化对话框自身的预设值属性
	InitDialogValue();

	SetWindowText(m_strTitle);

	CFont	tmpFont;
	tmpFont.Attach(GetStockObject(DEFAULT_GUI_FONT));

	LOGFONT font;
	memset(&font, 0, sizeof(font));
	tmpFont.GetLogFont(&font);

	CWindowDC dc(this);
	wcscpy(font.lfFaceName,DuiSystem::GetDefaultFont());
	font.lfHeight = -10 * GetDeviceCaps(dc.m_hDC, LOGPIXELSY) / 72;
	font.lfWeight = 600;

	m_TitleFont.CreateFontIndirect(&font);

	::SetWindowPos(m_hWnd, NULL, 0, 0, m_MinSize.cx, m_MinSize.cy, SWP_HIDEWINDOW | SWP_NOMOVE);
	
	CRect	rc;
	GetClientRect(rc);

	// 使用XML节点初始化窗口基础控件和普通控件
	if(pDlgElem != NULL)
	{
		TiXmlElement* pBaseElem = pDlgElem->FirstChildElement("base");
		if(pBaseElem != NULL)
		{
			InitBaseUI(rc, pBaseElem);
		}

		TiXmlElement* pBodyElem = pDlgElem->FirstChildElement("body");
		if(pBodyElem != NULL)
		{
			InitUI(rc, pBodyElem);
		}

		// 初始化预设值的控件值
		InitControlValue();
	}

	if(!m_bAppWin)
	{
		::SetWindowLong(m_hWnd, GWL_EXSTYLE, WS_EX_TOOLWINDOW);
	}

	//加载背景图
	if(!m_strBkImg.IsEmpty())	// 如果窗口设置了背景图片属性，就用此背景图片
	{
		// 通过Skin读取
		CString strBkSkin = _T("");
		if(m_strBkImg.Find(_T("skin:")) == 0)
		{
			strBkSkin = CEncodingUtil::AnsiToUnicode(DuiSystem::Instance()->GetSkin(CEncodingUtil::UnicodeToAnsi(m_strBkImg)));
		}else
		{
			strBkSkin = m_strBkImg;
		}

		if(strBkSkin.Find(_T(".")) != -1)	// 加载图片文件
		{
			CString strImgFile = DuiSystem::GetSkinPath() + strBkSkin;
			LoadImage(strImgFile);
		}else	// 加载图片资源
		{
			UINT nResourceID = _wtoi(strBkSkin);
			LoadImage(nResourceID, TEXT("PNG"));
		}
	}else
	if(m_crlBack != RGB(0,0,0))	// 如果窗口设置了背景颜色属性，就用此背景颜色
	{
		DrawBackground(m_crlBack);
	}else
	{
		// 调用DuiSystem获取背景信息并更新
		InitWindowBkSkin();
	}

	CenterWindow();
	ShowWindow(SW_SHOW);

	//启动定时器
	m_uTimerAnimation = CTimer::SetTimer(30);

	// 初始化Tooltip
	m_wndToolTip.Create(this);

	m_bInit = true;

	// 调用事件处理对象的初始化函数
	if(m_pDuiHandler)
	{
		m_pDuiHandler->OnInit();
	}

	return TRUE;
}

// 加载窗口基础控件
void CDlgBase::InitBaseUI(CRect rcClient, TiXmlElement* pNode)
{
	if(pNode)
	{
		TiXmlElement* pControlElem = NULL;
		for (pControlElem = pNode->FirstChildElement(); pControlElem != NULL; pControlElem=pControlElem->NextSiblingElement())
		{
			if(pControlElem != NULL)
			{
				CStringA strControlName = pControlElem->Value();
				CControlBase* pControl = _CreateControlByName(strControlName);
				if(pControl)
				{
					pControl->Load(pControlElem);
					if(pControl->IsClass(CArea::GetClassName()) || pControl->IsClass(CFrame::GetClassName()))
					{
						// Area和Frame不能响应鼠标,必须加到Area列表中
						m_vecBaseArea.push_back(pControl);
					}else
					{
						m_vecBaseControl.push_back(pControl);
					}
				}
			}
		}
	}
}

// 初始化窗口控件
void CDlgBase::InitUI(CRect rcClient, TiXmlElement* pNode)
{
	CRect rcTemp;
	int nStartX = 0;
	int nStartY = 0;
	CControlBase * pControlBase = NULL;

	// 主窗口的透明度渐变层蒙板(根据窗口的frame属性来设置)
	if(!m_strFramePicture.IsEmpty())
	{
		pControlBase = new CDuiPicture(GetSafeHwnd(), this, FRAME_MAINWND, rcClient);
		((CDuiPicture*)pControlBase)->OnAttributeImage(CEncodingUtil::UnicodeToAnsi(m_strFramePicture), FALSE);
		if(m_nFrameWLT != 0)
		{
			// 九宫格模式
			((CDuiPicture*)pControlBase)->SetShowModeMID(m_nFrameWLT, m_nFrameHLT, m_nFrameWRB, m_nFrameHRB);
		}else
		{
			// 边框模式
			((CDuiPicture*)pControlBase)->SetShowMode(enSMFrame, m_nFrameSize);
		}
		pControlBase->OnAttributePosChange("0,0,-0,-0", FALSE);
		m_vecBaseArea.push_back(pControlBase);
	}

	// 加载所有窗口控件
	if(pNode)
	{
		TiXmlElement* pControlElem = NULL;
		for (pControlElem = pNode->FirstChildElement(); pControlElem != NULL; pControlElem=pControlElem->NextSiblingElement())
		{
			if(pControlElem != NULL)
			{
				CStringA strControlName = pControlElem->Value();
				CControlBase* pControl = _CreateControlByName(strControlName);
				if(pControl)
				{
					pControl->Load(pControlElem);
					if(pControl->IsClass(CArea::GetClassName()) || pControl->IsClass(CFrame::GetClassName()))
					{
						// Area和Frame不能响应鼠标,必须加到Area列表中
						m_vecArea.push_back(pControl);
					}else
					{
						m_vecControl.push_back(pControl);
						// 对话框初始化时候显示一下所有控件，可以使edit等Windows原生控件可以自动创建出来
						//BOOL bIsVisible = pControl->GetVisible();
						//pControl->SetControlWndVisible(bIsVisible);
					}
				}
			}
		}
	}
}

// 添加控件预设置信息
void CDlgBase::SetControlValue(CString strName, CString strType, CString strValue)
{
	CONTROL_VALUE ctrlValue;
	ctrlValue.strName = strName;
	ctrlValue.strType = strType;
	ctrlValue.strValue = strValue;
	m_vecControlValue.push_back(ctrlValue);
}

// 初始化预设置对话框属性
void CDlgBase::InitDialogValue()
{
	for (size_t i = 0; i < m_vecControlValue.size(); i++)
	{
		CONTROL_VALUE* pCtrlValue = &(m_vecControlValue.at(i));

		// 对话框自身属性更改
		if(pCtrlValue->strName.IsEmpty())
		{
			if(pCtrlValue->strType == _T("width"))
			{
				m_MinSize.cx = _wtoi(pCtrlValue->strValue);
				// 更新窗口大小
				SetMinSize(m_MinSize.cx, m_MinSize.cy);
				SetRect(CRect(0, 0, m_MinSize.cx, m_MinSize.cy));
			}else
			if(pCtrlValue->strType == _T("height"))
			{
				m_MinSize.cy = _wtoi(pCtrlValue->strValue);
				// 更新窗口大小
				SetMinSize(m_MinSize.cx, m_MinSize.cy);
				SetRect(CRect(0, 0, m_MinSize.cx, m_MinSize.cy));
			}
		}
	}
}

// 初始化预设置控件值
void CDlgBase::InitControlValue()
{
	for (size_t i = 0; i < m_vecControlValue.size(); i++)
	{
		CONTROL_VALUE* pCtrlValue = &(m_vecControlValue.at(i));

		// 对话框自身属性更改
		if(pCtrlValue->strName.IsEmpty())
		{
			continue;
		}

		CControlBase* pControl = GetControl(pCtrlValue->strName);
		if(pControl == NULL)
		{
			// 如果在普通控件中未找到,则找一下基础控件
			pControl = GetBaseControl(pCtrlValue->strName);
		}
		if(pControl != NULL)
		{
			if(pCtrlValue->strType == _T("visible"))
			{
				pControl->SetVisible(_wtoi(pCtrlValue->strValue));
			}else
			if(pCtrlValue->strType == _T("disable"))
			{
				pControl->SetDisable(_wtoi(pCtrlValue->strValue));
			}else
			if(pCtrlValue->strType == _T("title"))
			{
				((CControlBaseFont*)pControl)->SetTitle(pCtrlValue->strValue);
			}else
			if(pCtrlValue->strType == _T("image"))
			{
				((CControlBaseFont*)pControl)->SetImage(pCtrlValue->strValue);
			}else
			if(pCtrlValue->strType == _T("check"))
			{
				((CCheckButton*)pControl)->SetCheck(pCtrlValue->strValue == _T("true"));
			}
		}
	}
}

// 设置窗口自动关闭的定时器
void CDlgBase::SetAutoCloseTimer(UINT uDelayTime, BOOL bHideWnd)
{
	// 先删除定时器,再创建新的定时器
	if(m_uTimerAutoClose != 0)
	{
		CTimer::KillTimer(m_uTimerAutoClose);
		m_uTimerAutoClose = 0;
	}
	m_uAutoCloseDelayTime = uDelayTime;
	m_bAutoClose = FALSE;
	m_bAutoHide = FALSE;
	if(uDelayTime != 0)
	{
		// 如果延迟时间设置的是非0,才启动自动关闭
		if(bHideWnd)
		{
			m_bAutoHide = TRUE;	// 隐藏窗口模式
		}else
		{
			m_bAutoClose = TRUE;	// 关闭窗口模式
		}
		if(!m_bTracking)
		{
			// 如果鼠标不在窗口范围内才创建定时器
			m_uTimerAutoClose = CTimer::SetTimer(uDelayTime);
		}
	}
}

void CDlgBase::OnSize(CRect rcClient)
{
}

// 获取子控件对象
CControlBase *CDlgBase::GetControl(UINT uControlID)
{
	for (size_t i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecControl.at(i);
		if (pControlBase)
		{
			if (pControlBase->GetControlID() == uControlID)
			{
				return pControlBase;
			}else
			{
				// 查找子控件
				CControlBase * pSubControl = pControlBase->GetControl(uControlID);
				if(pSubControl != NULL)
				{
					return pSubControl;
				}
			}
		}
	}

	for (size_t i = 0; i < m_vecArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecArea.at(i);
		if (pControlBase)
		{
			if (pControlBase->GetControlID() == uControlID)
			{
				return pControlBase;
			}else
			{
				// 查找子控件
				CControlBase * pSubControl = pControlBase->GetControl(uControlID);
				if(pSubControl != NULL)
				{
					return pSubControl;
				}
			}
		}
	}

	return NULL;
}

// 获取子控件对象
CControlBase *CDlgBase::GetControl(CString strControlName)
{
	for (size_t i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecControl.at(i);
		if (pControlBase)
		{
			if (pControlBase->GetName() == strControlName)
			{
				return pControlBase;
			}else
			{
				// 查找子控件
				CControlBase * pSubControl = pControlBase->GetControl(strControlName);
				if(pSubControl != NULL)
				{
					return pSubControl;
				}
			}
		}
	}

	for (size_t i = 0; i < m_vecArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecArea.at(i);
		if (pControlBase)
		{
			if (pControlBase->GetName() == strControlName)
			{
				return pControlBase;
			}else
			{
				// 查找子控件
				CControlBase * pSubControl = pControlBase->GetControl(strControlName);
				if(pSubControl != NULL)
				{
					return pSubControl;
				}
			}
		}
	}

	return NULL;
}

// 获取子控件对象
CControlBase *CDlgBase::GetBaseControl(UINT uControlID)
{
	for (size_t i = 0; i < m_vecBaseControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseControl.at(i);
		if (pControlBase)
		{
			if (pControlBase->GetControlID() == uControlID)
			{
				return pControlBase;
			}
		}
	}

	for (size_t i = 0; i < m_vecBaseArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseArea.at(i);
		if (pControlBase)
		{
			if (pControlBase->GetControlID() == uControlID)
			{
				return pControlBase;
			}
		}
	}

	return NULL;
}

// 获取子控件对象
CControlBase *CDlgBase::GetBaseControl(CString strControlName)
{
	for (size_t i = 0; i < m_vecBaseControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseControl.at(i);
		if (pControlBase)
		{
			if (pControlBase->GetName() == strControlName)
			{
				return pControlBase;
			}
		}
	}

	for (size_t i = 0; i < m_vecBaseArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseArea.at(i);
		if (pControlBase)
		{
			if (pControlBase->GetName() == strControlName)
			{
				return pControlBase;
			}
		}
	}

	return NULL;
}

// 设置当前焦点控件
void CDlgBase::SetFocusControl(CControlBase* pFocusControl)
{
	if(pFocusControl != m_pFocusControl)
	{
		if(m_pFocusControl != NULL)
		{
			m_pFocusControl->OnFocus(FALSE);
		}
		if(pFocusControl != NULL)
		{
			pFocusControl->OnFocus(TRUE);
		}
		m_pFocusControl = pFocusControl;
	}
}

// 获取当前焦点控件
CControlBase* CDlgBase::GetFocusControl()
{
	for (int i = m_vecControl.size()-1; i >= 0; i--)
	{
		CControlBase* pControlBase = m_vecControl.at(i);
		if (pControlBase && pControlBase->GetVisible() && !pControlBase->GetDisable() && (pControlBase == m_pFocusControl) && pControlBase->IsTabStop())
		{
			return pControlBase;
		}else
		if (pControlBase && (pControlBase == m_pControl))
		{
			// 查找子控件
			pControlBase = pControlBase->GetFocusControl(m_pFocusControl);
			if(pControlBase != NULL)
			{
				return pControlBase;
			}
		}
	}

	return NULL;
}

// 获取上一个可以获取焦点的子控件
CControlBase* CDlgBase::GetPrevFocusableControl()
{
	BOOL bStartSearch = FALSE;
	// 先按照焦点控件查找一次
	for (int i = m_vecControl.size()-1; i >= 0; i--)
	{
		CControlBase* pControlBase = m_vecControl.at(i);
		if (pControlBase && pControlBase->GetVisible() && !pControlBase->GetDisable() && bStartSearch && pControlBase->IsTabStop())
		{
			return pControlBase;			
		}else
		if (pControlBase && (pControlBase == m_pFocusControl))
		{
			bStartSearch = TRUE;
		}
	}

	// 再遍历子控件查找
	if(m_pFocusControl == NULL)
	{
		bStartSearch = TRUE;
	}
	for (int i = m_vecControl.size()-1; i >= 0; i--)
	{
		CControlBase* pControlBase = m_vecControl.at(i);
		if(m_pControl == NULL)
		{
			m_pControl = pControlBase;
		}
		if (pControlBase && pControlBase->GetVisible() && !pControlBase->GetDisable() && bStartSearch && pControlBase->IsTabStop())
		{
			return pControlBase;			
		}else
		if (pControlBase && (pControlBase == m_pControl))
		{
			// 查找子控件
			pControlBase = pControlBase->GetPrevFocusableControl(m_pFocusControl);
			if(pControlBase != NULL)
			{
				return pControlBase;
			}
			//bStartSearch = TRUE;
		}
	}

	return NULL;
}

// 获取下一个可以获取焦点的子控件
CControlBase* CDlgBase::GetNextFocusableControl()
{
	BOOL bStartSearch = FALSE;
	// 先按照焦点控件查找一次
	for (int i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase* pControlBase = m_vecControl.at(i);
		if (pControlBase && pControlBase->GetVisible() && !pControlBase->GetDisable() && bStartSearch && pControlBase->IsTabStop())
		{
			return pControlBase;			
		}else
		if (pControlBase && (pControlBase == m_pFocusControl))
		{
			bStartSearch = TRUE;
		}
	}

	// 再遍历子控件查找
	if(m_pFocusControl == NULL)
	{
		bStartSearch = TRUE;
	}
	for (int i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase* pControlBase = m_vecControl.at(i);
		if(m_pControl == NULL)
		{
			m_pControl = pControlBase;
		}
		if (pControlBase && pControlBase->GetVisible() && !pControlBase->GetDisable() && bStartSearch && pControlBase->IsTabStop())
		{
			return pControlBase;			
		}else
		if (pControlBase && (pControlBase == m_pControl))
		{
			// 查找子控件
			pControlBase = pControlBase->GetNextFocusableControl(m_pFocusControl);
			if(pControlBase != NULL)
			{
				return pControlBase;
			}
			//bStartSearch = TRUE;
		}
	}

	return NULL;
}

int CDlgBase::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	// 设置窗口风格
	DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE)
		| WS_MAXIMIZEBOX | WS_MINIMIZEBOX
		| WS_SYSMENU | WS_SIZEBOX | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CS_DBLCLKS;
	dwStyle &= ~(WS_CAPTION);

	// 改变窗口大小
	if(!m_bChangeSize)
	{
		dwStyle &= ~(WS_THICKFRAME);
	}

	::SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);

	return CDialog::OnCreate(lpCreateStruct);
}

// 从XML设置resize属性
HRESULT CDlgBase::OnAttributeResize(const CStringA& strValue, BOOL bLoading)
{
    if (strValue.IsEmpty()) return E_FAIL;

	// 获取resize属性，并重新设置一下窗口风格
	m_bChangeSize = atoi(strValue);

	// 设置窗口风格
	DWORD dwStyle = ::GetWindowLong(m_hWnd, GWL_STYLE)
		| WS_MAXIMIZEBOX | WS_MINIMIZEBOX
		| WS_SYSMENU | WS_SIZEBOX | WS_CLIPSIBLINGS | WS_CLIPCHILDREN | CS_DBLCLKS;
	dwStyle &= ~(WS_CAPTION);

	// 改变窗口大小
	if(!m_bChangeSize)
	{
		dwStyle &= ~(WS_THICKFRAME);
	}

	::SetWindowLong(m_hWnd, GWL_STYLE, dwStyle);
	
	return bLoading?S_FALSE:S_OK;
}

// 初始化窗口背景皮肤
void CDlgBase::InitWindowBkSkin()
{
	// 如果窗口设置了特殊的背景,则不用全局的背景
	if(!m_strBkImg.IsEmpty() || (m_crlBack != RGB(0,0,0)))
	{
		return;
	}

	// 调用DuiSystem获取背景信息
	int nType = 0;
	int nIDResource = 0;
	COLORREF clr = RGB(0,0,0);
	CString strImgFile = _T("");
	BOOL bRet = DuiSystem::Instance()->GetWindowBkInfo(nType, nIDResource, clr, strImgFile);
	if(bRet && !((nType == BKTYPE_IMAGE_RESOURCE) && (nIDResource == 0)))
	{
		if(nType == BKTYPE_IMAGE_RESOURCE)	// 图片资源
		{
			LoadImage(nIDResource);
		}else
		if(nType == BKTYPE_COLOR)	// 颜色
		{
			DrawBackground(clr);
		}else
		if(nType == BKTYPE_IMAGE_FILE)	// 图片文件
		{
			LoadImage(strImgFile);
		}
	}else
	{
		// 默认加载第一张背景图片
		CString strImgFile = CEncodingUtil::AnsiToUnicode(DuiSystem::Instance()->GetSkin("SKIN_PIC_0"));
		strImgFile = DuiSystem::GetSkinPath() + strImgFile;
		LoadImage(strImgFile);
	}
}

// 拖拽图片更新窗口背景图片
void CDlgBase::OnDropFiles(HDROP hDropInfo)
{
	TCHAR szFileName[MAX_PATH + 1] = {0};
	UINT nFiles = DragQueryFile(hDropInfo, 0xFFFFFFFF, NULL, 0);
	for(UINT i = 0; i < nFiles; i++)
	{		
		DragQueryFile(hDropInfo, i, szFileName, MAX_PATH);
		if(PathIsDirectory(szFileName))
		{
			continue;
		}	
		CString strFileName = szFileName;
		strFileName = strFileName.Right(3);
		if (0 == strFileName.CompareNoCase(TEXT("bmp")) || 0 == strFileName.CompareNoCase(TEXT("jpg")) || 0 == strFileName.CompareNoCase(TEXT("png")))
		{
			LoadImage(szFileName);
			// 保存背景信息
			DuiSystem::Instance()->SetWindowBkInfo(BKTYPE_IMAGE_FILE, 0, RGB(0,0,0), szFileName);
			// 刷新所有窗口的背景皮肤
			DuiSystem::Instance()->ResetAllWindowsBkSkin();
			break;
		}
	}
	// CDialog::OnDropFiles(hDropInfo);
}

void CDlgBase::LoadImage(UINT nIDResource, CString strType)
{
	CBitmap bitBackground;
	::LoadImage(nIDResource, bitBackground, m_sizeBKImage, strType);	
	DrawBackground(bitBackground);	
}

void CDlgBase::LoadImage(CString strFileName)
{
	CBitmap bitBackground;
	::LoadImage(strFileName, bitBackground, m_sizeBKImage);	
	DrawBackground(bitBackground);	
}

// 画背景图片
void CDlgBase::DrawBackground(CBitmap &bitBackground)
{
	if(m_MinSize.cx - 2 > m_sizeBKImage.cx || m_MinSize.cy - 2 > m_sizeBKImage.cy || m_bChangeSize)
	{
		if(m_MinSize.cx - 2 > m_sizeBKImage.cx)
		{
			m_nOverRegioX = __min(100, m_sizeBKImage.cx - 2);
		}
		else
		{
			m_nOverRegioX = 0;
		}

		if(m_MinSize.cy - 2 > m_sizeBKImage.cy)
		{
			m_nOverRegioY = __min(100, m_sizeBKImage.cy - 2);
		}
		else
		{
			m_nOverRegioY = 0;
		}
	}
 	else
 	{
 		m_nOverRegioX = 0;
		m_nOverRegioY = 0;
 	}
	if(bitBackground.m_hObject)
	{
		m_bDrawImage = TRUE;
		int nWidth = m_sizeBKImage.cx;
		int nHeight = m_sizeBKImage.cy;

		if(m_MemBKDC.m_hDC)
		{
			m_MemBKDC.DeleteDC();
		}

		CDC *pDC = GetDC();
		
		::GetAverageColor(pDC, bitBackground, m_sizeBKImage, m_clrBK);

		m_MemBKDC.CreateCompatibleDC(pDC);

		if(m_MemBK.m_hObject)
		{
			m_MemBK.DeleteObject();
		}
		m_MemBK.CreateCompatibleBitmap(pDC, m_sizeBKImage.cx, m_sizeBKImage.cy);
		m_pOldMemBK =  m_MemBKDC.SelectObject(&m_MemBK);

		CDC TempDC;
		TempDC.CreateCompatibleDC(pDC);

		CBitmap* pOldBitmap = TempDC.SelectObject(&bitBackground);

		// 画出平均图片
		m_MemBKDC.FillSolidRect(0, 0, nWidth, nHeight, m_clrBK); 		

		if(m_nOverRegioX > 0 && m_nOverRegioY > 0)
		{
			int nOverRegio = __min(m_nOverRegioX, m_nOverRegioY);

			// 左上
			m_MemBKDC.BitBlt(0, 0, nWidth - nOverRegio, nHeight - nOverRegio, &TempDC, 0, 0, SRCCOPY ); 

			// 中间
			CRect rc(0, 0, nWidth, nHeight);
			DrawRightBottomTransition(m_MemBKDC, TempDC, rc, nOverRegio, m_clrBK);

			// 右上
			rc.SetRect(nWidth - nOverRegio, 0, nWidth, nHeight - nOverRegio);
			DrawHorizontalTransition(m_MemBKDC, TempDC, rc, rc, 0, 100);

			// 左下
			rc.SetRect(0, nHeight - nOverRegio, nWidth - nOverRegio, nHeight);
			DrawVerticalTransition(m_MemBKDC, TempDC, rc, rc, 0, 100);
		}
		else if(m_nOverRegioX > 0 && m_nOverRegioY == 0)
		{
			// 左边
			m_MemBKDC.BitBlt(0, 0, nWidth - m_nOverRegioX, nHeight, &TempDC, 0, 0, SRCCOPY ); 

			// 右边
			CRect rc(nWidth - m_nOverRegioX, 0, nWidth, nHeight);
			DrawHorizontalTransition(m_MemBKDC, TempDC, rc, rc, 0, 100);
		}
		else if(m_nOverRegioX == 0 && m_nOverRegioY > 0)
		{
			// 边上
			m_MemBKDC.BitBlt(0, 0, nWidth, nHeight - m_nOverRegioY, &TempDC, 0, 0, SRCCOPY ); 

			// 下边
			CRect rc(0, nHeight - m_nOverRegioY, nWidth, nHeight);
			DrawVerticalTransition(m_MemBKDC, TempDC, rc, rc, 0, 100);
		}
		else
		{
			m_MemBKDC.BitBlt(0, 0, nWidth, nHeight, &TempDC, 0, 0, SRCCOPY ); 
		}

		TempDC.SelectObject(pOldBitmap);
		TempDC.DeleteDC();
		ReleaseDC(pDC);

		ResetControl();		
	}
}

// 画背景图片
void CDlgBase::DrawBackground(COLORREF clr)
{
	m_clrBK = clr;
	m_bDrawImage = FALSE;
	ResetControl();		
}

BOOL CDlgBase::OnEraseBkgnd(CDC* pDC) 
{
	return TRUE;
}

HCURSOR CDlgBase::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}

void CDlgBase::OnPaint()
{
	if (IsIconic())
	{
		// 最小化状态不用绘制图标
		/*CPaintDC dc(this); // 用于绘制的设备上下文

 		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// 使图标在工作区矩形中居中
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// 绘制图标
		dc.DrawIcon(x, y, m_hIcon);*/
	}
	else
	{
 		CRect rcUpdate;
		GetUpdateRect(&rcUpdate);

		CRect	rcClient;
 		GetClientRect(&rcClient);

		CPaintDC	dc(this);
		CDC MemDC;
		MemDC.CreateCompatibleDC(&dc);
		CBitmap memBmp;
		memBmp.CreateCompatibleBitmap(&dc, rcClient.Width(), rcClient.Height());
		CBitmap *pOldmap =  MemDC.SelectObject(&memBmp);

		DrawImageStyle(MemDC, rcClient, rcUpdate);
		dc.BitBlt(rcUpdate.left, rcUpdate.top, rcUpdate.Width(), rcUpdate.Height(), &MemDC, rcUpdate.left, rcUpdate.top, SRCCOPY);

		MemDC.SelectObject(pOldmap);
		MemDC.DeleteDC();
		memBmp.DeleteObject();
	}
}

// 画背景和控件
void CDlgBase::DrawImageStyle(CDC &dc, const CRect &rcClient, const CRect &rcUpdate)
{
	dc.FillSolidRect(rcUpdate.left, rcUpdate.top, rcUpdate.Width(), rcUpdate.Height(), m_clrBK);

	if(m_bDrawImage)
	{
		// 背景
		CRect rcBk(1, 1, 1 + m_sizeBKImage.cx, 1 + m_sizeBKImage.cy);
		rcBk = rcBk & rcUpdate;
		if(!rcBk.IsRectEmpty())
		{
			dc.BitBlt(rcBk.left, rcBk.top, rcBk.Width() , rcBk.Height(), &m_MemBKDC, rcBk.left, rcBk.top, SRCCOPY ); 
		}	
	}

	// 控件
	DrawControl(dc, rcUpdate);
}

// 重置控件
void CDlgBase::ResetControl()
{
	for (size_t i = 0; i < m_vecArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecArea.at(i);
		if (pControlBase)
		{
			pControlBase->SetUpdate(FALSE, m_clrBK);
		}
	}

	for (size_t i = 0; i < m_vecBaseArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseArea.at(i);
		if (pControlBase)
		{
			pControlBase->SetUpdate(FALSE, m_clrBK);
		}
	}

	for (size_t i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecControl.at(i);
		if (pControlBase)
		{
			pControlBase->SetUpdate(FALSE, m_clrBK);			
		}
	}

	for (size_t i = 0; i < m_vecBaseControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseControl.at(i);
		if (pControlBase)
		{
			pControlBase->SetUpdate(FALSE, m_clrBK);			
		}
	}

	InvalidateRect(NULL);
}

// 移动控件
CControlBase * CDlgBase::SetControlRect(UINT uControlID, CRect rc)
{
	CControlBase *pControlBase = GetControl(uControlID);
	if(pControlBase)
	{
		pControlBase->SetRect(rc);
		UpdateHover();
	}
	return pControlBase;
}

// 移动控件
CControlBase * CDlgBase::SetControlRect(CControlBase *pControlBase, CRect rc)
{
	if(pControlBase)
	{
		pControlBase->SetRect(rc);
		UpdateHover();
	}
	return pControlBase;
}

// 显示控件
CControlBase * CDlgBase::SetControlVisible(UINT uControlID, BOOL bVisible)
{
	CControlBase *pControlBase = GetControl(uControlID);
	if(pControlBase)
	{
		pControlBase->SetVisible(bVisible);
		UpdateHover();
	}
	return pControlBase;
}

// 显示控件
CControlBase * CDlgBase::SetControlVisible(CControlBase *pControlBase, BOOL bVisible)
{
	if(pControlBase)
	{
		pControlBase->SetVisible(bVisible);
		UpdateHover();
	}
	return pControlBase;
}

// 禁用控件
CControlBase * CDlgBase::SetControlDisable(UINT uControlID, BOOL bDisable)
{
	CControlBase *pControlBase = GetControl(uControlID);
	if(pControlBase)
	{
		pControlBase->SetDisable(bDisable);
		UpdateHover();
	}
	return pControlBase;
}

// 禁用控件
CControlBase * CDlgBase::SetControlDisable(CControlBase *pControlBase, BOOL bDisable)
{
	if(pControlBase)
	{
		pControlBase->SetDisable(bDisable);
		UpdateHover();
	}
	return pControlBase;
}

// 更新选中
void CDlgBase::UpdateHover()
{
	CPoint point;
	GetCursorPos(&point);
	OnMouseMove(0, point);
}

void CDlgBase::DrawControl(CDC &dc, const CRect &rcUpdate)
{
	for (size_t i = 0; i < m_vecArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecArea.at(i);
		if (pControlBase)
		{
			pControlBase->Draw(dc, rcUpdate);
		}
	}

	for (size_t i = 0; i < m_vecBaseArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseArea.at(i);
		if (pControlBase)
		{
			pControlBase->Draw(dc, rcUpdate);
		}
	}

	for (size_t i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecControl.at(i);
		if (pControlBase)
		{
			pControlBase->Draw(dc, rcUpdate);			
		}
	}

	for (size_t i = 0; i < m_vecBaseControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseControl.at(i);
		if (pControlBase)
		{
			pControlBase->Draw(dc, rcUpdate);			
		}
	}	
}

void CDlgBase::OnNcPaint()
{
}

LRESULT CDlgBase::OnNcHitTest(CPoint point)
{
	// 不能改变窗口大小
	if(!m_bChangeSize)
	{
		return	HTCLIENT;
	}
	CRect	rc;
	GetWindowRect(rc);
	rc.OffsetRect(-rc.left, -rc.top);
	ScreenToClient(&point);
	int x = point.x;
	int y = point.y;

	if ( x < m_nFrameLeftRightSpace && y < m_nFrameTopBottomSpace)
	{
		return	HTTOPLEFT;
	}
	if ( x <= m_nFrameLeftRightSpace && y >= rc.bottom - m_nFrameTopBottomSpace)
	{
		return	HTBOTTOMLEFT;
	}
	if ( x > rc.right - m_nFrameLeftRightSpace && y < m_nFrameTopBottomSpace)
	{
		return	HTTOPRIGHT;
	}
	if ( x >= rc.right - m_nFrameLeftRightSpace && y >= rc.bottom - m_nFrameTopBottomSpace)
	{
		return	HTBOTTOMRIGHT;
	}

	if ( x < m_nFrameLeftRightSpace)
	{
		return	HTLEFT;
	}
	if ( x >= rc.right - m_nFrameLeftRightSpace)
	{
		return	HTRIGHT;
	}
	if ( y < m_nFrameTopBottomSpace)
	{
		return	HTTOP;
	}
	if ( y > rc.bottom - m_nFrameTopBottomSpace)
	{
		return	HTBOTTOM;
	}
	// 	if ( y <= m_nFrameTopSpace)
	// 	{
	// 		return	HTCAPTION;
	// 	}
	return	HTCLIENT;
	//	return CDialog::OnNcHitTest(point);
}

void CDlgBase::OnSize(UINT nType, int cx, int cy)
{
	CDialog::OnSize(nType, cx, cy);

	if (!IsIconic())	// 非最小化状态
	{
		BOOL bIsMaximize = IsZoomed();
		int border_offset[] = {/*3, 2, */1};
		if (bIsMaximize)
		{				
			SetupRegion(border_offset, 0);
			m_nFrameLeftRightSpace = m_nFrameTopBottomSpace = 0;
		}
		else
		{
			SetupRegion(border_offset, 1/*3*/);
			m_nFrameLeftRightSpace = m_nFrameTopBottomSpace = 3;	// 设置可以鼠标拖动改变窗口大小的区域宽度
		}	

		// 主窗口的透明渐变层蒙板在九宫格模式或非最大化情况下才会可见
		CControlBase *pControlBase = GetBaseControl(FRAME_MAINWND);
		if (pControlBase && (m_nFrameWLT == 0))
		{
			pControlBase->SetVisible(!bIsMaximize);
		}
	}

	// 设置窗口大小
	CRect rc;
	GetClientRect(&rc);
	SetRect(rc);
	OnSize(rc);

	// 调整各个控件的位置
	// 基础AREA的大小设置
	for (size_t i = 0; i < m_vecBaseArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseArea.at(i);
		if (pControlBase)
		{
			pControlBase->OnPositionChange();
			UpdateHover();
		}
	}
	// 基础控件的大小设置
	for (size_t i = 0; i < m_vecBaseControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecBaseControl.at(i);
		if (pControlBase)
		{
			pControlBase->OnPositionChange();
			UpdateHover();
		}
	}

	// 用户AREA的大小设置
	for (size_t i = 0; i < m_vecArea.size(); i++)
	{
		CControlBase * pControlBase = m_vecArea.at(i);
		if (pControlBase)
		{
			pControlBase->OnPositionChange();
			UpdateHover();
		}
	}
	// 用户控件的大小设置
	for (size_t i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecControl.at(i);
		if (pControlBase)
		{
			pControlBase->OnPositionChange();
			UpdateHover();
		}
	}

	InvalidateRect(NULL);
}

// 设置窗口区域
void CDlgBase::SetupRegion(int border_offset[], int nSize)
{
	CDC* pDC = GetDC();

	CRect	rc;
	GetWindowRect(rc);
	rc.OffsetRect(-rc.left, -rc.top);

	CRgn	rgn;
	rgn.CreateRectRgn(0, 0, rc.Width(), rc.Height());
	CRgn	rgn_xor;
	CRect	rcXor;
	
	for (int y = 0; y < nSize; ++y)
	{
		rcXor.SetRect(0, y, border_offset[y], y + 1);
		rgn_xor.CreateRectRgn(0, y, border_offset[y], y + 1);
		rgn.CombineRgn(&rgn, &rgn_xor, RGN_XOR);
		rgn_xor.DeleteObject();
	}

	for (int y = 0; y < nSize; ++y)
	{
		rcXor.SetRect(rc.right - border_offset[y], y, rc.right, y + 1);
		rgn_xor.CreateRectRgn(rc.right - border_offset[y], y, rc.right, y + 1);
		rgn.CombineRgn(&rgn, &rgn_xor, RGN_XOR);
		rgn_xor.DeleteObject();
	}

	for (int y = 0; y < nSize; ++y)
	{
		rcXor.SetRect(0, rc.bottom - y - 1, border_offset[y],  rc.bottom - y);
		rgn_xor.CreateRectRgn(0, rc.bottom - y - 1, border_offset[y],  rc.bottom - y);
		rgn.CombineRgn(&rgn, &rgn_xor, RGN_XOR);
		rgn_xor.DeleteObject();
	}

	for (int y = 0; y < nSize; ++y)
	{
		rcXor.SetRect(rc.right - border_offset[y], rc.bottom - y - 1, rc.right, rc.bottom -  y);
		rgn_xor.CreateRectRgn(rc.right - border_offset[y], rc.bottom - y - 1, rc.right,rc.bottom -  y);
		rgn.CombineRgn(&rgn, &rgn_xor, RGN_XOR);
		rgn_xor.DeleteObject();
	}
// 	HWND hWnd = GetSafeHwnd();
// 	SetWindowLong(hWnd,GWL_EXSTYLE,GetWindowLong(hWnd,GWL_EXSTYLE) | WS_EX_LAYERED);
// 	SetLayeredWindowAttributes(RGB(255, 0, 255), 0, LWA_COLORKEY );	

	SetWindowRgn((HRGN)rgn, TRUE);
	m_Rgn.DeleteObject();
	m_Rgn.Attach(rgn.Detach());
	ReleaseDC(pDC);
}

void CDlgBase::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS* lpncsp)
{
	CDialog::OnNcCalcSize(bCalcValidRects, lpncsp);

	CRect	rcWindow;

	if (bCalcValidRects && lpncsp->lppos)
	{
		rcWindow.SetRect(lpncsp->lppos->x, lpncsp->lppos->y,
			lpncsp->lppos->x + lpncsp->lppos->cx,
			lpncsp->lppos->y + lpncsp->lppos->cy);
	}
	else
	{
		GetWindowRect(rcWindow);
	}

	lpncsp->rgrc[0] = rcWindow;
}

void CDlgBase::OnWindowPosChanging(WINDOWPOS* lpwndpos)
{
	if (lpwndpos->cx < m_MinSize.cx)
	{
		lpwndpos->cx = m_MinSize.cx;
	}
	if (lpwndpos->cy < m_MinSize.cy)
	{
		lpwndpos->cy = m_MinSize.cy;
	}
}

void CDlgBase::OnWindowPosChanged(WINDOWPOS* lpwndpos)
{
	//	CDialog::OnWindowPosChanged(lpwndpos);

	::SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0,
		SWP_NOMOVE | SWP_NOSIZE | SWP_FRAMECHANGED
		| SWP_NOSENDCHANGING | SWP_NOACTIVATE);

	SendMessage(WM_MOVE, 0, MAKELPARAM(lpwndpos->x, lpwndpos->y));
	SendMessage(WM_SIZE, 0, MAKELPARAM(lpwndpos->cx, lpwndpos->cy));

	OnSize(0, lpwndpos->cx, lpwndpos->cy);
}

void CDlgBase::PostNcDestroy()
{

}

// 窗口关闭
void CDlgBase::OnClose()
{
	// 重载的WM_CLOSE消息处理函数,不做任何事情,避免通过Windows任务栏可以关闭程序
	CControlBase* pControlBase = GetBaseControl(NAME_BT_CLOSE);
	if(pControlBase)
	{
		if(pControlBase->GetAction() == ACTION_HIDE_WINDOW)
		{
			// 如果设置了动作为隐藏窗口,则只隐藏窗口
			ShowWindow(SW_HIDE);
		}else
		{
			// 关闭窗口
			DoClose();
		}
	}
}

// 自定义的窗口关闭消息处理
LRESULT CDlgBase::OnUserCloseWindow(WPARAM wParam, LPARAM lParam)
{
	// 删除所有定时器
	CTimer::KillTimer();
	// wParam参数表示对话框的返回值
	if(wParam == IDOK)
	{
		OnCancel();
	}else
	if(wParam == IDCANCEL)
	{
		OnOK();
	}else
	{
		EndDialog(wParam);
	}

	return 0;
}

// 窗口最小化
void CDlgBase::OnMinimize()
{
	ShowWindow(SW_MINIMIZE);
}

// 窗口最大化切换
BOOL CDlgBase::OnMaximize()
{

 	if (IsZoomed())
 	{
 		ShowWindow(SW_RESTORE);
		return FALSE;
 	}
 	else
 	{
 		ShowWindow(SW_SHOWMAXIMIZED);
		return TRUE;
 	}
}

// 窗口的皮肤选择
void CDlgBase::OnSkin()
{
	CControlBase* pControlBase = GetBaseControl(NAME_BT_SKIN);
	if(pControlBase == NULL)
	{
		return;
	}

	CDlgSkin *pDlgSkin = new CDlgSkin;
	CRect rc = pControlBase->GetRect();
	rc.OffsetRect(-95, rc.Height());
	ClientToScreen(&rc);
	pDlgSkin->LoadXmlFile(DuiSystem::Instance()->GetXmlFile("dlg_skin"));
	pDlgSkin->Create(this, rc, WM_SKIN);
	pDlgSkin->ShowWindow(SW_SHOW);
}

// 窗口最小化按钮的消息处理
LRESULT CDlgBase::OnMessageButtomMin(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(BUTTOM_UP == uMsg)
	{
		OnMinimize();
	}

	return 0;
}

// 换肤
LRESULT CDlgBase::OnMessageSkin(WPARAM wParam, LPARAM lParam)
{
	if(MSG_CLOSE == wParam)
	{
	}
	else if(MSG_SKIN == wParam)
	{
		// 选择了背景皮肤
		SelectInfo *pSelectInfo = (SelectInfo *)lParam;
		if(pSelectInfo != NULL)
		{
			if(pSelectInfo->nType == BKTYPE_COLOR)
			{
				Color &clr = pSelectInfo->clr;
				COLORREF crlBack = RGB(clr.GetRed(), clr.GetGreen(), clr.GetBlue());
				DrawBackground(crlBack);
				// 保存背景信息
				DuiSystem::Instance()->SetWindowBkInfo(BKTYPE_COLOR, 0, crlBack, _T(""));
				// 刷新所有窗口的背景皮肤
				DuiSystem::Instance()->ResetAllWindowsBkSkin();
			}
			else if(pSelectInfo->nType == BKTYPE_IMAGE_RESOURCE)
			{
				CString strImgFile;
				strImgFile.Format(_T("%s\\SKIN_PIC_%d.png"), DuiSystem::GetExePath() + _T("bkimg"), pSelectInfo->uIndex);
				LoadImage(strImgFile);
				// 保存背景信息
				DuiSystem::Instance()->SetWindowBkInfo(BKTYPE_IMAGE_FILE, 0, RGB(0,0,0), strImgFile);
				// 刷新所有窗口的背景皮肤
				DuiSystem::Instance()->ResetAllWindowsBkSkin();
			}
		}
	}
	else if(MSG_SELECT_SKIN == wParam)
	{
		// 选择皮肤文件
 		CFileDialog DlgFile(TRUE,NULL,NULL, OFN_OVERWRITEPROMPT | OFN_HIDEREADONLY ,
 			TEXT("Skin picture(*.bmp;*.png;*.jpg)|*.bmp;*.png;*.jpg||"),this, 0);
 		DlgFile.m_ofn.nFilterIndex=1;
 		DlgFile.m_ofn.hwndOwner=m_hWnd;
 		DlgFile.m_ofn.lStructSize=sizeof(OPENFILENAME);
 		DlgFile.m_ofn.lpstrTitle=TEXT("Open");
 		DlgFile.m_ofn.nMaxFile=MAX_PATH;
 		if(DlgFile.DoModal() == IDOK)
 		{
 			CString strFileName = DlgFile.GetPathName();
 			CString strFileType = strFileName.Right(3);
 			if (0 == strFileType.CompareNoCase(TEXT("bmp")) || 0 == strFileType.CompareNoCase(TEXT("jpg")) || 0 == strFileType.CompareNoCase(TEXT("png")))
 			{
 				LoadImage(strFileName);
				// 保存背景信息
				DuiSystem::Instance()->SetWindowBkInfo(BKTYPE_IMAGE_FILE, 0, RGB(0,0,0), strFileName);
				// 刷新所有窗口的背景皮肤
				DuiSystem::Instance()->ResetAllWindowsBkSkin();
 			}
 		}
	}

	return 0;
}

// 转UI线程处理的任务
LRESULT CDlgBase::OnMessageUITask(WPARAM wParam, LPARAM lParam)
{
	DuiVision::IBaseTask* pTask = (DuiVision::IBaseTask*)wParam;
	DuiVision::CTaskMgr* pTaskMgr = (DuiVision::CTaskMgr*)lParam;

	BOOL bRet = FALSE;
	if((pTask != NULL) && (pTaskMgr != NULL))
	{
		bRet = pTask->TaskProcess(pTaskMgr);
		if(bRet)
		{
			pTask->TaskNotify(pTaskMgr, DuiVision::IBaseTask::TE_Completed);
		}else
		{
			pTask->TaskNotify(pTaskMgr, DuiVision::IBaseTask::TE_Canceled);
		}
	}
	return bRet;
}

// Tray消息
LRESULT CDlgBase::OnSystemTrayIcon(WPARAM wParam, LPARAM lParam)
{
	switch(lParam)
	{
	case WM_LBUTTONDBLCLK:
		{
			CString strTrayDbClickMsg = DuiSystem::Instance()->GetConfig("trayDbClickMsg");
			if(strTrayDbClickMsg == _T("1"))
			{
				// 发托盘双击消息
				DuiSystem::AddDuiActionTask(TRAY_ICON, MSG_TRAY_DBCLICK, 0, 0, NAME_TRAY_ICON, _T(""), this);
			}else
			{
				// 显示窗口
				SetForegroundWindow();
				ShowWindow(SW_NORMAL);
				ShowWindow(SW_SHOW);
				BringWindowToTop();
			}
		}
		break;
	case WM_RBUTTONUP:
		{
			// 显示托盘菜单
			CString strXmlFile = DuiSystem::Instance()->GetXmlFile("menu_tray");
			if(!m_strTrayMenuXml.IsEmpty())	// 如果设置了托盘菜单文件,则使用设置的文件
			{
				strXmlFile = m_strTrayMenuXml;
				if(strXmlFile.Find(_T(".xml")) == -1)
				{
					strXmlFile = DuiSystem::Instance()->GetXmlFile(CEncodingUtil::UnicodeToAnsi(strXmlFile));
				}else
				if(strXmlFile.Find(_T(":")) == -1)
				{
					strXmlFile = _T("xml:") + strXmlFile;
				}
			}
			if(!strXmlFile.IsEmpty())
			{
				SetForegroundWindow();
				// 加载菜单控件
				CDuiMenu *pDuiMenu = new CDuiMenu();
				pDuiMenu->SetParent(this);
				if(m_pTrayHandler != NULL)
				{
					// 设置事件处理对象
					pDuiMenu->RegisterHandler(m_pTrayHandler);
				}
				CPoint point;
				GetCursorPos(&point);
				pDuiMenu->LoadXmlFile(strXmlFile, this, point, WM_DUI_MENU);
				// 计算菜单的位置并显示
				CRect rc;
				pDuiMenu->GetWindowRect(&rc);
				rc.OffsetRect(rc.Width()/2, -rc.Height());
				// 如果超出屏幕右侧范围,则菜单窗口往左移动一些
				int nScreenWidth= GetSystemMetrics(SM_CXFULLSCREEN);
				if(rc.right > nScreenWidth)
				{
					rc.OffsetRect(nScreenWidth - rc.right -10, 0);
				}
				pDuiMenu->MoveWindow(rc);
				pDuiMenu->ShowWindow(SW_SHOW);
			}
		}
		break;
	}
	return 0;
}

// 用于跨进程传递信息的注册消息,第二个DuiVision应用进程通过此消息将一些信息传递给正在运行的DuiVision应用进程
LRESULT CDlgBase::OnCheckItsMe(WPARAM wParam, LPARAM lParam)
{
	// wParam用于区分应用,lParam用于具体命令,传送的内容则通过内存映射文件传递
	// 此跨进称消息是通过广播方式传送的,因此所有窗口都会收到,只有主窗口需要处理此消息,其他窗口应该忽略
	// 主窗口收到消息之后调用事件处理对象进行处理

	// 判断是否主窗口
	if(DuiSystem::Instance()->GetDuiDialog(0) != this)
	{
		return FALSE;
	}

	// 判断是否当前的DuiVision应用
	if(wParam != DuiSystem::Instance()->GetAppID())
	{
		return FALSE;
	}

	// 获取内存映射文件的内容
	// number of characters in memory-mapped file
	const DWORD dwMemoryFileSize = sizeof(DUI_INTERPROCESS_MSG);

	// memory-mapped file name
	const LPCTSTR sMemoryFileName = _T("DF034858-1608-4147-0604-4A0CD86F6C9F");

	HANDLE hFileMapping = NULL;
	LPVOID pViewOfFile = NULL;

	// Create file mapping which can contain dwMemoryFileSize characters
	hFileMapping = CreateFileMapping(
		INVALID_HANDLE_VALUE,           // system paging file
		NULL,                           // security attributes
		PAGE_READWRITE,                 // protection
		0,                              // high-order DWORD of size
		dwMemoryFileSize*sizeof(TCHAR), // low-order DWORD of size
		sMemoryFileName);               // name

	DWORD dwError = GetLastError();     // if ERROR_ALREADY_EXISTS 
	// this instance is not first (other instance created file mapping)

	if(! hFileMapping)
	{
		//TRACE(_T("Creating of file mapping failed.\n"));
		return FALSE;
	}

	pViewOfFile = MapViewOfFile(
		hFileMapping,               // handle to file-mapping object
		FILE_MAP_ALL_ACCESS,        // desired access
		0,
		0,
		0);                         // map all file

	if(!pViewOfFile)
	{
		//TRACE(_T("MapViewOfFile failed.\n"));
		CloseHandle(hFileMapping);
		return FALSE;
	}

	DUI_INTERPROCESS_MSG interMsg;
	memcpy(&interMsg, pViewOfFile, sizeof(DUI_INTERPROCESS_MSG));

	UnmapViewOfFile(pViewOfFile);
	CloseHandle(hFileMapping);

	// 调用事件处理对象进行处理
	DuiSystem::Instance()->CallDuiHandler(interMsg.uControlID, interMsg.wControlName, interMsg.uMsg, interMsg.wParam, (LPARAM)(&interMsg));

	return TRUE;
}

// 窗口最大化按钮的消息处理
LRESULT CDlgBase::OnMessageButtomMax(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(BUTTOM_UP == uMsg)
	{
		OnMaximize();
	}

	return 0;
}

// 窗口关闭按钮的消息处理
LRESULT CDlgBase::OnMessageButtomClose(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(BUTTOM_UP == uMsg)
	{
		DoClose();
	}

	return 0;
}

// 窗口Skin按钮的消息处理
LRESULT CDlgBase::OnMessageButtomSkin(UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	if(BUTTOM_UP == uMsg)
	{
		OnSkin();
	}

	return 0;
}

BOOL CDlgBase::OnNcActivate(BOOL bActive)
{
	m_bNCActive = bActive;

	if(m_bNCActive)
	{
		
	}
	else
	{
		m_bTracking = false;
		m_bIsSetCapture = false;
		m_bIsLButtonDblClk = FALSE;
		
		if(m_bIsLButtonDown)
		{
			m_bIsLButtonDown = FALSE;
			if(m_pControl)
			{
				m_pControl->OnLButtonUp(0, CPoint(-1, -1));
				m_pControl = NULL;				
			}
		}	
		else
		{
			if(m_pControl)
			{
				m_pControl->OnMouseMove(0, CPoint(-1, -1));
				m_pControl = NULL;				
			}
		}		
	}
	InvalidateRect(NULL);

	return TRUE;	
}

void CDlgBase::PreSubclassWindow()
{
	// 判断是否允许拖拽图片文件作为窗口背景
	CString strEnableDragFile = DuiSystem::Instance()->GetConfig("enableDragFile");
	if(strEnableDragFile == _T("1"))
	{
		DragAcceptFiles(TRUE);
	}
	
	CDialog::PreSubclassWindow();
}

// 设置当前的Tooltip
void CDlgBase::SetTooltip(CControlBase* pControl, CString strTooltip, CRect rect, BOOL bControlWidth)
{
	m_wndToolTip.DelTool(this, 1);
	m_wndToolTip.Activate(FALSE);
	if(!strTooltip.IsEmpty())
	{
		//int nCount = m_wndToolTip.GetToolCount();
		if(bControlWidth)
		{
			m_wndToolTip.SetMaxTipWidth(pControl->GetRect().Width());
		}
		m_wndToolTip.AddTool(this, strTooltip, rect, 1);//Tooltip默认都用1作为ID, pControl->GetID());
		m_wndToolTip.Activate(TRUE);
		m_nTooltipCtrlID = pControl->GetID();
	}
}

// 清除当前的Tooltip
void CDlgBase::ClearTooltip()
{
	m_wndToolTip.DelTool(this, 1);
}

void CDlgBase::OnMouseMove(UINT nFlags, CPoint point) 
{	
	if (!m_bTracking)
	{
		TRACKMOUSEEVENT tme;
		tme.cbSize = sizeof(tme);
		tme.hwndTrack = m_hWnd;
		tme.dwFlags = TME_LEAVE | TME_HOVER;
		tme.dwHoverTime = 1;
		m_bTracking = ::_TrackMouseEvent(&tme);

		// 如果有窗口关闭定时器,则先停止定时器
		if(m_bTracking && (m_uTimerAutoClose != 0))
		{
			CTimer::KillTimer(m_uTimerAutoClose);
			m_uTimerAutoClose = 0;
		}
	}
	
	if (m_pControl)
	{
		/* 设置tooltip改为通过控件的OnMouseMove函数来设置
		if((m_nTooltipCtrlID != m_pControl->GetID()) && !m_pControl->GetTooltip().IsEmpty())
		{
			// 如果当前控件有Tooltip,则添加一个Tooltip,Tooltip的ID默认都用1
			m_wndToolTip.DelTool(this, 1);
			m_wndToolTip.Activate(FALSE);
			m_wndToolTip.AddTool(this, m_pControl->GetTooltip(), m_pControl->GetRect(), 1);
			m_wndToolTip.Activate(TRUE);
			m_nTooltipCtrlID = m_pControl->GetID();
		}*/

		if(((m_pControl->PtInRect(point) && m_pControl->OnCheckMouseResponse(nFlags, point)) || m_bIsLButtonDown) && m_bTracking)
		{			
			m_pControl->OnMouseMove(nFlags, point);
			return;
		}
	}

	if (!m_bIsLButtonDown)
	{
		CControlBase * pOldControl = m_pControl;
		BOOL bIsSelect = FALSE;
		BOOL bIsSystemSelect = FALSE;

		if(m_bTracking)
		{
			// 窗口自身的基础控件
			for (size_t i = 0; i < m_vecBaseControl.size(); i++)
			{
				CControlBase * pControlBase = m_vecBaseControl.at(i);
				if (pControlBase)
				{
					pControlBase->OnMouseMove(nFlags, point);
					if(pControlBase->PtInRect(point) && pControlBase->GetRresponse())
					{
						m_pControl = pControlBase;
						bIsSystemSelect = TRUE;
					}
				}		
			}
			
			// 用户控件
			for (size_t i = 0; i < m_vecControl.size(); i++)
			{
				CControlBase * pControlBase = m_vecControl.at(i);
				if (pControlBase)
				{
					pControlBase->OnMouseMove(nFlags, point);
					if (!bIsSystemSelect)
					{
						if(pControlBase->PtInRect(point) && pControlBase->GetRresponse())
						{
							m_pControl = pControlBase;
							bIsSelect = TRUE;
						}
					}
				}		
			}
		}

		if (!bIsSelect && !bIsSystemSelect)
		{
			m_pControl = NULL;
		}
	}
	
	//CDialog::OnMouseMove(nFlags, point);
}

LRESULT CDlgBase::OnMouseLeave(WPARAM wParam, LPARAM lParam)
{
	m_bTracking = FALSE;
	if (!m_bIsLButtonDown)
	{
		if (m_pControl)
		{
			m_pControl->OnMouseMove(0, CPoint(-1, -1));
		}
			
		m_pControl = NULL;
	}

	// 如果有窗口关闭或隐藏标志,则创建关闭定时器
	if((m_bAutoClose || m_bAutoHide) && (m_uAutoCloseDelayTime != 0) && (m_uTimerAutoClose == 0))
	{
		m_uTimerAutoClose = CTimer::SetTimer(m_uAutoCloseDelayTime);
	}
	
	return 0;
}

LRESULT CDlgBase::OnMouseHover(WPARAM wParam, LPARAM lParam)
{
 	//if (m_pControl)
 	//{
 	//	CPoint point;
 	//	GetCursorPos(&point);
 	//	//ScreenToClient(&point);
		//m_pControl->OnMouseMove(0, point);
 	//}
	return 0;
}

// 鼠标滚轮消息处理
BOOL CDlgBase::OnMouseWheel(UINT nFlags, short zDelta, CPoint pt)
{
	ScreenToClient(&pt);
 	zDelta /= WHEEL_DELTA;
	UINT nFlag = (zDelta>0) ? SB_LINEUP : SB_LINEDOWN;
	zDelta = abs(zDelta);
	BOOL bResponse = FALSE;
	for(short i = 0; i < zDelta; i++)
	{
		if (m_pControl && m_pControl->OnScroll(TRUE, nFlag, pt))
		{
			bResponse = TRUE;
			continue;
		}else
		{
			// 窗口自身的基础控件
			for (size_t i = 0; i < m_vecBaseControl.size(); i++)
			{
				CControlBase * pControlBase = m_vecBaseControl.at(i);
				if (pControlBase && pControlBase->OnScroll(TRUE, nFlag, pt))
				{
					bResponse = TRUE;
					break;
				}		
			}
			
			// 用户控件
			for (size_t i = 0; i < m_vecControl.size(); i++)
			{
				CControlBase * pControlBase = m_vecControl.at(i);
				if (pControlBase && pControlBase->OnScroll(TRUE, nFlag, pt))
				{
					bResponse = TRUE;
					break;
				}	
			}
		}
	}

    return bResponse;
}

void CDlgBase::OnLButtonDown(UINT nFlags, CPoint point)
{
	BOOL bIsSelect = false;

	if(m_pFocusControl != m_pControl && m_pFocusControl != NULL)
	{
		m_pFocusControl->OnFocus(false);
		m_pFocusControl = NULL;
	}
	/*// 切换焦点控件
	CControlBase* pFocusControl = GetFocusControl();
	if(pFocusControl != m_pFocusControl)
	{
		if(m_pFocusControl != NULL)
		{
			m_pFocusControl->OnFocus(FALSE);
		}
		if(pFocusControl != NULL)
		{
			pFocusControl->OnFocus(TRUE);
		}
		m_pFocusControl = pFocusControl;
	}*/

	if (m_pControl)
	{
		if(m_pControl->GetVisible() && m_pControl->GetRresponse())
		{
			if (m_pControl->PtInRect(point) && m_pControl->OnCheckMouseResponse(nFlags, point))
			{
				bIsSelect = TRUE;
				m_bIsLButtonDown = TRUE;

				m_pFocusControl = m_pControl;
				m_pControl->OnLButtonDown(nFlags, point);						
			}
		}
	}
	
	// 当前控件是否可以捕获鼠标
	if (bIsSelect && !m_bIsSetCapture && ((m_pControl == NULL) || m_pControl->OnCheckMouseResponse(nFlags, point)))
	{
		SetCapture();
		m_bIsSetCapture = TRUE;
		
		return;
	}

	// 窗口拖动消息
	PostMessage(WM_NCLBUTTONDOWN,HTCAPTION,MAKELPARAM(point.x, point.y));

	CDialog::OnLButtonDown(nFlags, point);
}

void CDlgBase::OnLButtonUp(UINT nFlags, CPoint point)
{
	if (m_bIsSetCapture)
	{
		ReleaseCapture();
		m_bIsSetCapture = false;
	}

	m_bIsLButtonDown = FALSE;

	if (m_pControl)
	{
		if(m_pControl->GetVisible() && m_pControl->GetRresponse())
		{
			CRect rc = m_pControl->GetRect();
			m_pControl->OnLButtonUp(nFlags, point);				

			if (!rc.PtInRect(point))
			{
				m_pControl = NULL;
			}
		}
		else
		{
			m_pControl = NULL;
		}	
	}
	
	m_bIsLButtonDblClk = FALSE;

	CDialog::OnLButtonUp(nFlags, point);
}

void CDlgBase::OnLButtonDblClk(UINT nFlags, CPoint point)
{
	m_bIsLButtonDblClk = TRUE;

	if(m_pControl)
	{
		if(!m_pControl->GetDblClk())
		{
			return OnLButtonDown(nFlags, point);
		}
	}

	//OnMaximize();

	CDialog::OnLButtonDblClk(nFlags, point);
}

// 键盘事件处理(移到PreTranslateMessage中实现子控件的键盘事件调用,因为对话框的OnKeyDown函数有局限性,不能捕获到ALT等组合键)
void CDlgBase::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
{
	CDialog::OnKeyDown(nChar, nRepCnt, nFlags);
}

BOOL CDlgBase::PreTranslateMessage(MSG* pMsg)
{
	if (( pMsg->message == WM_KEYDOWN ) || ( pMsg->message == WM_SYSKEYDOWN ))
	{
		// 键盘事件处理
		UINT nFlags = 0;
		BOOL bCtrl=::GetKeyState(VK_CONTROL)&0x8000;
		BOOL bShift=::GetKeyState(VK_SHIFT)&0x8000;
		BOOL bAlt=::GetKeyState(VK_MENU)&0x8000;	// ALT键只有WM_SYSKEYDOWN消息才能捕获到
		nFlags |= (bCtrl ? VK_CONTROL : 0);
		nFlags |= (bShift ? VK_SHIFT : 0);
		nFlags |= (bAlt ? VK_MENU : 0);

		// 处理tab键和shift+tab键,切换焦点
		if((pMsg->wParam == VK_TAB) && (nFlags == 0))
		{
			CControlBase* pFocusControl = GetFocusControl();
			CControlBase* pNextControl = GetNextFocusableControl();
			if(pNextControl != pFocusControl)
			{
				if(pFocusControl != NULL)
				{
					pFocusControl->OnFocus(FALSE);
				}
				if(pNextControl != NULL)
				{
					pNextControl->OnFocus(TRUE);
				}
				m_pFocusControl = pNextControl;
				return TRUE;
			}
		}else
		if((pMsg->wParam == VK_TAB) && (nFlags == VK_SHIFT))
		{
			CControlBase* pFocusControl = GetFocusControl();
			CControlBase* pPrevControl = GetPrevFocusableControl();
			if(pPrevControl != pFocusControl)
			{
				if(pFocusControl != NULL)
				{
					pFocusControl->OnFocus(FALSE);
				}
				if(pPrevControl != NULL)
				{
					pPrevControl->OnFocus(TRUE);
				}
				m_pFocusControl = pPrevControl;
				return TRUE;
			}
		}

		// 当前控件是否能处理
		if (m_pControl && m_pControl->OnKeyDown(pMsg->wParam, 1, nFlags))
		{
			return TRUE;
		}

		// 窗口自身的基础控件
		for (size_t i = 0; i < m_vecBaseControl.size(); i++)
		{
			CControlBase * pControlBase = m_vecBaseControl.at(i);
			if (pControlBase && pControlBase->OnKeyDown(pMsg->wParam, 1, nFlags))
			{
				return TRUE;
			}		
		}
		
		// 用户控件
		for (size_t i = 0; i < m_vecControl.size(); i++)
		{
			CControlBase * pControlBase = m_vecControl.at(i);
			if (pControlBase && pControlBase->OnKeyDown(pMsg->wParam, 1, nFlags))
			{
				return TRUE;
			}	
		}

		// 如果控件没有处理,则回车和ESC按键需要屏蔽掉
		if(pMsg->wParam == VK_ESCAPE) 
		{
			return TRUE;
		}
		if(pMsg->wParam == VK_RETURN)
		{
			return TRUE;
		}
	}else
	if ( pMsg->message == WM_MOUSEMOVE ||
		 pMsg->message == WM_LBUTTONDOWN ||
		 pMsg->message == WM_LBUTTONUP )
	{
		m_wndToolTip.RelayEvent(pMsg);
	}

	return CDialog::PreTranslateMessage(pMsg);
}

// 定时器消息
void CDlgBase::OnTimer(UINT uTimerID)
{
	if(m_uTimerAnimation == uTimerID)	// 动画定时器
	{
		for (size_t i = 0; i < m_vecBaseControl.size(); i++)
		{
			CControlBase * pControlBase = m_vecBaseControl.at(i);
			if (pControlBase)
			{
				pControlBase->OnTimer();
			}		
		}

		for (size_t i = 0; i < m_vecControl.size(); i++)
		{
			CControlBase * pControlBase = m_vecControl.at(i);
			if (pControlBase)
			{
				pControlBase->OnTimer();
			}		
		}
	}else
	if(m_uTimerAutoClose == uTimerID)	// 窗口自动关闭的定时器
	{
		CTimer::KillTimer(m_uTimerAutoClose);
		m_uTimerAutoClose = 0;
		if(m_bAutoHide)
		{
			ShowWindow(SW_HIDE);	// 隐藏窗口模式
		}else
		{
			DoClose();	// 关闭窗口模式
		}
	}
}

// 定时器消息(带定时器名字的定时函数)
void CDlgBase::OnTimer(UINT uTimerID, CString strTimerName)
{
	// 应用创建的定时器都调用事件处理对象的定时处理函数
	DuiSystem::Instance()->CallDuiHandlerTimer(uTimerID, strTimerName);
}

// 基础控件的消息响应
LRESULT CDlgBase::OnBaseMessage(UINT uID, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	CControlBase* pControlBase = GetBaseControl(uID);
	CControlBase* pControl = GetControl(uID);
	if((pControlBase == NULL) && (pControl == NULL))
	{
		return 0L;
	}

	if(pControlBase != NULL)
	{
		// 2.判断是否几个系统按钮,执行相应的系统操作
		if(pControlBase->IsThisObject(BT_MIN, NAME_BT_MIN))
		{
			OnMessageButtomMin(uMsg, wParam, lParam);
		}
		else if(pControlBase->IsThisObject(BT_MAX, NAME_BT_MAX))
		{
			OnMessageButtomMax(uMsg, wParam, lParam);
		}
		else if(pControlBase->IsThisObject(BT_CLOSE, NAME_BT_CLOSE))
		{
			if(pControlBase->GetAction() == ACTION_HIDE_WINDOW)
			{
				// 如果设置了动作为隐藏窗口,则只隐藏窗口
				ShowWindow(SW_HIDE);
			}else
			{
				// 关闭窗口
				OnMessageButtomClose(uMsg, wParam, lParam);
			}
		}
		else if(pControlBase->IsThisObject(BT_SKIN, NAME_BT_SKIN))
		{
			OnMessageButtomSkin(uMsg, wParam, lParam);
		}
		else
		{
			// 3.不是系统按钮,则调用各个对话框默认控件的事件处理函数
			for (size_t i = 0; i < m_vecBaseControl.size(); i++)
			{
				CControlBase * pControlBase = m_vecBaseControl.at(i);
				if (pControlBase && (pControlBase->GetControlID() == uID))
				{
					pControlBase->OnMessage(uID, uMsg, wParam, lParam);
				}
			}
		}
	}else
	if(pControl != NULL)
	{
		// 对话框控件的事件处理
		OnMessage(uID, uMsg, wParam, lParam);
	}

	return 0L; 
}

LRESULT CDlgBase::OnControlUpdate(CRect rcUpdate, BOOL bUpdate, CControlBase *pUpdateControlBase) 
{
	if(pUpdateControlBase == NULL) return 0;

	CRect rcAllUpDate = rcUpdate;
	if(bUpdate)
	{
		pUpdateControlBase->SetUpdate(FALSE, m_clrBK);
	}
	
	if(m_bInit)
	{
		BOOL bFind = false;
		for (size_t i = 0; i < m_vecArea.size(); i++)
		{
			CControlBase * pControlBase = m_vecArea.at(i);
			if (pControlBase)
			{
				if(bFind)
				{
					if (pControlBase->GetVisible() && !(pControlBase->GetRect() & rcUpdate).IsRectEmpty())
					{
						rcAllUpDate |= pControlBase->GetRect();
						pControlBase->SetUpdate(FALSE, m_clrBK);
					}
				}
				else if(pControlBase == pUpdateControlBase )
				{
					bFind = true;
				}
			}
		}

		for (size_t i = 0; i < m_vecBaseArea.size(); i++)
		{
			CControlBase * pControlBase = m_vecBaseArea.at(i);
			if (pControlBase)
			{
				if(bFind)
				{
					if (pControlBase->GetVisible() && !(pControlBase->GetRect() & rcUpdate).IsRectEmpty())
					{
						rcAllUpDate |= pControlBase->GetRect();
						pControlBase->SetUpdate(FALSE, m_clrBK);
					}
				}
				else if(pControlBase == pUpdateControlBase )
				{
					bFind = true;
				}
			}
		}

		for (size_t i = 0; i < m_vecControl.size(); i++)
		{
			CControlBase * pControlBase = m_vecControl.at(i);
			if (pControlBase)
			{
				if(bFind)
				{
					if (pControlBase->GetVisible() && !(pControlBase->GetRect() & rcUpdate).IsRectEmpty())
					{
						rcAllUpDate |= pControlBase->GetRect();
						pControlBase->SetUpdate(FALSE, m_clrBK);
					}
				}
				else if(pControlBase == pUpdateControlBase )
				{
					bFind = true;
				}		
			}
		}

		for (size_t i = 0; i < m_vecBaseControl.size(); i++)
		{
			CControlBase * pControlBase = m_vecBaseControl.at(i);
			if (pControlBase)
			{
				if(bFind)
				{
					if (pControlBase->GetVisible() && !(pControlBase->GetRect() & rcUpdate).IsRectEmpty())
					{
						rcAllUpDate |= pControlBase->GetRect();
						pControlBase->SetUpdate(FALSE, m_clrBK);
					}
				}
				else if(pControlBase == pUpdateControlBase )
				{
					bFind = true;
				}			
			}
		}	
	}

	InvalidateRect(&rcAllUpDate);

	return 0L; 
};

// DUI消息处理函数
LRESULT CDlgBase::OnMessage(UINT uID, UINT Msg, WPARAM wParam, LPARAM lParam)
{
	// 遍历每个控件，看哪个控件是此ID，则进行事件的处理
	CControlBase * pThisControlBase = NULL;
	for (size_t i = 0; i < m_vecControl.size(); i++)
	{
		CControlBase * pControlBase = m_vecControl.at(i);
		if (pControlBase && (pControlBase->GetControlID() == uID))
		{
			pThisControlBase = pControlBase;
			pControlBase->OnMessage(uID, Msg, wParam, lParam);
		}
	}

	return 0;
}

// 打开弹出对话框
void CDlgBase::OpenDlgPopup(CDlgPopup *pWndPopup, CRect rc, UINT uMessageID)
{
	ASSERT(pWndPopup);
	CloseDlgPopup();
	ClientToScreen(&rc);
	m_pWndPopup = pWndPopup;
	m_pWndPopup->Create(this, rc, uMessageID);
	m_pWndPopup->ShowWindow(SW_SHOW);
}

// 关闭弹出对话框
void CDlgBase::CloseDlgPopup()
{
	if(m_pWndPopup)
	{
 		if(m_pFocusControl && m_pControl != m_pFocusControl)
 		{
 			m_pFocusControl->OnFocus(false);
 			m_pFocusControl = NULL;
 		}
		if(IsWindow(m_pWndPopup->GetSafeHwnd()))
		{
			m_pWndPopup->CloseWindow();			
		}
	}
	m_pWndPopup = NULL;
}

void CDlgBase::OnDestroy()
{
	__super::OnDestroy();

	// 结束定时器
	CTimer::KillTimer();
}
