/*===================================================================
  KeymacsProp.c -- Copyright(c) 1999 Shinsuke Noguchi <noguchi@i.am>
===================================================================*/
#include <windows.h>
#include <windowsx.h>
#include <commctrl.h>
#include "resource.h"
#include "Keymacs.h"
#include "KeymacsProp.h"
#include "dll/KeymacsDll.h"

static HWND hLVDesktop, hLVIgnore;	// リストビューのハンドラ
static const DWORD KEYMAP[] =		// チェックボックスのリソースIDと同じ順番であること!!
{
	KMF_UP, KMF_DOWN, KMF_LEFT, KMF_RIGHT, KMF_PGUP, KMF_PGDN, KMF_HOME, KMF_END, 
	KMF_DEL, KMF_BKSP, KMF_CUT, KMF_COPY, KMF_PASTE, KMF_ENTER, KMF_UNDO, KMF_ESC, 
	KMF_CUTEND, KMF_MARKSET, KMF_TAB, KMF_SEARCH
};
static const int numKEYMAP = sizeof(KEYMAP)/sizeof(KEYMAP[0]);



/*===============================
  プロパティシートの作成
===============================*/
void MakeKeymacsProp(HWND hWnd)
{
	HINSTANCE hIns;
    PROPSHEETPAGE psp;
    PROPSHEETHEADER psh;
    HPROPSHEETPAGE hpsp[2];

	hIns = (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE );

	psp.dwSize = sizeof(PROPSHEETPAGE);
    psp.dwFlags = PSP_DEFAULT;
    psp.hInstance = hIns;
    
    psp.pszTemplate = MAKEINTRESOURCE(IDD_KEYMAP);
    psp.pfnDlgProc = (DLGPROC)DlgKeymapProc;
    hpsp[0] = CreatePropertySheetPage(&psp);

    psp.pszTemplate = MAKEINTRESOURCE(IDD_IGNORE);
    psp.pfnDlgProc = (DLGPROC)DlgIgnoreProc;
    hpsp[1] = CreatePropertySheetPage(&psp);

	ZeroMemory((void *)&psh, sizeof(psh));
    psh.dwSize = sizeof(PROPSHEETHEADER);
    psh.dwFlags = PSH_DEFAULT;
    psh.hInstance = hIns;
    psh.hwndParent = hWnd;
    psh.nPages = 2;
    psh.phpage = hpsp;
    psh.pszCaption = "Keymacs - 設定";

    (HWND)PropertySheet(&psh);
}



/*============================
  キーマップダイアログ
============================*/
static LRESULT CALLBACK DlgKeymapProc(HWND hDlgWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG( hDlgWnd, WM_INITDIALOG, DlgKeymapOnInitdialog );
		HANDLE_MSG( hDlgWnd, WM_COMMAND, DlgKeymapOnCommand );
		HANDLE_MSG( hDlgWnd, WM_NOTIFY, DlgKeymapOnNotify );
    }
    return FALSE;
}


/*======================================================
 WM_INITDIALOGのハンドラ
 引数: (hwnd), (HWND)(wParam), lParam
 1. チェックボックスの初期化
======================================================*/
static BOOL DlgKeymapOnInitdialog( HWND hDlgWnd, HWND hWndFocus, LPARAM lParam )
{
	int i;

	for( i=0; i<numKEYMAP; i++ )
		SendMessage( GetDlgItem(hDlgWnd, IDC_CHECK1+i), BM_SETCHECK, (WPARAM)((fKeyMap & KEYMAP[i]) != 0), 0L );
	SetDlgItemText( hDlgWnd, IDC_EDIT_TEST, "ここでテストできます\r\nただし適用ボタンを押してからでないと\r\n変更は反映されません" );
	return TRUE;
}

/*=========================================================================
 WM_COMMANDのハンドラ
 引数: (hwnd), (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)
 1.ボタンを押したときの処理
 2.プロパティシートへの通知
=========================================================================*/
static void DlgKeymapOnCommand( HWND hDlgWnd, int id, HWND hwndCtrl,UINT codeNotify )
{
	int i;

	switch(id)
	{
	case IDC_ALL:
		for( i=0; i<numKEYMAP; i++ )
			SendMessage( GetDlgItem(hDlgWnd, IDC_CHECK1+i), BM_SETCHECK, (WPARAM)1, 0L );
		break;
	case IDC_CLEAR:
		for( i=0; i<numKEYMAP; i++ )
			SendMessage( GetDlgItem(hDlgWnd, IDC_CHECK1+i), BM_SETCHECK, (WPARAM)0, 0L );
		break;
	default:
		break;
	}
	PropSheet_Changed(GetParent(hDlgWnd), hDlgWnd);
}



