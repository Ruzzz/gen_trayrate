/*
** Tray Rating plugin for Winamp
** Ruzzz [ ruzzzua@gmail.com / http://ruzzz.com ]
*/

#include "gen_trayrate.h"

// TODO: Нужны ли нам CharNext и CharPrev? Буду ли я использовать UTF-8?
TCHAR* LastChar(TCHAR* str, UINT len = 0)
{
	TCHAR* p;
	UINT i;

	p = str;	
	if (len)
		for (i = 1; (i < len) && (*p != TEXT('\0')); i++) p = CharNext(p);
	else
		while (*p != TEXT('\0')) p = CharNext(p); 
	
	if (p != str && *p == TEXT('\0'))
		p = CharPrev(str, p);
	return p;
}

void CutToChar(TCHAR* str, TCHAR delim, UINT len = 0)
{
	TCHAR* p;
	p = LastChar(str, len);
	while((p != str) && (*p != delim)) p = CharPrev(str, p);
	if (*p == delim) *p = TEXT('\0');	
}

void UpdateTray()
{
	NOTIFYICONDATA tnid = {0, };	
	tnid.cbSize = NOTIFYICONDATA_V2_SIZE; //sizeof(NOTIFYICONDATA);
	tnid.hWnd = pluginData.hwndParent;
	tnid.uID = TRAY_ICON_ID;

	if (Active) {		
		tnid.uFlags = NIF_ICON | NIF_TIP | NIF_MESSAGE;
		tnid.uCallbackMessage = WM_TRAY_MSG;		
		tnid.hIcon = HTrayIcon[CurrentTrackRate];

		// TODO: For Version 5.0 and later, szTip can have a maximum of 128 characters
		//StringCchCopy(tnid.szTip, 128, CurrentTrackTitle);
		// StringCchCopy приводит к аварийному закрытию Winamp'a при использовании IPC_GETPLAYLISTTITLEW
		lstrcpyn(tnid.szTip, CurrentTrackTitle, 128);

		Shell_NotifyIcon((trayIconVisible) ? NIM_MODIFY : NIM_ADD, &tnid);
		trayIconVisible = TRUE; // TODO: Убрать
	} else {	
		// Удаляем иконку в трее
		Shell_NotifyIcon(NIM_DELETE, &tnid);		
		trayIconVisible = FALSE;
	}
}

void SetActive(BOOL newState)//, BOOL updateTray = TRUE)
{
	//if (Active == newState) return;
	Active = newState;	
	//if (updateTray) 
	UpdateTray();
}

UINT idtTimer = 0;
SYSTEMTIME stLastOperation;

__int64 TimeInterval(SYSTEMTIME *t1, SYSTEMTIME *t2) // interval in msec.
{
	FILETIME ft1, ft2;
	__int64 nNanoSec1, nNanoSec2, res; // Кол-во сотен наносекунд

	SystemTimeToFileTime(t1, &ft1);	
	nNanoSec1 = (((__int64)ft1.dwHighDateTime) << 32) | ft1.dwLowDateTime;

	SystemTimeToFileTime(t2, &ft2);
	nNanoSec2 = (((__int64)ft2.dwHighDateTime) << 32) | ft2.dwLowDateTime;
	
	res = (nNanoSec2 - nNanoSec1) / 10000;

	if (res < 0) res = -res;
	return res;
}

void CALLBACK TimerProc(HWND hwnd, UINT message, UINT idTimer, DWORD dwTime)
{	
	SYSTEMTIME stNow;
	GetLocalTime(&stNow);
	if (TimeInterval(&stLastOperation, &stNow) < 100) return;

	KillTimer(0, idtTimer);
	idtTimer = 0;

	CurrentTrackRate = SendMessage(pluginData.hwndParent, WM_WA_IPC, 0, IPC_GETRATING);
	CurrentTrackTitle = (wchar_t*)SendMessage(pluginData.hwndParent, WM_WA_IPC, SendMessage(pluginData.hwndParent, WM_WA_IPC, 0, IPC_GETLISTPOS), IPC_GETPLAYLISTTITLEW);
	UpdateTray();
}

