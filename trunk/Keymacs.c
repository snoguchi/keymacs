/*==================================================================
  Keymacs.c -- Copyright(c) 1999 Shinsuke Noguchi <noguchi@i.am>
==================================================================*/
#include <windows.h>
#include <windowsx.h>
#include <winuser.h>
#include "resource.h"
#include "Keymacs.h"
#include "KeymacsProp.h"
#include "dll/KeymacsDll.h"

int (*pSetKeyMap)() = 0;
int (*pSetIgnoreApp)() = 0;
int (*pKeyHookSet)() = 0;
int (*pKeyHookRelease)() = 0;
int (*pIsKeyHooked)() = 0;

DWORD	fKeyMap = 0;
char	IgnoreApp[IGNORE_MAX][IGNORE_NAMELEN]={'\0'};

static const char pszAppTitle[]="Keymacs";	// アプリケーションクラスの名前
static NOTIFYICONDATA nIcon;				// タスクトレイのアイコンデータ
static HICON hIcon, hOnIcon, hOffIcon;		// アイコンデータ
static HMENU hMenu;							// タスクトレイ右クリック時のメニュー
static HMODULE hDLL;						// DLLのハンドル


/*=====================================
  WinMain
=====================================*/
int WINAPI WinMain(HINSTANCE hIns, HINSTANCE hPrevIns, LPSTR lpszArgv,int nDefaultWindowMode)
{
	static char pszMutexObjectName[]="Keymacs";
	static HANDLE hMutex;
	MSG message;

	// 重複起動防止
	hMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, pszMutexObjectName);
	if(hMutex)
	{
		CloseHandle(hMutex);
		return FALSE;
	}
	hMutex = CreateMutex(FALSE,0,pszMutexObjectName);

    if (!InitApp(hIns))	return FALSE;
    if (!InitInstance(hIns)) return FALSE;

	while(GetMessage(&message,NULL,0,0))
	{
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	ReleaseMutex(hMutex);

	return message.wParam;
}

/*=====================================
  ウインドウクラスの初期化
=====================================*/
static BOOL InitApp(HINSTANCE hIns)
{
	WNDCLASSEX wcl;
	ZeroMemory(&wcl,sizeof(wcl));

	wcl.hInstance = hIns;
	wcl.lpszClassName = pszAppTitle;
	wcl.lpfnWndProc = WndProc;
	wcl.style=0;
	wcl.hIcon=hIcon;
	wcl.hIconSm=NULL;
	wcl.hCursor=LoadCursor(NULL,IDI_APPLICATION);
	wcl.lpszMenuName=NULL;
	wcl.cbClsExtra=wcl.cbWndExtra=0;
	wcl.hbrBackground =(HBRUSH)GetStockObject(LTGRAY_BRUSH);
	wcl.cbSize =sizeof(WNDCLASSEX);

    return (RegisterClassEx(&wcl));
}


/*=====================================
  ウインドウ生成
=====================================*/
static BOOL InitInstance(HINSTANCE hIns)
{
    HWND hWnd;

	hWnd = CreateWindowEx(
		WS_EX_TOOLWINDOW, pszAppTitle, pszAppTitle, WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, 300, 200,
		HWND_DESKTOP, NULL,	hIns, NULL
	);

    if (!hWnd) return FALSE;

    ShowWindow(hWnd, SW_HIDE);
    UpdateWindow(hWnd);

    return TRUE;
}

/*=====================================
  ウインドウプロシージャ
=====================================*/
static LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch(message)
	{
		HANDLE_MSG(hWnd, WM_CREATE, OnCreate);
		HANDLE_MSG(hWnd, WM_COMMAND, OnCommand);
		HANDLE_MSG(hWnd, WM_CLOSE, OnClose);
		HANDLE_MSG(hWnd, WM_DESTROY, OnDestroy);
		HANDLE_MSG(hWnd, WM_USER_TASKTRAY, OnUserTasktray);
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return FALSE;
}


/* ================ ここからWndProc用のメッセージハンドラ群 =============== */

