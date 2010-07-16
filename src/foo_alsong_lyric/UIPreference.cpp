#include "stdafx.h"
#include "ConfigStore.h"
#include "resource.h"
#include "UIPreference.h"
#include "UIWnd.h"

static BOOL CALLBACK UIConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);
static BOOL CALLBACK ConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam);

class preferences_page_instance_alsong_lyric : public preferences_page_instance
{
private:
	HWND m_hWnd;
	HWND hProp;
	preferences_page_callback::ptr m_callback;
public:
	preferences_page_instance_alsong_lyric(HWND parent, preferences_page_callback::ptr callback) : m_callback(callback)
	{
		m_hWnd = uCreateDialog(IDD_PREF, parent, _PrefConfigProc, (LPARAM)this);
	}

	virtual t_uint32 get_state()
	{
		return 0;
	}

	virtual HWND get_wnd()
	{
		return m_hWnd;
	}

	virtual void apply()
	{
		SendMessage(hProp, PSM_APPLY, NULL, NULL);
	}

	virtual void reset()
	{
		
	}

	static int CALLBACK PropCallback(HWND hWnd, UINT message, LPARAM lParam)
	{
		switch (message)
		{
		case PSCB_PRECREATE:
			{
				LPDLGTEMPLATE lpTemplate = (LPDLGTEMPLATE)lParam;
				DWORD dwOldProtect;
				VirtualProtect(lpTemplate, sizeof(DLGTEMPLATE), PAGE_READWRITE, &dwOldProtect);

				lpTemplate->style = DS_SETFONT | DS_FIXEDSYS | WS_CHILD | WS_SYSMENU | WS_VISIBLE | DS_3DLOOK;
				lpTemplate->dwExtendedStyle = 0;
				return TRUE;
			}
		case PSCB_INITIALIZED:
			{
				ShowWindow(GetDlgItem(hWnd, 0x00000001), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, 0x00000002), SW_HIDE);
				ShowWindow(GetDlgItem(hWnd, 0x00003021), SW_HIDE);

				//Property sheet bug. See http://support.microsoft.com/kb/149501
				SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_CONTROLPARENT);
				SetWindowPos(hWnd, NULL, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
			}
		}

