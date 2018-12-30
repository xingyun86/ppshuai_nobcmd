#pragma once

#include "resource.h"

#include <atlconv.h>

class CWindowConsole {

#define T_KEY_CONSOLE_WND _T("KEY_CONSOLE_WND")
#define T_KEY_CONSOLE_TID _T("KEY_CONSOLE_TID")
#define T_KEY_CONSOLE_PID _T("KEY_CONSOLE_PID")

public:
	__inline static VOID SendASCII(WCHAR wData, BOOL bShift)
	{
		INPUT input[2] = { 0 };
		memset(input, 0, sizeof(input));

		if (bShift)
		{
			input[0].type = INPUT_KEYBOARD;
			input[0].ki.wVk = VK_SHIFT;
			SendInput(1, input, sizeof(INPUT));
		}

		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = wData;

		input[1].type = INPUT_KEYBOARD;
		input[1].ki.wVk = wData;
		input[1].ki.dwFlags = KEYEVENTF_KEYUP;

		SendInput(2, input, sizeof(INPUT));

		if (bShift)
		{
			input[0].type = INPUT_KEYBOARD;
			input[0].ki.wVk = VK_SHIFT;
			input[0].ki.dwFlags = KEYEVENTF_KEYUP;
			SendInput(1, input, sizeof(INPUT));
		}
	}
	__inline static VOID SendUNICODE(WCHAR wData)
	{
		INPUT input[2] = { 0 };
		memset(input, 0, sizeof(input));

		input[0].type = INPUT_KEYBOARD;
		input[0].ki.wVk = 0;
		input[0].ki.wScan = wData;
		input[0].ki.dwFlags = KEYEVENTF_UNICODE;

		input[1].type = INPUT_KEYBOARD;
		input[1].ki.wVk = 0;
		input[1].ki.wScan = wData;
		input[1].ki.dwFlags = KEYEVENTF_KEYUP | KEYEVENTF_UNICODE;

		SendInput(2, input, sizeof(INPUT));
	}
	__inline static VOID SendInputString(LPTSTR lpszInputString)
	{
		SHORT sVK = 0x00;
		BOOL bShift = FALSE;
		WCHAR * pwData = NULL;
		size_t nDataSize = 0L;

		USES_CONVERSION;

		pwData = T2W(lpszInputString);
		nDataSize = wcslen(pwData);

		for (size_t i = 0; i < nDataSize; i++)
		{
			//ASCII字符
			if (pwData[i] >= 0x00 && pwData[i] <= 0xFF) 
			{
				sVK = VkKeyScanW(pwData[i]);

				if (sVK == (-1))
				{
					SendUNICODE(pwData[i]);
				}
				else
				{
					if (sVK < 0)
					{
						sVK = ~sVK + 0x1;
					}

					bShift = (sVK >> 8) & 0x1;

					if (GetKeyState(VK_CAPITAL) & 0x1)
					{
						if ((pwData[i] >= L'a' && pwData[i] <= L'z') || (pwData[i] >= L'A' && pwData[i] <= L'Z'))
						{
							bShift = !bShift;
						}
					}

					SendASCII(sVK & 0xFF, bShift);
				}
			}
			//UNICODE字符
			else 
			{
				SendUNICODE(pwData[i]);
			}
		}
	}