/*======================================================
 WM_NOTIFYのハンドラ
 引数: (hwnd), (int)(wParam), (NMHDR FAR*)(lParam)
 1. プロパティシートの「OK」「適用」が押されたときの処理
======================================================*/
static BOOL DlgKeymapOnNotify( HWND hDlgWnd, int id, NMHDR FAR* hdr )
{
	int i;
	HKEY	hRegkey;
	DWORD	dwDisposition;
	DWORD	dwSize;

	switch( hdr->code )
	{
	case PSN_APPLY:
		// 設定の反映
		fKeyMap = 0;
		for( i=0; i<numKEYMAP; i++ )
			if(IsDlgButtonChecked(hDlgWnd, IDC_CHECK1+i) == BST_CHECKED) fKeyMap |= KEYMAP[i];
		(*pSetKeyMap)(fKeyMap);

		
		// レジストリのオープン
		if( RegCreateKeyEx(
			HKEY_CURRENT_USER, "Software\\Guchi\\Keymacs", 0, "", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,	NULL, &hRegkey, &dwDisposition) != ERROR_SUCCESS )
			MessageBox(HWND_DESKTOP, "レジストリのオープンに失敗しました", NULL, MB_ICONEXCLAMATION);
		// レジストリに書き込み
		dwSize = sizeof(fKeyMap);
		RegSetValueEx( hRegkey, "keymap", 0, REG_DWORD, (LPBYTE)&fKeyMap, dwSize);
		// レジストリキーのクローズ
		RegCloseKey(hRegkey);

		return TRUE;
	}
	return FALSE;
}











/*===========================
  無視リストダイアログ
===========================*/
static LRESULT CALLBACK DlgIgnoreProc(HWND hDlgWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
		HANDLE_MSG( hDlgWnd, WM_INITDIALOG, DlgIgnoreOnInitdialog );
		HANDLE_MSG( hDlgWnd, WM_COMMAND, DlgIgnoreOnCommand );
		HANDLE_MSG( hDlgWnd, WM_NOTIFY, DlgIgnoreOnNotify );
    }
    return FALSE;
}


// 「起動しているアプリ」リストにアイテム挿入
static void InsertDesktopItem( LPTSTR lpClass, LPTSTR lpTitle, int index )
{
	LV_ITEM item;

	item.mask = LVIF_TEXT;
	item.pszText = lpClass;
	item.iItem = index;
	item.iSubItem = 0;
	ListView_InsertItem(hLVDesktop, &item);

	item.pszText = lpTitle;
	item.iItem = index;
	item.iSubItem = 1;
	ListView_SetItem(hLVDesktop, &item);
}


// 「起動しているアプリ」の読み込みと表示
static void showDesktopList()
{
	HWND hWndDesktop;
	HWND hWnd;
	char ClassName[IGNORE_NAMELEN];
	char TitleName[64];

	hWndDesktop = GetDesktopWindow();
	hWnd = GetTopWindow(hWndDesktop);
	for( 0; hWnd; hWnd=GetNextWindow(hWnd, GW_HWNDNEXT) )
	{
		if( GetClassName( hWnd, ClassName, sizeof(ClassName) ) && 
			GetWindowText( hWnd, TitleName, sizeof(TitleName) ) &&
			IsWindowVisible( hWnd ) )
		{
			InsertDesktopItem( ClassName, TitleName, 0 );
			ClassName[0] = '\0';
		}
	}
}

// 「無視するアプリ」リストにアイテム挿入
static void InsertIgnoreItem( LPTSTR lpClass, int index )
{
	LV_ITEM item;

	item.mask = LVIF_TEXT;
	item.pszText = lpClass;
	item.iItem = index;
	item.iSubItem = 0;
	ListView_InsertItem(hLVIgnore, &item);
}

// 「無視するアプリ」リストの表示
static void showIgnoreList()
{
	int i;

	for( i=0; i<IGNORE_MAX; i++ )
	{
		if( IgnoreApp[i][0] == '\0' ) break;
		InsertIgnoreItem( IgnoreApp[i], 0 );
	}
}