		return 0;
	}

	BOOL PrefConfigProc(UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch(iMessage)
		{
		case WM_INITDIALOG:
			{
				PROPSHEETPAGE pages[2];
				HPROPSHEETPAGE hpages[2];
				pages[0].dwSize = sizeof(PROPSHEETPAGE);
				pages[0].dwFlags = PSP_DEFAULT | PSP_USETITLE;
				pages[0].hInstance = core_api::get_my_instance();
				pages[0].pszTemplate = MAKEINTRESOURCE(IDD_COMMON_PREF);
				pages[0].pfnDlgProc = &preferences_page_instance_alsong_lyric::CommonConfigProc;
				pages[0].pszTitle = TEXT("공통 설정");
				pages[0].lParam = NULL;

				pages[1].dwSize = sizeof(PROPSHEETPAGE);
				pages[1].dwFlags = PSP_DEFAULT | PSP_USETITLE;
				pages[1].hInstance = core_api::get_my_instance();
				pages[1].pszTemplate = MAKEINTRESOURCE(IDD_UI_PREF_COMMON);
				pages[1].pfnDlgProc = &UIPreference::ConfigProcDispatcher;
				pages[1].pszTitle = TEXT("외부 창 설정");
				pages[1].lParam = (LPARAM)(&cfg_outer.get_value());

				hpages[0] = CreatePropertySheetPage(&pages[0]);
				hpages[1] = CreatePropertySheetPage(&pages[1]);

				PROPSHEETHEADER psh;
				psh.dwSize = sizeof(psh);
				psh.dwFlags = PSH_DEFAULT | PSH_NOCONTEXTHELP | PSH_USEHICON | PSH_USECALLBACK | PSH_MODELESS;
				psh.hwndParent = m_hWnd;
				psh.hInstance = core_api::get_my_instance();
				psh.hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
				psh.pszCaption = TEXT("알송 가사 설정");
				psh.nPages = 2;
				psh.nStartPage = 0;
				psh.pfnCallback = &preferences_page_instance_alsong_lyric::PropCallback;
				psh.phpage = hpages;

				hProp = (HWND)PropertySheet(&psh);
			}
			return TRUE;
		case WM_SIZE:
			{
				RECT rt;
				GetWindowRect(m_hWnd, &rt);
				HWND hTab = (HWND)SendMessage(hProp, PSM_GETTABCONTROL, 0, 0);
				MoveWindow(hProp, 0, 0, rt.right - rt.left, rt.bottom - rt.top, TRUE);
				MoveWindow(hTab, 10, 3, rt.right - rt.left - 15, rt.bottom - rt.top - 5, TRUE);
			}
			return TRUE;
		case WM_DESTROY:
			SendMessage(hProp, PSM_APPLY, NULL, NULL);//마지막 위치 저장 -> 열때 탭복구
			return TRUE;
		}
		return FALSE;
	}

	static BOOL CALLBACK _PrefConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		static preferences_page_instance_alsong_lyric *_this = NULL;
		if(iMessage == WM_INITDIALOG)
		{
			_this = (preferences_page_instance_alsong_lyric *)lParam;
			_this->m_hWnd = hWnd;
		}
		if(_this)
			return _this->PrefConfigProc(iMessage, wParam, lParam);
		
		return FALSE;
	}

	static BOOL CALLBACK CommonConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
	{
		switch (iMessage)
		{
		case WM_INITDIALOG:
			CheckDlgButton(hWnd, IDC_SAVELRC, cfg_save_to_lrc);
			CheckDlgButton(hWnd, IDC_LOADFROMLRC, cfg_load_from_lrc);
			uSetDlgItemText(hWnd, IDC_LRCPATH, cfg_lrc_save_path);
			CheckDlgButton(hWnd, IDC_MIMIC, cfg_mimic_lyricshow);
			CheckDlgButton(hWnd, IDC_SAVETOFILE, cfg_lyric_savetofile);
			break;

		case WM_NOTIFY:
			if(((LPNMHDR)lParam)->code == PSN_APPLY)
			{
				cfg_load_from_lrc = (IsDlgButtonChecked(hWnd, IDC_LOADFROMLRC) ? true : false);
				cfg_save_to_lrc = (IsDlgButtonChecked(hWnd, IDC_SAVELRC) ? true : false);
				cfg_lrc_save_path = uGetDlgItemText(hWnd, IDC_LRCPATH).get_ptr();
				cfg_mimic_lyricshow = (IsDlgButtonChecked(hWnd, IDC_MIMIC) ? true : false);

				SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
			}
			else if(((LPNMHDR)lParam)->code == PSN_KILLACTIVE)
			{
				SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
			}
			break;
		default:
			return FALSE;
		}
		return TRUE;
	}
};

// {A58D6A8E-5932-4def-AD63-44185988B105}
static const GUID guid_prefs_alsong_lyric = { 0xa58d6a8e, 0x5932, 0x4def, { 0xad, 0x63, 0x44, 0x18, 0x59, 0x88, 0xb1, 0x5 } };

class preferences_page_alsong_lyric : public preferences_page_v3
{
public:
	virtual const char * get_name()
	{
		return "Alsong Live Lyric";
	}

	virtual GUID get_guid()
	{
		return guid_prefs_alsong_lyric;
	}

	virtual GUID get_parent_guid()
	{
		return preferences_page::guid_tools;
	}

	virtual preferences_page_instance::ptr instantiate(HWND parent, preferences_page_callback::ptr callback)
	{
		return new service_impl_t<preferences_page_instance_alsong_lyric>(parent, callback);
	}
};

static preferences_page_factory_t<preferences_page_alsong_lyric> foo_preferences_page_alsong_lyric;

