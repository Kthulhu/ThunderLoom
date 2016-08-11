#include "PatternEditor.h"
#include "resource.h"
#include <stdlib.h>
#include <Commctrl.h>
#include "max.h"

const int entry_size=20;

typedef struct{
	wcWeaveParameters *params;
	unsigned char *bitmap;
	int current_yarn_type;
	int warp;
} EditorData;

typedef struct{
	POINT pos;
	int w,h;
}WindowSize;

WindowSize get_window_size(HWND hwnd)
{
	WindowSize ret;
	RECT rect;
	GetWindowRect(hwnd,&rect);
	ret.w=rect.right-rect.left;
	ret.h=rect.bottom-rect.top;
	ret.pos.x = rect.left;
	ret.pos.y = rect.top;
	return ret;
}

WindowSize get_canvas(HWND hWnd)
{
	WindowSize ret;
	HWND list_hwnd=GetDlgItem(hWnd,IDC_YARN_TYPE_LIST);
	WindowSize window_size=get_window_size(hWnd);
	WindowSize list_size=get_window_size(list_hwnd);
	ret.w=list_size.pos.x - window_size.pos.x-30;
	ret.h=window_size.h-50;
	ret.pos=window_size.pos;
	ret.pos.x+=10;
	ret.pos.y+=40;
	return ret;
}

void draw_pattern_entry(int x, int y, int w, int r, int g, int b, int warp,
	unsigned char *bitmap)
{
	for(int yy=0;yy<entry_size;yy++){
		for(int xx=0;xx<entry_size;xx++){
			int tmp_r=r;
			int tmp_g=g;
			int tmp_b=b;
			if((warp&&(xx==0||xx==entry_size-1)) || 
				(!warp &&(yy==0||yy==entry_size-1))){
				tmp_r=tmp_g=tmp_b=0;
			}
			int x_pos=x*entry_size+xx;
			int y_pos=y*entry_size+yy;
			unsigned char *p = bitmap + (x_pos+y_pos*w*entry_size)*4;
			p[0]=tmp_b; p[1]=tmp_g; p[2]=tmp_r;
		}
	}
}

void draw_pattern(unsigned char *bitmap, wcWeaveParameters *param)
{
	int w=param->pattern_width;
	int h=param->pattern_height;
	Pattern *pattern=param->pattern;
	for(int y=0;y<h;y++){
		for(int x=0;x<w;x++){
			PatternEntry pe=pattern->entries[x+y*w];
			YarnType yt=pattern->yarn_types[pe.yarn_type];
			if(!yt.color_enabled){
				yt=pattern->yarn_types[0];
			}
			unsigned char r = yt.color[0]*255.f;
			unsigned char g = yt.color[1]*255.f;
			unsigned char b = yt.color[2]*255.f;
			draw_pattern_entry(x,y,w,r,g,b,pe.warp_above,bitmap);
		}
	}
}