/*=========================
 WM_INITDAIALOGハンドラ
 リストビューの初期化
=========================*/
static BOOL DlgIgnoreOnInitdialog( HWND hDlgWnd, HWND hWndFocus, LPARAM lParam )
{
	LV_COLUMN lvcol;
	DWORD lstyle;

	hLVDesktop = GetDlgItem(hDlgWnd,IDC_LIST_DESKTOP);
	hLVIgnore = GetDlgItem( hDlgWnd, IDC_LIST_IGNORE );

	lvcol.mask = LVCF_FMT | LVCF_SUBITEM | LVCF_TEXT | LVCF_WIDTH;
	lvcol.fmt = LVCFMT_LEFT;

	lvcol.cx = 90;
	lvcol.pszText = "クラス名";
	lvcol.iSubItem = 0;
	ListView_InsertColumn(hLVDesktop, 0, &lvcol);
	ListView_InsertColumn(hLVIgnore, 0, &lvcol);

	lvcol.cx = 125;
	lvcol.pszText = "タイトル";
	lvcol.iSubItem = 1;
	ListView_InsertColumn(hLVDesktop, 1, &lvcol);

	//行選択できるようにする
	lstyle = SendMessage( hLVDesktop, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	lstyle = lstyle | LVS_EX_FULLROWSELECT;
	SendMessage(hLVDesktop, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lstyle);

	lstyle = SendMessage( hLVIgnore, LVM_GETEXTENDEDLISTVIEWSTYLE, 0, 0 );
	lstyle = lstyle | LVS_EX_FULLROWSELECT;
	SendMessage(hLVIgnore, LVM_SETEXTENDEDLISTVIEWSTYLE, 0, lstyle);

	showDesktopList();
	showIgnoreList();
	return TRUE;
}

/*=====================
 WM_COMMANDハンドラ
 ボタン押し下げの処理
=====================*/
static void DlgIgnoreOnCommand( HWND hDlgWnd, int id, HWND hwndCtrl,UINT codeNotify )
{
	int nActiveIndex=-1;
	char ClassStr[IGNORE_NAMELEN];

	switch(id)
	{
	case IDC_ADD:
		nActiveIndex = ListView_GetNextItem( hLVDesktop, -1, LVNI_SELECTED );
		if( nActiveIndex >= 0 )
		{
			ListView_GetItemText( hLVDesktop, nActiveIndex, 0, ClassStr, sizeof(ClassStr) );
			ListView_DeleteItem( hLVDesktop, nActiveIndex );
			InsertIgnoreItem( ClassStr, 0 );
			ClassStr[0] = '\0';
		}
		break;

	case IDC_DEL:
		nActiveIndex = ListView_GetNextItem( hLVIgnore, -1, LVNI_SELECTED );
		if( nActiveIndex >= 0 )
		{
			ListView_GetItemText( hLVIgnore, nActiveIndex, 0, ClassStr, sizeof(ClassStr) );
			ListView_DeleteItem( hLVIgnore, nActiveIndex );
			InsertDesktopItem( ClassStr, "不明", 0 );
			ClassStr[0] = '\0';
		}
		break;

	case IDC_RELOAD:
		ListView_DeleteAllItems( hLVDesktop );
		showDesktopList();
		break;

	case IDC_CLEAR:
		ListView_DeleteAllItems( hLVIgnore );
		break;
	}
	PropSheet_Changed(GetParent(hDlgWnd), hDlgWnd);
}

/*====================================
 WM_NOTIFYハンドラ
 プロパティシートのボタン押し下げ処理
====================================*/
static BOOL DlgIgnoreOnNotify( HWND hDlgWnd, int id, NMHDR FAR* hdr )
{
	int i;
	char ClassStr[IGNORE_NAMELEN];
	HKEY	hRegkey;
	DWORD	dwDisposition;
	DWORD	dwSize;
	
	switch( hdr->code )
	{
	// 適用ボタン押下の処理
	case PSN_APPLY:
		// 設定の反映
		ZeroMemory( &IgnoreApp, sizeof(IgnoreApp) );
		for( i=ListView_GetNextItem(hLVIgnore, -1, LVNI_ALL);
			 i!=-1 && i<IGNORE_MAX;
			 i=ListView_GetNextItem(hLVIgnore, i, LVNI_ALL) )
		{
			ListView_GetItemText( hLVIgnore, i, 0, ClassStr, sizeof(ClassStr) );
			lstrcpy( IgnoreApp[i], ClassStr );
		}
		(*pSetIgnoreApp)(IgnoreApp);

		// レジストリのオープン
		if( RegCreateKeyEx(
			HKEY_CURRENT_USER, "Software\\Guchi\\Keymacs", 0, "", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,	NULL, &hRegkey, &dwDisposition) != ERROR_SUCCESS )
			MessageBox(HWND_DESKTOP, "レジストリのオープンに失敗しました", NULL, MB_ICONEXCLAMATION);
		// レジストリへ書き込み
		dwSize = sizeof(IgnoreApp);
		RegSetValueEx( hRegkey, "ignore", 0, REG_BINARY, (LPBYTE)IgnoreApp, dwSize);
		// レジストリキーのクローズ
		RegCloseKey(hRegkey);

		return TRUE;
	}
	return FALSE;
}