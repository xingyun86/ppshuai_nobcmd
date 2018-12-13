// NobCmd.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "NobCmd.h"

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
	_In_opt_ HINSTANCE hPrevInstance,
	_In_ LPTSTR    lpCmdLine,
	_In_ int       nCmdShow)
{
	UNREFERENCED_PARAMETER(hPrevInstance);
	UNREFERENCED_PARAMETER(lpCmdLine);

	// TODO: Place code here.
	
	typedef HWND(WINAPI *PFN_GETCONSOLEWINDOW)();
	HMODULE hModule = GetModuleHandle(_T("KERNEL32"));
	PFN_GETCONSOLEWINDOW GetConsoleWindow = (PFN_GETCONSOLEWINDOW)GetProcAddress(hModule, ("GetConsoleWindow"));
	HWND hWndCmd = GetConsoleWindow();
	
	return DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDLGBOX), NULL,
		(DLGPROC)[](HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)->INT_PTR {
		UNREFERENCED_PARAMETER(lParam);
		RECT rcSpace = { 0L };
		::SetRect(&rcSpace, 4, 62, 6, 6);
		switch (uMsg)
		{
		case WM_INITDIALOG:
		{
			CWindowConsole::NewWindowConsole(hWnd, &rcSpace);
			CWindowConsole::SendConsoleString(hWnd, _T("\n"));
			::SetConsoleCP(CP_UTF8);
		}
		return (INT_PTR)TRUE;
		/*case WM_MOVE:
		case WM_MOVING:
		case WM_SIZE:
		case WM_SIZING:
		case WM_ENTERSIZEMOVE:
		case WM_EXITSIZEMOVE:
		{
			::ShowWindow(CWindowConsole::GetWindowHwnd(hWnd), SW_SHOW);
		}
		break;*/
		case WM_COMMAND:
		{
			switch (LOWORD(wParam))
			{
			case IDC_BTN_SENDMSG:
			{
				CWindowConsole::SendConsoleString(hWnd, _T("DIR\n"));
			}
			break;
			case IDOK:
			{
				CWindowConsole::NewWindowConsole(hWnd, &rcSpace);
				::SetConsoleCP(936);
			}
			break;
			case IDCANCEL:
			{
				CWindowConsole::EndConsoleWindow(hWnd);
				EndDialog(hWnd, LOWORD(wParam));
				return (INT_PTR)TRUE;
			}
			break;
			default:
			{

			}
			break;
			}
		}
		break;
		default:
		{
			
		}
		break;
		}
		return (INT_PTR)FALSE;
	});
}