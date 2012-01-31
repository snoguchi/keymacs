/*==================================================================
  keymacsDll.h -- Copyright(c) 1999 Shinsuke Noguchi <noguchi@i.am>
==================================================================*/
#define DLLEXPORT	__declspec(dllexport)

//キーマップON/OFFのフラグ
#define KMF_UP		0x1
#define KMF_DOWN	0x2
#define KMF_LEFT	0x4
#define KMF_RIGHT	0x8
#define KMF_PGUP	0x10
#define KMF_PGDN	0x20
#define KMF_HOME	0x40
#define KMF_END		0x80
#define KMF_DEL		0x100
#define KMF_BKSP	0x200
#define KMF_COPY	0x400
#define KMF_CUT		0x800
#define KMF_PASTE	0x1000
#define KMF_CUTEND	0x2000
#define KMF_UNDO	0x4000
#define KMF_ENTER	0x8000
#define KMF_ESC		0x10000
#define KMF_MARKSET	0x20000
#define KMF_TAB		0x40000
#define KMF_SEARCH	0x80000

#define IGNORE_MAX		16	//無視できるアプリケーションの最大数
#define IGNORE_NAMELEN	32	//アプリケーションのクラス名をどこまで見るか

DLLEXPORT void SetKeyMap( UINT );
DLLEXPORT void SetIgnoreApp( LPTSTR );
DLLEXPORT BOOL KeyHookSet();
DLLEXPORT BOOL KeyHookRelease();
DLLEXPORT BOOL IsKeyHooked();
DLLEXPORT LRESULT CALLBACK KeyboardProc( int, WPARAM, LPARAM );
void KMEventCtrlUp( UINT );
void KMEventAltUp( UINT );
void KMEventAltUpCtrlDn( UINT );
HWND GetFocusRoot();