/*======================================================
 WM_CREATEのハンドラ
 引数: (hwnd), (LPCREATESTRUCT)(lParam)
 0. アイコン，メニューのロード
 1. DLLのロード
 2. タスクトレイにアイコン登録
 3. レジストリデータのロード
 4. フック開始
======================================================*/
static BOOL OnCreate(HWND hWnd, LPCREATESTRUCT lpCreateStruct)
{
	HKEY	hRegkey;
	DWORD	dwDisposition;
	DWORD	dwByte;
	HINSTANCE hIns;

	hIns = (HINSTANCE)GetWindowLong( hWnd, GWL_HINSTANCE );

	//DLLのロード
	hDLL=LoadLibrary("Keymacs.dll");
	if(hDLL==NULL)
	{
		MessageBox(HWND_DESKTOP,"Keymacs.dllが有りません",NULL,MB_ICONEXCLAMATION);
		return FALSE;
	}
	pSetKeyMap		= (int (*)())GetProcAddress(hDLL, "SetKeyMap");
	pSetIgnoreApp	= (int (*)())GetProcAddress(hDLL, "SetIgnoreApp");
	pKeyHookSet		= (int (*)())GetProcAddress(hDLL, "KeyHookSet");
	pKeyHookRelease = (int (*)())GetProcAddress(hDLL, "KeyHookRelease");
	pIsKeyHooked	= (int (*)())GetProcAddress(hDLL, "IsKeyHooked");


	// レジストリのオープン
	if( RegCreateKeyEx(
		HKEY_CURRENT_USER, "Software\\Guchi\\Keymacs", 0, "", REG_OPTION_NON_VOLATILE,
		KEY_ALL_ACCESS,	NULL, &hRegkey, &dwDisposition) != ERROR_SUCCESS)
	{
		MessageBox(HWND_DESKTOP, "レジストリのオープンに失敗しました", NULL, MB_ICONEXCLAMATION);
		return FALSE;
	}

	// キーマップフラグを読み込み(ない場合は初期値の0)
	dwByte = sizeof(fKeyMap);
	RegQueryValueEx( hRegkey, "keymap", NULL, NULL, (LPBYTE)&fKeyMap, &dwByte );
	dwByte = sizeof(IgnoreApp);
	RegQueryValueEx( hRegkey, "ignore", NULL, NULL, (LPBYTE)IgnoreApp, &dwByte );

	// レジストリキーのクローズ
	RegCloseKey(hRegkey);

	// キーマップフラグ, 無視リストのセット
	(*pSetKeyMap)(fKeyMap);
	(*pSetIgnoreApp)(IgnoreApp);


	// アイコン，メニューのロード
	hIcon=LoadIcon(hIns,MAKEINTRESOURCE(IDI_KEYMACS));
	if(hIcon == NULL) return FALSE;
	hOnIcon = (HICON)LoadImage(hIns,MAKEINTRESOURCE(IDI_TASKTRAY_ON),IMAGE_ICON,16,16,0);
	if(hOnIcon == NULL) return FALSE;
	hOffIcon = (HICON)LoadImage(hIns,MAKEINTRESOURCE(IDI_TASKTRAY_OFF),IMAGE_ICON,16,16,0);
	if(hOffIcon == NULL) return FALSE;
	hMenu = GetSubMenu(LoadMenu(hIns,MAKEINTRESOURCE(IDR_TASKTRAY)), 0);
	if(hMenu == NULL) return FALSE;


	//タスクトレイにアイコン表示
	nIcon.cbSize=sizeof(NOTIFYICONDATA);
	nIcon.uID=1;
	nIcon.hWnd=hWnd;
	nIcon.uFlags=NIF_ICON | NIF_MESSAGE | NIF_ICON | NIF_TIP;
	nIcon.uCallbackMessage=WM_USER_TASKTRAY;
	nIcon.hIcon=hOnIcon;
	lstrcpy(nIcon.szTip, "Keymacs 動作中");
	if (!Shell_NotifyIcon(NIM_ADD,&nIcon)) return FALSE;
	
	//フック開始
	if ( !(*pKeyHookSet)() ) return FALSE;

	return TRUE;
}


/*======================================
 WM_DESTROYハンドラ
======================================*/
static void OnDestroy(HWND hWnd)
{
	PostQuitMessage(0);
}


/*=======================================
 WM_CLOSEハンドラ
  1. フック終了
  2. レジストリにデータ登録
  3. タスクトレイのアイコン削除
  4. DLLの解放
=======================================*/
static void OnClose(HWND hWnd)
{
	// フック終了
	(*pKeyHookRelease)();

	//タスクトレイのアイコン削除
	Shell_NotifyIcon(NIM_DELETE,&nIcon);
	
	//DLLの解放
	FreeLibrary(hDLL);

	DestroyWindow(hWnd);
}


/*=======================================================================
 WM_COMMANDハンドラ
 引数: hWnd, (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)
=======================================================================*/
static void OnCommand(HWND hWnd, int id, HWND hwndCtl, UINT codeNotify)
{
	switch(id)
	{
	case ID_MNU_EXIT:
		Shell_NotifyIcon(NIM_DELETE,&nIcon);
		SendMessage(hWnd, WM_CLOSE, 0, 0L);
		break;
	case ID_MNU_CONFIG:
		MakeKeymacsProp(hWnd);
		break;
	}
}

/*======================================
 WM_USER_TASKTRAYハンドラ
 引数: (hwnd), (UINT)(lParam)
======================================*/
static void OnUserTasktray(HWND hWnd, UINT id)
{
	POINT point;

	switch(id)
	{
	case WM_LBUTTONDOWN:		// タスクトレイのアイコンを左クリック
		if ((*pIsKeyHooked)())
		{
			nIcon.hIcon=hOffIcon;
			lstrcpy(nIcon.szTip,"Keymacs 停止中");
			(*pKeyHookRelease)();
		}
		else
		{
			nIcon.hIcon=hOnIcon;
			lstrcpy(nIcon.szTip, "Keymacs 動作中");
			(*pKeyHookSet)();
		}
		Shell_NotifyIcon(NIM_MODIFY ,&nIcon);
		break;

	case WM_RBUTTONDOWN:		// タスクトレイのアイコンを右クリック
		GetCursorPos(&point);
		SetForegroundWindow(hWnd);
		SetFocus(hWnd);
		TrackPopupMenu(
			hMenu, TPM_BOTTOMALIGN | TPM_LEFTALIGN | TPM_RIGHTBUTTON, 
			point.x, point.y, 0, hWnd, NULL );
		PostMessage(hWnd, WM_NULL, 0, 0);
		break;
	}
}