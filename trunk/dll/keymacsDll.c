/*==================================================================
  keymacsDll.c -- Copyright(c) 1999 Shinsuke Noguchi <noguchi@i.am>
==================================================================*/
#include <windows.h>
#include "KeymacsDll.h"

#pragma data_seg(".sharedata")
HHOOK	hKeyHook = 0;
UINT	fKeyMap = 0;
char	IgnoreApp[IGNORE_MAX][IGNORE_NAMELEN] = {'\0'};
HWND	hPrevWnd = 0;
#pragma data_seg()

static HINSTANCE hDllInst = NULL;	// DLL�̃C���X�^���X�n���h��



/*=======================
  DLL Main 
=======================*/
BOOL APIENTRY DllMain( HANDLE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved )
{
	hDllInst = hModule;
    return TRUE;
}



/*=======================
  �L�[�t�b�N�֌W 
=======================*/
// �L�[�}�b�v��ݒ肷�邽�߂̊֐�
DLLEXPORT void SetKeyMap( UINT srcfKeyMap )
{
	fKeyMap = srcfKeyMap;
}

// �����A�v����ݒ肷�邽�߂̊֐�
DLLEXPORT void SetIgnoreApp( LPTSTR srcIgnoreApp )
{
	ZeroMemory( IgnoreApp, sizeof(IgnoreApp) );
	CopyMemory( IgnoreApp, srcIgnoreApp, sizeof(IgnoreApp) );
	hPrevWnd = 0;
}

// �L�[�t�b�N�̃Z�b�g
DLLEXPORT BOOL KeyHookSet()
{
	hKeyHook = SetWindowsHookEx( WH_KEYBOARD, (HOOKPROC)KeyboardProc, hDllInst, 0 );
	if(!hKeyHook) return FALSE;
	return TRUE;
}

// �L�[�t�b�N�̉���
DLLEXPORT BOOL KeyHookRelease()
{
	BOOL res;
	if( hKeyHook )
		res = UnhookWindowsHookEx( hKeyHook );
	hKeyHook = NULL;
	return res;
}

// ���݃L�[�t�b�N����Ă��邩�ǂ���
DLLEXPORT BOOL IsKeyHooked()
{
	return (BOOL)hKeyHook;
}

