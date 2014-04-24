/*
** Tray Rating plugin for Winamp
** Ruzzz [ ruzzzua@gmail.com / http://ruzzz.com ]
*/

#pragma once

#ifndef WINVER				
#define WINVER 0x0501 //(Windows XP and Windows .NET Server)")	
#endif
#define _WIN32_IE 0x0500
//#ifndef _WIN32_IE			
//#define _WIN32_IE 0x0600	
//#endif

#include <windows.h>
#include <strsafe.h>

#include "sdk/wa_ipc.h"
#include "sdk/gen.h"

#include "resource.h"

BOOL Active = TRUE;

#define PLUGIN_NAME TEXT("Tray Rating")
#define PLUGIN_NAME_ANSI "Ruzzz Tray Rating v0.1"
#define PLUGIN_FILE TEXT("gen_trayrate")

// Настраиваем структуру для Winampa
winampGeneralPurposePlugin pluginData =
{
	GPPHDR_VER,
	PLUGIN_NAME_ANSI,
	0,
	0,
	0,
};

WNDPROC lpOldWinampWndProc; // Запоминаем старую оконную функцию Winamp'а

UINT msgTaskbarRestart = 0;

TCHAR winampDir[MAX_PATH];
TCHAR pluginIniFile[MAX_PATH];

//------------------------------
// TRAY
//------------------------------

// TODO: Брать все настройки из INI-файла, чтобы была возможность настраивать, в том числе и имена файлов иконок
#define RATE_COUNT 6 // 5 пунктов + "нет рейтинга"
HICON HTrayIcon[RATE_COUNT];

#define MENU_ICON_WIDTH					12
#define MENU_ICON_WIDTH_PADDING			2
#define MENU_ICON_FULL_WIDTH			(MENU_ICON_WIDTH + MENU_ICON_WIDTH_PADDING * 2)
#define MENU_ITEM_WIDTH_PADDING			5 // |<->MENU_STRING<->|

#define MENU_ICON_HEIGHT				12
#define MENU_ICON_HEIGHT_PADDING		4
#define MENU_ICON_FULL_HEIGHT			(MENU_ICON_HEIGHT + MENU_ICON_HEIGHT_PADDING * 2)

// Лучше использовать массив :), хоть иконки не связаны по смыслу, но так удобней
/*
IDI_ICON_STAR
IDI_ICON_STAR_H
IDI_ICON_STAR_S
IDI_ICON_STAR_S_H
IDI_ICON_STAR_B
IDI_ICON_STAR_B_H
*/
#define MENU_ICON_BACKGROUND_INDEX 4 // IDI_ICON_STAR_B
#define MENU_ICON_COUNT 6
HICON HMenuIcon[MENU_ICON_COUNT];

// TODO: Лучше использовать RegisterWindowMessage
#define WM_TRAY_MSG WM_USER + 5708
#define TRAY_ICON_ID 3023
BOOL trayIconVisible = FALSE;

//TCHAR TrackInfo[64] = {0,};

//------------------------------
// SETTINGS
//------------------------------

int CurrentTrackRate = 0;
wchar_t* CurrentTrackTitle = NULL;