void UpdateTrayUseTimer()
{
	GetLocalTime(&stLastOperation);
	if (!idtTimer)
		idtTimer = SetTimer(0, 0, 100, (TIMERPROC)TimerProc);
}

// TODO: See it
// IPC_GET_PLAYING_FILENAME
// IPC_GETPLAYLISTFILE
// IPC_GETLISTPOS
// IPC_GETLISTLENGTH
// IPC_GET_BASIC_FILE_INFOW
// IPC_GET_EXTENDED_FILE_INFO

// IPC_GETPLAYLISTFILEW
// IPC_GETPLAYLISTTITLEW

// Внедряемся в оконную функцию Winamp'а
LRESULT CALLBACK HookWinampWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	int ret;

	if (Active) {
		switch (message) {  
			//case WM_WA_MPEG_EOF:
			case WM_TRAY_MSG: 
				//int listPos = SendMessage(hwnd_winamp, WM_WA_IPC, 0, IPC_GETLISTPOS);
				if ((wParam == TRAY_ICON_ID) && ((lParam == WM_LBUTTONUP) || (lParam == WM_RBUTTONUP))) {
					
					CurrentTrackRate = SendMessage(pluginData.hwndParent, WM_WA_IPC, 0, IPC_GETRATING);

					//HMENU hMenu = ::LoadMenu(pluginData.hDllInstance, MAKEINTRESOURCE(IDR_MENU_TRAY));
					HMENU hMenu = ::CreateMenu();
					if (!hMenu) {}; // TODO Error
					//HMENU hPopup = ::GetSubMenu(hMenu, 0);
					HMENU hPopup = ::CreatePopupMenu();
					if (!hPopup) {}; // TODO Error;

					AppendMenu(hMenu, MF_POPUP, (UINT)hPopup, NULL);					

					for (int i = RATE_COUNT - 1; i >= 0 ; i--)
						AppendMenu(hPopup, MF_OWNERDRAW, IDM_0 + i, NULL);
					
					//CheckMenuRadioItem(hPopup, IDM_0, IDM_5, IDM_0 + CurrentTrackRate, MF_BYCOMMAND); 

					// TODO: Может создавать свое окно? Иначе если Винамп не минимизирован, то не приятно выскакивает :)
					SetForegroundWindow(hwnd);

					POINT pt;
					::GetCursorPos(&pt);
					BOOL bOK = ::TrackPopupMenu(hPopup, 0, pt.x, pt.y, 0, hwnd, NULL);

					::DestroyMenu(hMenu);
					return 0;
				}
				break;

			case WM_COMMAND: // TODO: Check HIWORD(wParam) ?
				switch (LOWORD(wParam)) {
					case IDM_0: 
					case IDM_1: 
					case IDM_2: 
					case IDM_3:
					case IDM_4: 
					case IDM_5: 
						SendMessage(pluginData.hwndParent, WM_WA_IPC, LOWORD(wParam) - IDM_0, IPC_SETRATING);
						
						// Bug, при воспроизведении или паузе, если изменяем рейтинг IPC_SETRATING, то "stars-control" не меняется
						if (SendMessage(pluginData.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING))
							SendMessage(pluginData.hwndParent, WM_WA_IPC, IPC_CB_MISC_TITLE_RATING, IPC_CB_MISC);
						return 0;
						break;
				} // switch (LOWORD(wParam))
				break;

			case WM_WA_IPC:
				switch (lParam) {
					case IPC_SETRATING: 
						CurrentTrackRate = wParam;
						UpdateTray();
						break;

					case IPC_CB_MISC:
						if (wParam == IPC_CB_MISC_TITLE_RATING || 
							wParam == IPC_CB_MISC_TITLE || wParam == IPC_CB_MISC_STATUS) {

							// Bug, при воспроизведении или паузе, если изменяем рейтинг из контекстного меню списка воспроизведения
							//   то вместо IPC_CB_MISC_TITLE_RATING Winamp отсылает IPC_CB_MISC_TITLE
							if (wParam == IPC_CB_MISC_TITLE && SendMessage(pluginData.hwndParent, WM_WA_IPC, 0, IPC_ISPLAYING))
								SendMessage(pluginData.hwndParent, WM_WA_IPC, IPC_CB_MISC_TITLE_RATING, IPC_CB_MISC);
							else
								UpdateTrayUseTimer();
						}
						break;

					// При добавлении нескольких сот треков в список, винамп сходит с ума, поэтому делаем через таймер :)
					case IPC_PLAYLIST_MODIFIED:
						UpdateTrayUseTimer();
						break;

				} // switch (lParam)
				break;

			case WM_MEASUREITEM: 
				{
					LPMEASUREITEMSTRUCT lpmis = (LPMEASUREITEMSTRUCT) lParam;
					if (lpmis->CtlType == ODT_MENU) {
						lpmis->itemHeight = MENU_ICON_FULL_HEIGHT;
						lpmis->itemWidth = MENU_ICON_FULL_WIDTH * (RATE_COUNT - 1) + MENU_ITEM_WIDTH_PADDING * 2 - GetSystemMetrics(SM_CXMENUCHECK);						
						return TRUE;
					};
				}
				break;

			case WM_DRAWITEM: 
				{
					LPDRAWITEMSTRUCT lpdis = (LPDRAWITEMSTRUCT) lParam;
					if (lpdis->CtlType == ODT_MENU) {
						// TODO: DoubleBuffered

						int i = lpdis->itemID - IDM_0;
						//int y = (lpdis->rcItem.top + lpdis->rcItem.bottom - MENU_ICON_HEIGHT) / 2;
						int y = lpdis->rcItem.top + MENU_ICON_HEIGHT_PADDING;

						HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255));
						//HBRUSH brush = GetSysColorBrush(COLOR_MENU) // COLOR_HIGHLIGHT, COLOR_MENUHILIGHT
						FillRect(lpdis->hDC, &lpdis->rcItem, brush); 
						
						int sel = (lpdis->itemState & ODS_SELECTED) ? 1 : 0;
						int iconIndex = (CurrentTrackRate == i) ? 2 : 0;
						//HICON iconMain = (lpdis->itemState & ODS_CHECKED) ? HStarSelected : HStar;
						
						int dx = MENU_ICON_WIDTH_PADDING + MENU_ITEM_WIDTH_PADDING;

						// TODO: Рисовать не ICO, а PNG-маску, с настраиваемым цветом
						for (int x = 0; x < RATE_COUNT - 1; x++)
							DrawIconEx(lpdis->hDC, lpdis->rcItem.left + x * MENU_ICON_FULL_WIDTH + dx, y, 
								HMenuIcon[sel + ((x >= i) ? MENU_ICON_BACKGROUND_INDEX : iconIndex)], 
								MENU_ICON_WIDTH, MENU_ICON_HEIGHT, 0, 0, DI_NORMAL);
						
						DeleteObject(brush);

						return TRUE;
					};
				}
				break;


		} // switch (message)		

	} // if (Active)
	
	if (IsWindowUnicode(hwnd))
		ret = CallWindowProcW(lpOldWinampWndProc, hwnd, message, wParam, lParam);
	else
		ret = CallWindowProcA(lpOldWinampWndProc, hwnd, message, wParam, lParam);

	// Востанавливаем после Винампа, чтобы его иконки и наша не менялись местами
	if (Active && message == msgTaskbarRestart) {
		trayIconVisible = FALSE;
		UpdateTray();
	};

	return ret;
}