void UpdateOuterWindowStyle(HWND hWnd)
{
	SetWindowLong(hWnd, GWL_STYLE, (cfg_outer_border ? WS_POPUP | WS_SYSMENU | WS_MINIMIZEBOX : WS_OVERLAPPEDWINDOW & ~WS_MAXIMIZEBOX));
	//TODO: 작업 표시줄, Alt+Tab에서 없애기

	//100%투명도 아닐경우에만 적용. 항상위 강제
	if(cfg_outer_layered == true && cfg_outer_transparency != 100 && cfg_outer_nolayered == false)
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_TRANSPARENT | WS_EX_TOPMOST);
		if(cfg_outer_shown)
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_FRAMECHANGED);
		else
			SetWindowPos(hWnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
	}
	else
	{
		SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) & ~WS_EX_TRANSPARENT & (cfg_outer_topmost ? 0xFFFFFFFF : ~WS_EX_TOPMOST) & (cfg_outer_nolayered ? ~WS_EX_LAYERED : 0xFFFFFFFF));
		if(cfg_outer_shown)
			SetWindowPos(hWnd, (cfg_outer_topmost ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOSIZE | SWP_SHOWWINDOW | SWP_NOMOVE | SWP_FRAMECHANGED);
		else
			SetWindowPos(hWnd, (cfg_outer_topmost ? HWND_TOPMOST : HWND_NOTOPMOST), 0, 0, 0, 0, SWP_NOSIZE | SWP_NOMOVE | SWP_FRAMECHANGED);
	}
	SetLayeredWindowAttributes(hWnd, NULL, (255 * cfg_outer_transparency) / 100, LWA_ALPHA);

	InvalidateRect(hWnd, NULL, TRUE);
}

