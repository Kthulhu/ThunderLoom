#include "PatternEditor.h"
#include "resource.h"

INT_PTR CALLBACK DlgProc(HWND hDlg,UINT message,WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch(message){
		case WM_INITDIALOG:
		{
			return (INT_PTR)TRUE;
		}

		case WM_PAINT:
		{
			HDC     hDC;
			PAINTSTRUCT ps;
			hDC=BeginPaint(hDlg,&ps);
			RECT rect={0,0,400,400};
			HBRUSH hBrush=CreateSolidBrush(RGB(20,40,90));
			FillRect(hDC,&rect,hBrush);

			EndPaint(hDlg,&ps);
			break;
		}

		case WM_CLOSE:
		{
			EndDialog(hDlg,0);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
		{
			if(LOWORD(wParam)==IDC_BUTTON_CLOSE){
				EndDialog(hDlg,0);
				return (INT_PTR)TRUE;
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}

void StartPatternEditor(HINSTANCE hinst, HWND parent_hwnd)
{
	DialogBox(hinst, MAKEINTRESOURCE(IDD_PATTERN_EDITOR),parent_hwnd,DlgProc);
}
