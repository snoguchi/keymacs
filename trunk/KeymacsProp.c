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

static HWND hLVDesktop, hLVIgnore;	// ���X�g�r���[�̃n���h��
static const DWORD KEYMAP[] =		// �`�F�b�N�{�b�N�X�̃��\�[�XID�Ɠ������Ԃł��邱��!!
{
	KMF_UP, KMF_DOWN, KMF_LEFT, KMF_RIGHT, KMF_PGUP, KMF_PGDN, KMF_HOME, KMF_END, 
	KMF_DEL, KMF_BKSP, KMF_CUT, KMF_COPY, KMF_PASTE, KMF_ENTER, KMF_UNDO, KMF_ESC, 
	KMF_CUTEND, KMF_MARKSET, KMF_TAB, KMF_SEARCH
};
static const int numKEYMAP = sizeof(KEYMAP)/sizeof(KEYMAP[0]);



/*===============================
  �v���p�e�B�V�[�g�̍쐬
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
    psh.pszCaption = "Keymacs - �ݒ�";

    (HWND)PropertySheet(&psh);
}



/*============================
  �L�[�}�b�v�_�C�A���O
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
 WM_INITDIALOG�̃n���h��
 ����: (hwnd), (HWND)(wParam), lParam
 1. �`�F�b�N�{�b�N�X�̏�����
======================================================*/
static BOOL DlgKeymapOnInitdialog( HWND hDlgWnd, HWND hWndFocus, LPARAM lParam )
{
	int i;

	for( i=0; i<numKEYMAP; i++ )
		SendMessage( GetDlgItem(hDlgWnd, IDC_CHECK1+i), BM_SETCHECK, (WPARAM)((fKeyMap & KEYMAP[i]) != 0), 0L );
	SetDlgItemText( hDlgWnd, IDC_EDIT_TEST, "�����Ńe�X�g�ł��܂�\r\n�������K�p�{�^���������Ă���łȂ���\r\n�ύX�͔��f����܂���" );
	return TRUE;
}

/*=========================================================================
 WM_COMMAND�̃n���h��
 ����: (hwnd), (int)(LOWORD(wParam)), (HWND)(lParam), (UINT)HIWORD(wParam)
 1.�{�^�����������Ƃ��̏���
 2.�v���p�e�B�V�[�g�ւ̒ʒm
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
 WM_NOTIFY�̃n���h��
 ����: (hwnd), (int)(wParam), (NMHDR FAR*)(lParam)
 1. �v���p�e�B�V�[�g�́uOK�v�u�K�p�v�������ꂽ�Ƃ��̏���
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
		// �ݒ�̔��f
		fKeyMap = 0;
		for( i=0; i<numKEYMAP; i++ )
			if(IsDlgButtonChecked(hDlgWnd, IDC_CHECK1+i) == BST_CHECKED) fKeyMap |= KEYMAP[i];
		(*pSetKeyMap)(fKeyMap);

		
		// ���W�X�g���̃I�[�v��
		if( RegCreateKeyEx(
			HKEY_CURRENT_USER, "Software\\Guchi\\Keymacs", 0, "", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,	NULL, &hRegkey, &dwDisposition) != ERROR_SUCCESS )
			MessageBox(HWND_DESKTOP, "���W�X�g���̃I�[�v���Ɏ��s���܂���", NULL, MB_ICONEXCLAMATION);
		// ���W�X�g���ɏ�������
		dwSize = sizeof(fKeyMap);
		RegSetValueEx( hRegkey, "keymap", 0, REG_DWORD, (LPBYTE)&fKeyMap, dwSize);
		// ���W�X�g���L�[�̃N���[�Y
		RegCloseKey(hRegkey);

		return TRUE;
	}
	return FALSE;
}











/*===========================
  �������X�g�_�C�A���O
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


// �u�N�����Ă���A�v���v���X�g�ɃA�C�e���}��
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


// �u�N�����Ă���A�v���v�̓ǂݍ��݂ƕ\��
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

// �u��������A�v���v���X�g�ɃA�C�e���}��
static void InsertIgnoreItem( LPTSTR lpClass, int index )
{
	LV_ITEM item;

	item.mask = LVIF_TEXT;
	item.pszText = lpClass;
	item.iItem = index;
	item.iSubItem = 0;
	ListView_InsertItem(hLVIgnore, &item);
}

// �u��������A�v���v���X�g�̕\��
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
 WM_INITDAIALOG�n���h��
 ���X�g�r���[�̏�����
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
	lvcol.pszText = "�N���X��";
	lvcol.iSubItem = 0;
	ListView_InsertColumn(hLVDesktop, 0, &lvcol);
	ListView_InsertColumn(hLVIgnore, 0, &lvcol);

	lvcol.cx = 125;
	lvcol.pszText = "�^�C�g��";
	lvcol.iSubItem = 1;
	ListView_InsertColumn(hLVDesktop, 1, &lvcol);

	//�s�I���ł���悤�ɂ���
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
 WM_COMMAND�n���h��
 �{�^�����������̏���
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
			InsertDesktopItem( ClassStr, "�s��", 0 );
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
 WM_NOTIFY�n���h��
 �v���p�e�B�V�[�g�̃{�^��������������
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
	// �K�p�{�^�������̏���
	case PSN_APPLY:
		// �ݒ�̔��f
		ZeroMemory( &IgnoreApp, sizeof(IgnoreApp) );
		for( i=ListView_GetNextItem(hLVIgnore, -1, LVNI_ALL);
			 i!=-1 && i<IGNORE_MAX;
			 i=ListView_GetNextItem(hLVIgnore, i, LVNI_ALL) )
		{
			ListView_GetItemText( hLVIgnore, i, 0, ClassStr, sizeof(ClassStr) );
			lstrcpy( IgnoreApp[i], ClassStr );
		}
		(*pSetIgnoreApp)(IgnoreApp);

		// ���W�X�g���̃I�[�v��
		if( RegCreateKeyEx(
			HKEY_CURRENT_USER, "Software\\Guchi\\Keymacs", 0, "", REG_OPTION_NON_VOLATILE,
			KEY_ALL_ACCESS,	NULL, &hRegkey, &dwDisposition) != ERROR_SUCCESS )
			MessageBox(HWND_DESKTOP, "���W�X�g���̃I�[�v���Ɏ��s���܂���", NULL, MB_ICONEXCLAMATION);
		// ���W�X�g���֏�������
		dwSize = sizeof(IgnoreApp);
		RegSetValueEx( hRegkey, "ignore", 0, REG_BINARY, (LPBYTE)IgnoreApp, dwSize);
		// ���W�X�g���L�[�̃N���[�Y
		RegCloseKey(hRegkey);

		return TRUE;
	}
	return FALSE;
}