void GetPluginIniFile()
{
	char winampIni[MAX_PATH] = {0,};
	TCHAR winampIniT[MAX_PATH] = {0,};
	
	// TODO: Check, for SendMessageW result is wchar_t? If not, why? :)
	StringCchCopyA(winampIni, MAX_PATH, (char*)SendMessage(pluginData.hwndParent, WM_WA_IPC, 0, IPC_GETINIFILE));
#ifdef UNICODE
	MultiByteToWideChar(CP_ACP, 0, winampIni, -1, winampIniT, MAX_PATH);
#else
	winampIniT = winampIni;
#endif
	CutToChar((TCHAR*)winampIniT, TEXT('\\'));
	StringCchPrintf(pluginIniFile, MAX_PATH, TEXT("%s\\Plugins\\%s.ini"), winampIniT, PLUGIN_FILE);
}

// Функция вызываемая Winamp'ом при инициализации плагина
int pluginInit()
{
	int i;

	msgTaskbarRestart = RegisterWindowMessage(TEXT("TaskbarCreated"));

	// Загружаем иконки
	for (i = 0; i < RATE_COUNT; i++)
		HTrayIcon[i] = LoadIcon(pluginData.hDllInstance, MAKEINTRESOURCE(IDI_ICON_STAR_0 + i));

	for (i = 0; i < MENU_ICON_COUNT; i++)
		HMenuIcon[i] = LoadIcon(pluginData.hDllInstance, MAKEINTRESOURCE(IDI_ICON_STAR + i));

	// Перехватываем сообщения Winamp'а
	// TODO: Переместить в pluginConfig (Activate/Disable plugin)
	lpOldWinampWndProc = (WNDPROC)SetWindowLong(pluginData.hwndParent, GWL_WNDPROC, (LONG)HookWinampWndProc);
	
	// Get Plugin Ini Full Filename
	GetPluginIniFile();

	SetActive(GetPrivateProfileInt(TEXT("Main"), TEXT("Active"), TRUE, pluginIniFile));

	return 0;
}

