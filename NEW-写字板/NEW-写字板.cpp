// NEW-写字板.cpp : 定义应用程序的入口点。
//

#include "stdafx.h"
#include "NEW-写字板.h"
#include<windows.h>

#define BUFFER(x,y) *(pBuffer+x+cxBuffer*y)

LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
BOOL CALLBACK AboutDlg(HWND, UINT, WPARAM, LPARAM);
void DoCaption(HWND, TCHAR *);

static TCHAR szAppName[] = TEXT("写字板");

int WINAPI WinMain(HINSTANCE hInstance,HINSTANCE hPrevInstance,
				   PSTR lpCmdLine,int nCmdShow)
{
	MSG msg;
	HWND hWnd;
	WNDCLASS wndclass;

	wndclass.style = CS_HREDRAW | CS_VREDRAW;
	wndclass.lpfnWndProc = WndProc;
	wndclass.cbClsExtra = 0;
	wndclass.cbWndExtra = 0;
	wndclass.hInstance = hInstance;
	wndclass.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_NEW));
	wndclass.hCursor = LoadCursor(NULL, IDC_ARROW);
	wndclass.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
	wndclass.lpszClassName = szAppName;
	wndclass.lpszMenuName = MAKEINTRESOURCE(IDC_NEW);

	if (!RegisterClass(&wndclass))
	{
		MessageBox(NULL, TEXT("This Program requires Windows NT!"),
			szAppName, MB_ICONERROR);
		return 0;
	}

	hWnd = CreateWindow(szAppName, TEXT("写字板"), WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
		NULL, NULL, hInstance, NULL);

	ShowWindow(hWnd, nCmdShow);
	UpdateWindow(hWnd);
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int) msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	static TCHAR szFileName[MAX_PATH], szTitleName[MAX_PATH];
	static DWORD dwCharSet = DEFAULT_CHARSET;
	static int cxClient, cyClient,
		cxChar, cyChar, cxBuffer, cyBuffer,
		xPos, yPos;
	int x, y, i;

	static HINSTANCE hInst;
	TEXTMETRIC tm;
	PAINTSTRUCT ps;
	HDC hdc;

	static TCHAR * pBuffer = NULL;

	switch (message)
	{
	case WM_INPUTLANGCHANGE:
		dwCharSet = wParam;
		return 0;

	case WM_CREATE:
		hInst = ((LPCREATESTRUCT)lParam)->hInstance;
		hdc = GetDC(hWnd);
		SelectObject(hdc, CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
			dwCharSet, 0, 0, 0, FIXED_PITCH, NULL));
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cyChar = tm.tmHeight;
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
		ReleaseDC(hWnd,hdc);
		return 0;

	case WM_SIZE:
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);

		cxBuffer = max(1, cxClient / cxChar);
		cyBuffer = max(1, cyClient / cyChar);

		if (pBuffer != NULL)
			free(pBuffer);
		pBuffer = (TCHAR *)malloc(cxBuffer*cyBuffer*sizeof(TCHAR));

		for (y = 0; y < cyBuffer;y++)
		for (x = 0; x < cxBuffer; x++)
			BUFFER(x, y) = ' ';

		xPos = 0;
		yPos = 0;
		if (hWnd = GetFocus())
			SetCaretPos(xPos*cxChar, yPos*cyChar);
		InvalidateRect(hWnd, NULL, TRUE);
		return 0;

	case WM_SETFOCUS:
		CreateCaret(hWnd, NULL, 1, cyChar);
		SetCaretPos(xPos*cxChar, yPos*cyChar);
		SetCaretBlinkTime(500);
		ShowCaret(hWnd);
		return 0;

	case WM_KILLFOCUS:
		HideCaret(hWnd);
		DestroyCaret();
		return 0;

	case WM_KEYDOWN:
		switch (wParam)
		{
		case VK_HOME:
			xPos = 0;
			break;

		case VK_END:
			xPos = cxBuffer - 1;
			break;

		case VK_PRIOR:
			yPos = 0;
			break;

		case VK_NEXT:
			yPos = cyBuffer - 1;
			break;

		case VK_LEFT:
			xPos = max(xPos - 1, 0);
			break;

		case VK_RIGHT:
			xPos = min(xPos + 1, cxBuffer - 1);
			break;

		case VK_UP:
			yPos = max(yPos - 1., 0);
			break;

		case VK_DOWN:
			yPos = min(yPos + 1, cyBuffer - 1);
			break;

		case VK_DELETE:
			for (x = xPos; x < cxBuffer-1; x++)
				BUFFER(x, yPos) = BUFFER(x + 1, yPos);
			BUFFER(cxBuffer - 1, yPos) = ' ';			

			HideCaret(hWnd);
			hdc = GetDC(hWnd);
			SelectObject(hdc, CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
				dwCharSet, 0, 0, 0, FIXED_PITCH, NULL));
			TextOut(hdc, xPos*cxChar, yPos*cyChar, &BUFFER(xPos, yPos),
				cxBuffer*cyBuffer - (xPos + yPos*cxBuffer));
			DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));

			ReleaseDC(hWnd, hdc);
			ShowCaret(hWnd);

			break;
		}
		SetCaretPos(xPos*cxChar, yPos*cyChar);
		return 0;

	case WM_CHAR:
		for (i = 0; i < (int)LOWORD(lParam); i++)
		{
			switch (wParam)
			{
			case '\b':
				if (xPos>0)
				{
					xPos--;
					SendMessage(hWnd, WM_KEYDOWN, VK_DELETE, 1);
				}
				break;

			case '\t':
				do
				{
					SendMessage(hWnd, WM_CHAR, ' ', 1);
				} while (xPos % 8 != 0);
				break;

			case '\n':
				if (++yPos == cyBuffer)
					yPos = cyBuffer - 1;
				break;

			case '\r':
				xPos = 0;
				if (++yPos == cyBuffer)
					yPos = cyBuffer - 1;
				break;

			case '\x1B':
				for (y = 0; y < cyBuffer; y++)
				for (x = 0; x < cxBuffer; x++)
					BUFFER(x, y) = ' ';

				xPos = 0;
				yPos = 0;
				InvalidateRect(hWnd, NULL, FALSE);
				break;

			default:
				BUFFER(xPos, yPos) = (TCHAR)wParam;
				HideCaret(hWnd);
				hdc = GetDC(hWnd);
				SelectObject(hdc, CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
					dwCharSet, 0, 0, 0, FIXED_PITCH, NULL));
				TextOut(hdc, xPos*cxChar, yPos*cyChar, &BUFFER(xPos, yPos), 1);
				DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
				ReleaseDC(hWnd, hdc);
				ShowCaret(hWnd);

				if (++xPos == cxBuffer)
				{
					xPos = 0;
					if (++yPos == cyBuffer)
						yPos = cyBuffer - 1;
				}	
				
				break;
			}
		}
		SetCaretPos(xPos*cxChar, yPos*cyChar);
		return 0;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{		
		case IDM_ABOUT:
			DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, AboutDlg);
			break;

		case IDM_EXIT:
			DestroyWindow(hWnd);
			break;
		}
		return 0;

	case WM_PAINT:
		hdc = BeginPaint(hWnd, &ps);
		SelectObject(hdc, CreateFont(0, 0, 0, 0, 0, 0, 0, 0,
			dwCharSet, 0, 0, 0, FIXED_PITCH, NULL));
		for (y = 0; y < cyBuffer; y++)
			TextOut(hdc, 0, y*cyChar, &BUFFER(0, y), cxBuffer);
		DeleteObject(SelectObject(hdc, GetStockObject(SYSTEM_FONT)));
		EndPaint(hWnd, &ps);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		return 0;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}


void DoCaption(HWND hWnd, TCHAR * szTitleName)
{
	TCHAR szCaption[MAX_PATH];
	wsprintf(szCaption, TEXT("%s - %s"),
		szTitleName[0] ? szTitleName : TEXT("UNTITLED"), szAppName);
	SetWindowText(hWnd, szCaption);
}

BOOL CALLBACK AboutDlg(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message)
	{
	case WM_INITDIALOG:
		return TRUE;

	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
		case IDCANCEL:
			EndDialog(hDlg, 0);
			return TRUE;
		}
		break;
	}
	return FALSE;
}