//TODO: 줄간격 설정
BOOL UIPreference::UIConfigProc(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam, HWND hParent)
{
	static UIPreference OldSetting;

	static int old_transparency;
	static bool old_layered, old_border;

	switch (iMessage)
	{
	case WM_INITDIALOG:
		{
			memcpy(&OldSetting, this, sizeof(UIPreference));
			old_transparency = cfg_outer_transparency;
			old_layered = cfg_outer_layered;
			old_border = cfg_outer_border;

			SendMessage(GetDlgItem(hWnd, IDC_NLINESPIN), UDM_SETRANGE32, 1, 20);
			SendMessage(GetDlgItem(hWnd, IDC_NLINESPIN), UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, IDC_NLINE), 0);
			SendMessage(GetDlgItem(hWnd, IDC_NLINESPIN), UDM_SETPOS32, 0, GetnLine());

			SendMessage(GetDlgItem(hWnd, IDC_MARGINSPIN), UDM_SETRANGE32, 0, 100);
			SendMessage(GetDlgItem(hWnd, IDC_MARGINSPIN), UDM_SETBUDDY, (WPARAM)GetDlgItem(hWnd, IDC_LINEMARGIN), 0);
			SendMessage(GetDlgItem(hWnd, IDC_MARGINSPIN), UDM_SETPOS32, 0, GetLineMargin());

//			if(Setting->Script)
//				uSetDlgItemText(hWnd, IDC_UISCRIPT, Setting->Script->get_ptr());
			if(GetParent(hParent) == NULL)
			{
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_SETRANGE, TRUE, MAKELONG(0, 100));
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_SETPOS, TRUE, cfg_outer_transparency);

				int pos;
				pos = SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_GETPOS, 0, 0);
				TCHAR temp[255];
				wsprintf(temp, TEXT("%d%%"), pos);
				SetWindowText(GetDlgItem(hWnd, IDC_TRANSPARENCY_LABEL), temp);

				CheckDlgButton(hWnd, IDC_LAYERED, cfg_outer_layered);
				CheckDlgButton(hWnd, IDC_BORDER, cfg_outer_border);
				CheckDlgButton(hWnd, IDC_NOLAYERED, cfg_outer_nolayered);
			}
			else
			{
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY_STATIC), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY_LABEL), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_LAYERED), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_BORDER), WM_CLOSE, 0, 0);
				SendMessage(GetDlgItem(hWnd, IDC_NOLAYERED), WM_CLOSE, 0, 0);
			}

			SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("위"));
			SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("가운데"));
			SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("아래"));
			SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("왼쪽"));
			SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("가운데"));
			SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_ADDSTRING, NULL, (LPARAM)TEXT("오른쪽"));

			SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_SETCURSEL, GetVerticalAlign() - 1, NULL);
			SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_SETCURSEL, GetHorizentalAlign() - 1, NULL);

			SendMessage(GetDlgItem(hWnd, IDC_MANUALSCRIPT), WM_CLOSE, 0, 0);
			SendMessage(GetDlgItem(hWnd, IDC_UISCRIPT), WM_CLOSE, 0, 0);

			SendMessage(GetDlgItem(hWnd, IDC_FONTINDICATOR), WM_SETFONT, (WPARAM)GetFont(), TRUE);

			SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_ADDSTRING, NULL, (LPARAM)TEXT("색"));
			SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_ADDSTRING, NULL, (LPARAM)TEXT("그림"));
			if(GetParent(hParent) != NULL)
				SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_ADDSTRING, NULL, (LPARAM)TEXT("없음"));

			SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_SETCURSEL, GetBgType(), NULL);
		}

		break;
	case WM_CTLCOLORSTATIC:
		if((HWND)lParam == GetDlgItem(hWnd, IDC_FGINDICATOR))
			return (INT_PTR)CreateSolidBrush(GetFgColor());
		if((HWND)lParam == GetDlgItem(hWnd, IDC_BKINDICATOR))
			return (INT_PTR)CreateSolidBrush(GetBkColor());
		return FALSE;
	case WM_PAINT:
		{
			PAINTSTRUCT ps;
			HDC hdc = BeginPaint(hWnd, &ps);
			Gdiplus::Graphics g((HDC)hdc);
			Gdiplus::Image im(GetBgImagePath());

			g.DrawImage(&im, 20, 270, 200, 120);
			EndPaint(hWnd, &ps);
		}
		break;
	case WM_HSCROLL:
		if(GetParent(hParent) == NULL)
		{
			int pos;
			pos = SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_GETPOS, 0, 0);
			TCHAR temp[255];
			wsprintf(temp, TEXT("%d%%"), pos);
			SetWindowText(GetDlgItem(hWnd, IDC_TRANSPARENCY_LABEL), temp);
			SendMessage(GetParent(hWnd), PSM_CHANGED, (WPARAM)hWnd, 0);
		}
		break;
	case WM_COMMAND:
		if(HIWORD(wParam) == EN_CHANGE || HIWORD(wParam) == BN_CLICKED || HIWORD(wParam) == CBN_SELCHANGE)
			SendMessage(GetParent(hWnd), PSM_CHANGED, (WPARAM)hWnd, 0);
		if(HIWORD(wParam) == BN_CLICKED)
		{
			switch(LOWORD(wParam))
			{
			case IDC_BKCOLOR:
				OpenBkColorPopup(hWnd);
				InvalidateRect(GetDlgItem(hWnd, IDC_BKINDICATOR), NULL, TRUE);
				break;
			case IDC_FONTCHANGE:
				OpenFontPopup(hWnd);
				SendMessage(GetDlgItem(hWnd, IDC_FONTINDICATOR), WM_SETFONT, (WPARAM)GetFont(), TRUE);
				break;
			case IDC_FGCOLOR:
				OpenFgColorPopup(hWnd);
				InvalidateRect(GetDlgItem(hWnd, IDC_FGINDICATOR), NULL, TRUE);
				break;
			case IDC_BGIMAGE:
				OpenBgImagePopup(hWnd);
				InvalidateRect(hWnd, NULL, TRUE);
				break;
			}
		}
		break;
	case WM_NOTIFY:
		if(((LPNMHDR)lParam)->code == PSN_APPLY)
		{
			nLine = SendMessage(GetDlgItem(hWnd, IDC_NLINESPIN), UDM_GETPOS32, NULL, NULL);
			LineMargin = SendMessage(GetDlgItem(hWnd, IDC_MARGINSPIN), UDM_GETPOS32, NULL, NULL);
			if(GetParent(hParent) == NULL)
			{
				cfg_outer_transparency = SendMessage(GetDlgItem(hWnd, IDC_TRANSPARENCY), TBM_GETPOS, 0, 0);
				cfg_outer_layered = (IsDlgButtonChecked(hWnd, IDC_LAYERED) ? true : false);
				cfg_outer_border = (IsDlgButtonChecked(hWnd, IDC_BORDER) ? true : false);
				cfg_outer_nolayered = (IsDlgButtonChecked(hWnd, IDC_NOLAYERED) ? true : false);
				UpdateOuterWindowStyle(WndInstance.GetHWND());
			}

			VerticalAlign = static_cast<AlignPosition>(SendMessage(GetDlgItem(hWnd, IDC_VERTICALALIGN), CB_GETCURSEL, NULL, NULL) + 1);
			HorizentalAlign = static_cast<AlignPosition>(SendMessage(GetDlgItem(hWnd, IDC_HORIZENTALALIGN), CB_GETCURSEL, NULL, NULL) + 1);
			bgType = static_cast<BgType>(SendMessage(GetDlgItem(hWnd, IDC_BGTYPE), CB_GETCURSEL, NULL, NULL));
//			if(Setting->Script)
//				uGetDlgItemText(hWnd, IDC_UISCRIPT, *(Setting->Script));

			InvalidateRect(hParent, NULL, TRUE);

			SetWindowLong(hWnd, DWL_MSGRESULT, PSNRET_NOERROR);
		}
		else if(((LPNMHDR)lParam)->code == PSN_KILLACTIVE)
		{
			SetWindowLong(hWnd, DWL_MSGRESULT, FALSE);
		}
		else if(((LPNMHDR)lParam)->code == PSN_RESET)
		{
			memcpy(this, &OldSetting, sizeof(UIPreference));
			if(GetParent(hParent) == NULL)
			{
				cfg_outer_transparency = old_transparency;
				cfg_outer_layered = old_layered;
				cfg_outer_border = old_border;

				UpdateOuterWindowStyle(WndInstance.GetHWND());
			}
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}