// Функция вызываемая Winamp'ом для отображения настроек плагина
void pluginConfig()
{
	TCHAR str[10];
	HWND hwndParent = (HWND)SendMessage(pluginData.hwndParent, WM_WA_IPC, 0, IPC_GETDIALOGBOXPARENT);

	if (Active) {
		if (MessageBox(hwndParent, TEXT("Disable plugin?"), PLUGIN_NAME, MB_ICONQUESTION | MB_YESNO) == IDYES)
			SetActive(FALSE);
	} else {
		if (MessageBox(hwndParent, TEXT("Activate plugin?"), PLUGIN_NAME, MB_ICONQUESTION | MB_YESNO) == IDYES)
			SetActive(TRUE);
	}

	StringCchPrintf(str, 10, TEXT("%d"), Active); // TODO: Заменить на нормальное num->str :)
	WritePrivateProfileString(TEXT("Main"), TEXT("Active"), str, pluginIniFile);
}

// Функция вызываемая Winamp'ом при отключении плагина
void pluginQuit()
{	
	SetActive(FALSE);

	// TODO: Переместить в pluginConfig (Activate/Disable plugin)
	if (IsWindowUnicode(pluginData.hwndParent))
		SetWindowLongPtrW(pluginData.hwndParent, GWL_WNDPROC, (LONG)lpOldWinampWndProc);
	else
		SetWindowLongPtrA(pluginData.hwndParent, GWL_WNDPROC, (LONG)lpOldWinampWndProc);

	for (int i = 0; i < RATE_COUNT; i++)
		 DestroyIcon(HTrayIcon[i]);

	for (int i = 0; i < MENU_ICON_COUNT; i++)
		 DestroyIcon(HMenuIcon[i]);
}

// Экспортируем для Winamp'а
#ifdef __cplusplus
extern "C" {
#endif

__declspec(dllexport) winampGeneralPurposePlugin * winampGetGeneralPurposePlugin()
{
	pluginData.config = pluginConfig;
	pluginData.init = pluginInit;
	pluginData.quit = pluginQuit;

	return &pluginData;
}

#ifdef __cplusplus
}
#endif 
