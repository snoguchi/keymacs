/*===============================================================
  Keymacs.h -- Copyright(c) 1999 Shinsuke Noguchi <noguchi@i.am>
===============================================================*/
#include "dll/KeymacsDll.h"

#ifndef STRICT
	#define STRICT
#endif

#define WM_USER_TASKTRAY (WM_APP+100)
// void OnUserTasktray(HWND hwnd, UINT id)
#define HANDLE_WM_USER_TASKTRAY(hwnd, wParam, lParam, fn) \
    ((fn)((hwnd), (UINT)(lParam)), 0L)


BOOL InitApp(HINSTANCE);
BOOL InitInstance(HINSTANCE);
LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);


BOOL OnCreate(HWND, LPCREATESTRUCT);
void OnClose(HWND);
void OnDestroy(HWND);
void OnCommand(HWND, int, HWND, UINT);
void OnUserTasktray(HWND, UINT);


extern int (*pSetKeyMap)();
extern int (*pSetIgnoreApp)();
extern int (*pKeyHookSet)();
extern int (*pKeyHookRelease)();
extern int (*pIsKeyHooked)();


extern DWORD	fKeyMap;
extern char		IgnoreApp[IGNORE_MAX][IGNORE_NAMELEN];
