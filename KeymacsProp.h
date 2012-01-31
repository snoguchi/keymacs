/*===================================================================
  KeymacsProp.h -- Copyright(c) 1999 Shinsuke Noguchi <noguchi@i.am>
===================================================================*/
void MakeKeymacsProp(HWND);

LRESULT CALLBACK DlgKeymapProc(HWND, UINT, WPARAM, LPARAM);
BOOL DlgKeymapOnInitdialog( HWND , HWND , LPARAM );
void DlgKeymapOnCommand( HWND , int, HWND, UINT );
BOOL DlgKeymapOnNotify( HWND, int, NMHDR FAR* );

LRESULT CALLBACK DlgIgnoreProc(HWND, UINT, WPARAM, LPARAM);
BOOL DlgIgnoreOnInitdialog( HWND, HWND, LPARAM );
void DlgIgnoreOnCommand( HWND, int, HWND, UINT );
BOOL DlgIgnoreOnNotify( HWND, int, NMHDR FAR* );

void InsertDesktopItem( LPTSTR, LPTSTR, int );
void showDesktopList();
void InsertIgnoreItem( LPSTR, int );
void showIgnoreList();