// �L�[�C�x���g�������Ɏ��s�����֐�
DLLEXPORT LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam)
{
	int i;
	BOOL fShift	= FALSE;
	BOOL fCtrl	= FALSE;
	BOOL fAlt	= FALSE;
	static BOOL fIgnoreKeyHook = FALSE;
	char ClassName[IGNORE_NAMELEN];
	static BYTE keystate[256];

	// ����͂��񑩂Ŗ���
	if( nCode < 0 || nCode == HC_NOREMOVE )
		return CallNextHookEx( hKeyHook, nCode, wParam, lParam );


	if( GetKeyState(VK_SHIFT) < 0 )		fShift = TRUE;
	if( GetKeyState(VK_CONTROL) < 0 )	fCtrl = TRUE;
	if( (lParam & 0x20000000) != 0 )	fAlt = TRUE;


	// �����A�v���`�F�b�N(�t�H�[�J�X���ς�����Ƃ�����)
	if( GetFocus()!=hPrevWnd && (lParam & 0x80000000)==0 )
	{
		fIgnoreKeyHook = FALSE;
		GetClassName(GetFocusRoot(), ClassName, sizeof(ClassName));
		for(i=0; i<IGNORE_MAX; i++)
		{
			if( lstrcmp(ClassName, IgnoreApp[i])==0 )
			{
				fIgnoreKeyHook = TRUE;
				break;
			}
		}
		hPrevWnd = GetFocus();
	}


	// �����A�v���Ȃ�....
	if( fIgnoreKeyHook )
		return CallNextHookEx( hKeyHook, nCode, wParam, lParam );


	// WM_KEYUP�͖��� �� �����̂�?
	if( lParam & 0x80000000 )
		return CallNextHookEx( hKeyHook, nCode, wParam, lParam );

	
	// ���̐�C�c�[�X�g���[�N�L�[�ȊO�͖���
	if( !(fShift | fCtrl | fAlt) )
		return CallNextHookEx( hKeyHook, nCode, wParam, lParam );


	// ��������L�[�R�[�h�̓���ւ�
	switch( wParam )
	{

	case 0x50:	//Ctrl+P -> Up
		if( fCtrl && (fKeyMap & KMF_UP) != 0 )
		{
			KMEventCtrlUp( 0x26 );
			return TRUE;
		}
		else
			break;


	case 0x4E: //Ctrl+N -> Down
		if( fCtrl && (fKeyMap & KMF_DOWN) != 0 )
		{
			KMEventCtrlUp( 0x28 );
			return TRUE;
		}
		else
			break;


	case 0x42: //Ctrl+B -> Left
		if( fCtrl && (fKeyMap & KMF_LEFT) != 0 )
		{
			KMEventCtrlUp( 0x25 );
			return TRUE;
		}
		else
			break;


	case 0x46:	//Ctrl+F -> Right
		if( fCtrl && (fKeyMap & KMF_RIGHT) != 0 )
		{
			KMEventCtrlUp( 0x27 );
			return TRUE;
		}
		else
			break;


	case 0x56:	//Alt+V -> PgUp, Ctrl+V -> PgDn
		if( fAlt && (fKeyMap & KMF_PGUP) != 0 )
		{
			KMEventAltUp( 0x21 );
			return TRUE;
		}
		else if( fCtrl && (fKeyMap & KMF_PGDN) != 0 )
		{
			KMEventCtrlUp( 0x22 );
			return TRUE;
		}
		else
			break;


	case 0x41:	//Ctrl+A -> Home
		if( fCtrl && (fKeyMap & KMF_HOME)  != 0 )
		{
			KMEventCtrlUp( 0x24 );
			return TRUE;
		}
		else
			break;


	case 0x45:	//Ctrl+E -> End
		if( fCtrl && (fKeyMap & KMF_END) != 0 )
		{
			KMEventCtrlUp( 0x23 );
			return TRUE;
		}
		else
			break;


	case 0x44: //Ctrl+D -> Del
		if( fCtrl && (fKeyMap & KMF_DEL) != 0 )
		{
			KMEventCtrlUp( 0x2E );
			return TRUE;
		}
		else
			break;


	case 0x48: //Ctrl+H -> BkSp
		if( fCtrl && (fKeyMap & KMF_BKSP) != 0 )
		{
			KMEventCtrlUp( 0x08 );
			return TRUE;
		}
		else
			break;


	case 0x57:	//Ctrl+W -> Ctrl+X,   Alt+W -> Ctrl+C
		if( fCtrl && (fKeyMap & KMF_CUT) != 0 )
		{
			PostMessage( GetFocus(), WM_KEYDOWN, 0x58, 0);
			if( fShift ) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			return TRUE;
		}
		else if( fAlt && (fKeyMap & KMF_COPY) != 0 )
		{
			KMEventAltUpCtrlDn( 0x43 );
			if( fShift ) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			return TRUE;
		}
		else
			break;


	case 0x59: //Ctrl+Y -> Ctrl+V
		if( fCtrl && (fKeyMap & KMF_PASTE) != 0 )
		{
			if( fShift ) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			PostMessage( GetFocus(), WM_KEYDOWN, 0x56, 0 );
			return TRUE;
		}
		else
			break;


	case 0xBF: //Ctrl+/ -> Ctrl+Z
		if( fCtrl && (fKeyMap & KMF_UNDO) != 0 )
		{
			PostMessage( GetFocus(), WM_KEYDOWN, 0x5A, 0 );
			if( fShift ) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			return TRUE;
		}
		else
			break;


	case 0x4D: //Ctrl+M -> Enter
		if( fCtrl && (fKeyMap & KMF_ENTER) != 0 )
		{
			KMEventCtrlUp( 0x0D );
			return TRUE;
		}
		else
			break;


	case 0x47: //Ctrl+G -> Esc
		if( fCtrl && (fKeyMap & KMF_ESC) != 0 )
		{
			KMEventCtrlUp( 0x1B );
			if( fShift ) keybd_event(VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			return TRUE;
		}
		else
			break;


	case 0x49: //Ctrl+I -> TAB
		if( fCtrl && (fKeyMap & KMF_TAB) != 0 )
		{
			KMEventCtrlUp( 0x09 );
			return TRUE;
		}
		else
			break;


	case 0x53: //Ctrl+S -> Ctrl+F
		if( fCtrl && (fKeyMap & KMF_SEARCH) != 0 )
		{
			PostMessage( GetFocus(), WM_KEYDOWN, 0x46, 0 );
			return TRUE;
		}
		else
			break;


	case 0x4B: //Ctrl+K -> Shift+End, Ctrl+X
		if( fCtrl && (fKeyMap & KMF_CUTEND) != 0 )
		{
			keybd_event( VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
			keybd_event( VK_SHIFT, 0, 0, 0);
			keybd_event( 0x23, 0, 0, 0);
			keybd_event( 0x23, 0, KEYEVENTF_KEYUP, 0);
			keybd_event( VK_SHIFT, 0, KEYEVENTF_KEYUP, 0);
			keybd_event( VK_CONTROL, 0, 0, 0);
			keybd_event( 0x58, 0, 0, 0);
			keybd_event( 0x58, 0, KEYEVENTF_KEYUP, 0);
			return TRUE;
		}
		else 
			break;

	case 0x20:	//Ctrl+SPACE -> Shift ON/OFF
		if( fCtrl && (fKeyMap & KMF_MARKSET) != 0 )
		{
			keybd_event( VK_SHIFT, 0, ( GetKeyState(VK_SHIFT)<0 ? KEYEVENTF_KEYUP : 0), 0);
			return TRUE;
		}
		else
			break;



	default:
		break;
	}

	return CallNextHookEx( hKeyHook, nCode, wParam, lParam );
}




/*===========================
  ����������L�[�C�x���g�Q
===========================*/
// Ctrl�L�[��
static void KMEventCtrlUp( UINT keycode )
{
	BYTE	ks[256];

	GetKeyboardState( (PBYTE)ks );
	ks[ VK_CONTROL ] = 0;
	SetKeyboardState( (PBYTE)ks );

	keybd_event( VK_CONTROL, 0, KEYEVENTF_KEYUP, 0);
	keybd_event( (BYTE)keycode, 0, 0, 0);
	keybd_event( (BYTE)keycode, 0, KEYEVENTF_KEYUP, 0);
	keybd_event( VK_CONTROL, 0, 0, 0);

	GetKeyboardState( (PBYTE)ks );
	ks[ VK_CONTROL ] = 1;
	SetKeyboardState( (PBYTE)ks );
}

// Alt�L�[��
static void KMEventAltUp( UINT keycode )
{
	keybd_event( 0x12, 0, KEYEVENTF_KEYUP, 0 );
	keybd_event( (BYTE)keycode, 0, 0, 0 );
	keybd_event( (BYTE)keycode, 0, KEYEVENTF_KEYUP, 0);
	keybd_event( 0x12, 0, 0, 0);
}

// Alt�L�[��, Ctrl�L�[��
static void KMEventAltUpCtrlDn( UINT keycode )
{
	BYTE	ks[256];

	GetKeyboardState( (PBYTE)ks );
	ks[ VK_CONTROL ] = 1;
	ks[ VK_MENU ] = 0;
	SetKeyboardState( (PBYTE)ks );

	keybd_event( VK_MENU, 0, KEYEVENTF_KEYUP, KF_EXTENDED );
	keybd_event( VK_CONTROL, 0, 0, 0 );
	keybd_event( (BYTE)keycode, 0, 0, 0 );
	keybd_event( (BYTE)keycode, 0, KEYEVENTF_KEYUP, 0 );
	keybd_event( VK_CONTROL, 0, KEYEVENTF_KEYUP, 0 );

	GetKeyboardState( (PBYTE)ks );
	ks[ VK_CONTROL ] = 0;
	ks[ VK_MENU ] = 1;
	SetKeyboardState( (PBYTE)ks );
}

/*=========================
  ���̑� 
=========================*/
// �t�H�[�J�X�̂���E�C���h�E�̍ŏ�ʃE�C���h�E�̃n���h����Ԃ�
static HWND GetFocusRoot()
{
	HWND hWnd, hRootWnd;
	for( hWnd=GetFocus(); hWnd ;hWnd=GetParent(hWnd) )
		hRootWnd = hWnd;
	return hRootWnd;
}