	__inline static HWND NewWindowConsole(HWND hParentWnd, RECT * prcSpace = NULL)
	{
		HWND hWnd = NULL;
		BOOL bResult = FALSE;
		DWORD dwThreadId = 0L;
		DWORD dwProcessId = 0L;
		STARTUPINFO si = { 0 };
		PROCESS_INFORMATION pi = { 0 };
		_TCHAR T_CMD_NAME[MAX_PATH] = { 0 };
		_TCHAR T_CMD_LINE[MAX_PATH] = { 0 };
		_TCHAR T_CMD_PATH[MAX_PATH] = { 0 };
		RECT rcSpace = { 0 };

		_sntprintf(T_CMD_NAME, sizeof(T_CMD_NAME) / sizeof(*T_CMD_NAME), _T("00A%XA00"), ::GetModuleHandle(NULL) + ::GetCurrentProcessId() + ::GetCurrentThreadId());

		if (!((hWnd = GetWindowHwnd(hParentWnd)) &&
			(dwThreadId = ::GetWindowThreadProcessId(hWnd, &dwProcessId)) &&
			(dwProcessId == GetWindowProcessId(hParentWnd) && dwThreadId == GetWindowThreadId(hParentWnd))))
		{
			si.cb = sizeof(si);
			si.wShowWindow = SW_HIDE;
			si.dwFlags = STARTF_USESHOWWINDOW;
			::GetSystemDirectory(T_CMD_PATH, sizeof(T_CMD_PATH) / sizeof(*T_CMD_PATH));
			::lstrcat(T_CMD_PATH, _T("\\CMD.EXE"));
			_sntprintf(T_CMD_LINE, sizeof(T_CMD_LINE) / sizeof(*T_CMD_LINE), _T(" /K TITLE %s"), T_CMD_NAME);
			
			bResult = ::CreateProcess(T_CMD_PATH, T_CMD_LINE, NULL, NULL, FALSE, CREATE_NEW_CONSOLE, NULL, NULL, &si, &pi);
			if (bResult)
			{
				// Close the handles.
				CloseHandle(pi.hProcess);
				CloseHandle(pi.hThread);

				while (pi.dwProcessId && pi.dwThreadId)
				{
					if ((hWnd = ::FindWindowEx(NULL, NULL, _T("ConsoleWindowClass"), T_CMD_NAME)) &&
						(dwThreadId = ::GetWindowThreadProcessId(hWnd, &dwProcessId)) &&
						(dwProcessId == pi.dwProcessId && dwThreadId == pi.dwThreadId))
					{
						if (prcSpace)
						{
							memcpy(&rcSpace, prcSpace, sizeof(rcSpace));
						}
						SetWindowParams(hParentWnd, hWnd, dwThreadId, dwProcessId, prcSpace);
						break;
					}
					::Sleep(WAIT_TIMEOUT);
				}
			}
		}

		return hWnd;
	}
	/////////////////////////////////////////////////////////////////////////////////////////////
	//函数功能：设置圆角窗口
	//函数参数：
	//	hWnd		要设置的窗口句柄
	//	pstEllipse	要设置圆角的横向半径和纵向半径
	//	prcExcepted	要排除圆角的左上右下侧大小
	//返回值：无返回
	__inline static void SetWindowEllispeFrame(HWND hWnd, SIZE * pszEllipse = 0, RECT * prcExcepted = 0, BOOL bErase = TRUE)
	{
		HRGN hRgnWindow = 0;
		POINT ptPosition = { 0, 0 };
		RECT rcWindow = { 0, 0, 0, 0 };

		::GetWindowRect(hWnd, &rcWindow);
		if (prcExcepted)
		{
			ptPosition.x = prcExcepted->left;
			ptPosition.y = prcExcepted->top;
			rcWindow.left += prcExcepted->left;
			rcWindow.top += prcExcepted->top;
			rcWindow.right -= prcExcepted->right;
			rcWindow.bottom -= prcExcepted->bottom;
		}

		hRgnWindow = ::CreateRoundRectRgn(ptPosition.x, ptPosition.y, \
			rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, pszEllipse->cx, pszEllipse->cy);
		if (hRgnWindow)
		{
			::SetWindowRgn(hWnd, hRgnWindow, bErase);
		}
	}
	__inline static BOOL SetWindowParams(HWND hParentWnd, HWND hWnd, DWORD dwThreadId, DWORD dwProcessId, RECT * prcSpace)
	{
		RECT rect = { 0 };
		SIZE size = { 0 };
		::SetParent(hWnd, hParentWnd);
		::SetWindowLongPtr(hWnd, GWL_STYLE, ::GetWindowLongPtr(hWnd, GWL_STYLE) & (~WS_CAPTION) & (~WS_POPUPWINDOW) & (~WS_THICKFRAME) & (~WS_HSCROLL) & (~WS_VSCROLL));
		if (GetLastError() != ERROR_SUCCESS)
		{
			::SetRect(&rect,
				::GetSystemMetrics(SM_CXFRAME),
				::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CYCAPTION),
				::GetSystemMetrics(SM_CXFRAME) + ::GetSystemMetrics(SM_CXVSCROLL),
				::GetSystemMetrics(SM_CYFRAME) + ::GetSystemMetrics(SM_CYHSCROLL));
			SetWindowEllispeFrame(hWnd, &size, &rect);
		}
		::GetClientRect(hParentWnd, &rect);
		::MoveWindow(hWnd, rect.left + prcSpace->left, rect.top + prcSpace->top, rect.right - rect.left - prcSpace->left - prcSpace->right, rect.bottom - rect.top - prcSpace->top - prcSpace->bottom, FALSE);
		::RedrawWindow(hParentWnd, &rect, NULL, RDW_ALLCHILDREN | RDW_UPDATENOW | RDW_INVALIDATE);
		::SetWindowPos(hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_SHOWWINDOW);
		::ShowWindow(hWnd, SW_SHOW);
		::SetForegroundWindow(hWnd);
		return (::SetProp(hParentWnd, T_KEY_CONSOLE_WND, reinterpret_cast<HANDLE>(hWnd)) &
			::SetProp(hParentWnd, T_KEY_CONSOLE_TID, reinterpret_cast<HANDLE>(dwThreadId)) &
			::SetProp(hParentWnd, T_KEY_CONSOLE_PID, reinterpret_cast<HANDLE>(dwProcessId)));
	}
	__inline static HWND GetWindowHwnd(HWND hParentWnd) 
	{ 
		return reinterpret_cast<HWND>(::GetProp(hParentWnd, T_KEY_CONSOLE_WND));
	};
	__inline static DWORD GetWindowThreadId(HWND hParentWnd) 
	{ 
		return reinterpret_cast<DWORD>(::GetProp(hParentWnd, T_KEY_CONSOLE_TID));
	};
	__inline static DWORD GetWindowProcessId(HWND hParentWnd)
	{
		return reinterpret_cast<DWORD>(::GetProp(hParentWnd, T_KEY_CONSOLE_PID));
	};
	__inline static BOOL SetForegroundWindow(HWND hParentWnd)
	{
		return ::SetForegroundWindow((HWND)::GetProp(hParentWnd, T_KEY_CONSOLE_WND));
	};
	__inline static BOOL EndConsoleWindow(HWND hParentWnd)
	{
		return ::TerminateProcess(::OpenProcess(PROCESS_TERMINATE, FALSE, GetWindowProcessId(hParentWnd)), 0);
	}

	__inline static VOID SendConsoleString(HWND hParentWnd, LPTSTR lpszInputString)
	{
		if (::GetWindowThreadProcessId(GetWindowHwnd(hParentWnd), NULL) == GetWindowThreadId(hParentWnd))
		{
			::SetForegroundWindow(GetWindowHwnd(hParentWnd));
			SendInputString(lpszInputString);
		}
	}
};