static HWND g_ConfigParent = NULL; //TODO

BOOL CALLBACK UIPreference::ConfigProcDispatcher(HWND hWnd, UINT iMessage, WPARAM wParam, LPARAM lParam)
{
	static UIPreference *_this = NULL;
	if(iMessage == WM_INITDIALOG)
		_this = (UIPreference *)((PROPSHEETPAGE *)lParam)->lParam;

	if(iMessage == WM_DESTROY)
	{
		_this = NULL;
		g_ConfigParent = NULL;
	}

	if(_this)
		return _this->UIConfigProc(hWnd, iMessage, wParam, lParam, g_ConfigParent);
	else
		return FALSE;
}

void UIPreference::OpenConfigPopup(HWND hParent)
{
	if(g_ConfigParent)
	{
		MessageBox(hParent, TEXT("Cannot open more than two config popup"), TEXT("error"), MB_OK);
		return;
	}
	g_ConfigParent = hParent;

	HPROPSHEETPAGE pages[1];
	PROPSHEETPAGE page;
	page.dwSize = sizeof(PROPSHEETPAGE);
	page.dwFlags = PSP_DEFAULT;
	page.hInstance = core_api::get_my_instance();
	page.pszTemplate = MAKEINTRESOURCE(IDD_UI_PREF_COMMON);
	page.pfnDlgProc = &UIPreference::ConfigProcDispatcher;
	page.lParam = (LPARAM)this;
	pages[0] = CreatePropertySheetPage(&page);

	PROPSHEETHEADER psh;
	psh.dwSize = sizeof(psh);
	psh.dwFlags = PSH_DEFAULT | PSH_NOCONTEXTHELP | PSH_USEHICON;
	psh.hwndParent = hParent;
	psh.hInstance = core_api::get_my_instance();
	psh.hIcon = static_api_ptr_t<ui_control>()->get_main_icon();
	psh.pszCaption = TEXT("고급 설정");
	psh.nPages = 1;
	psh.nStartPage = 0;
	psh.phpage = pages;

	int nRet = PropertySheet(&psh);
}