INT_PTR CALLBACK DlgProc(HWND hWnd,UINT message,WPARAM wParam,LPARAM lParam)
{
	UNREFERENCED_PARAMETER(lParam);
	switch(message){
		case WM_INITDIALOG:
		{
			EditorData *data=(EditorData*)lParam;
            SetWindowLongPtr(hWnd,GWLP_USERDATA,(LONG_PTR)data);
			HWND list_hwnd=GetDlgItem(hWnd,IDC_YARN_TYPE_LIST);
			Pattern *pattern=data->params->pattern;
			for(int i=pattern->num_yarn_types-1;i>0;i--){
				wchar_t buffer[256];
				swprintf(buffer,L"Yarn type %d",i);
				LVITEM item={0};
				item.mask=LVIF_PARAM|LVIF_TEXT;
				item.pszText=buffer;
				item.lParam=(LPARAM)i;
				ListView_InsertItem(list_hwnd,&item);
			}
			LVITEM item={0};
			item.mask=LVIF_PARAM|LVIF_TEXT;
			item.pszText=L"Keep yarn type";
			item.lParam=(LPARAM)0;
			ListView_InsertItem(list_hwnd,&item);

			HWND spinner_hwnd=GetDlgItem(hWnd,IDC_PATTERN_WIDTH_SPIN);
			HWND edit_hwnd   =GetDlgItem(hWnd,IDC_PATTERN_WIDTH_EDIT);
			ISpinnerControl * s=GetISpinner(spinner_hwnd);
			s->LinkToEdit(edit_hwnd,EDITTYPE_POS_INT);
			s->SetLimits(0.f,10000.f,FALSE);\
			s->SetScale(1.f);
			s->SetValue((int)data->params->pattern_width,FALSE);
			ReleaseISpinner(s);

			spinner_hwnd=GetDlgItem(hWnd,IDC_PATTERN_HEIGHT_SPIN);
			edit_hwnd   =GetDlgItem(hWnd,IDC_PATTERN_HEIGHT_EDIT);
			s=GetISpinner(spinner_hwnd);
			s->LinkToEdit(edit_hwnd,EDITTYPE_POS_INT);
			s->SetLimits(0.f,10000.f,FALSE);\
			s->SetScale(1.f);
			s->SetValue((int)data->params->pattern_height,FALSE);
			ReleaseISpinner(s);

			HWND button_hwnd=GetDlgItem(hWnd,IDC_WARP);
			ICustButton *button=GetICustButton(button_hwnd);
			button->SetText(L"Warp");
			button->SetType(CBT_CHECK);
			ReleaseICustButton(button);

			button_hwnd=GetDlgItem(hWnd,IDC_WEFT);
			button=GetICustButton(button_hwnd);
			button->SetText(L"Weft");
			button->SetType(CBT_CHECK);
			ReleaseICustButton(button);

			return (INT_PTR)TRUE;
		}

		case WM_PAINT:
		{
			EditorData *data =
				(EditorData*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
			wcWeaveParameters *param = data->params;
			Pattern *pattern=param->pattern;
			HDC     hDC;
			PAINTSTRUCT ps;
			hDC=BeginPaint(hWnd,&ps);
			WindowSize canvas=get_canvas(hWnd);

			int w=param->pattern_width*entry_size;
			int h=param->pattern_height*entry_size;

			BITMAPINFOHEADER bitmap_info_header = {
				sizeof(BITMAPINFO),
				w,h,1,32,BI_RGB,0,600,600,0,0
			};
			BITMAPINFO bitmap_info = {
				bitmap_info_header
			};
			ScreenToClient(hWnd,&canvas.pos);
			StretchDIBits(hDC,canvas.pos.x,canvas.pos.y,canvas.w,canvas.h,0,0,w,h,data->bitmap,
				&bitmap_info,DIB_RGB_COLORS,SRCCOPY);

			EndPaint(hWnd,&ps);
			break;
		}
		case WM_LBUTTONDOWN:
		{
			EditorData *data =
				(EditorData*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
			wcWeaveParameters *param = data->params;
			POINT p;
			GetCursorPos(&p);
			WindowSize canvas=get_canvas(hWnd);
			p.x-=canvas.pos.x;
			p.y-=canvas.pos.y;
			if(p.x > 0 && p.x<canvas.w && p.y > 0 && p.y<canvas.h){
				int x=((float)p.x/(float)canvas.w)*
					(float)param->pattern_width;
				int y=(1.f - (float)p.y/(float)canvas.h)*
					(float)param->pattern_height;
				if(data->current_yarn_type>0){
					param->pattern->entries[(x+y*param->pattern_width)].
						yarn_type=data->current_yarn_type;
				}
				if(data->warp>=0){
					param->pattern->entries[(x+y*param->pattern_width)].
						warp_above= data->warp;
				}
				draw_pattern(data->bitmap,param);
				RedrawWindow(hWnd,NULL,NULL,RDW_INVALIDATE);
			}
			break;
		}
		case WM_CLOSE:
		{
			EndDialog(hWnd,0);
			return (INT_PTR)TRUE;
		}
		case WM_COMMAND:
		{
			int id = LOWORD(wParam);
			HWND button_hwnd=(HWND)lParam;
			switch(HIWORD(wParam)){
				case BN_CLICKED:
				{
					EditorData *data =
						(EditorData*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
					switch(id){
						case IDC_WARP:
						case IDC_WEFT:
						{
							HWND warp_hwnd=GetDlgItem(hWnd,IDC_WARP);
							HWND weft_hwnd=GetDlgItem(hWnd,IDC_WEFT);
							data->warp=0;
							HWND other_hwnd=weft_hwnd;
							if(warp_hwnd==button_hwnd){
								data->warp=1;
								other_hwnd=warp_hwnd;
							}
							ICustButton *warp_button=GetICustButton(warp_hwnd);
							ICustButton *weft_button=GetICustButton(weft_hwnd);
							warp_button->SetCheck(data->warp == 1);
							weft_button->SetCheck(data->warp == 0);
							ReleaseICustButton(warp_button);
							ReleaseICustButton(weft_button);
							break;
						}
						case IDC_BUTTON_CLOSE:
							EndDialog(hWnd,0);
							return (INT_PTR)TRUE;
					}
				}
			}
			break;
		}
		case WM_NOTIFY:
		{
			switch(((LPNMHDR)lParam)->code){
				case NM_CLICK:
				{
					HWND list_hwnd=GetDlgItem(hWnd,IDC_YARN_TYPE_LIST);
					int index=ListView_GetNextItem(list_hwnd,-1,LVNI_SELECTED);
					EditorData *data =
						(EditorData*)GetWindowLongPtr(hWnd,GWLP_USERDATA);
					data->current_yarn_type=index;
					break;
				}
			}
			break;
		}
	}
	return (INT_PTR)FALSE;
}

void StartPatternEditor(HINSTANCE hinst, HWND parent_hwnd,
	wcWeaveParameters *param)
{
	EditorData data={};
	data.params=param;
	data.bitmap=(unsigned char*)calloc(param->pattern_height*param->pattern_width
		*entry_size*entry_size,4);
	data.current_yarn_type=1;
	draw_pattern(data.bitmap,param);
	DialogBoxParam(hinst, MAKEINTRESOURCE(IDD_PATTERN_EDITOR),parent_hwnd,
		DlgProc,(LPARAM)&data);
}
