#include "SKR_DirectRenderer.h"
#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <wincodec.h>

unsigned char quitrequested = 0;
HINSTANCE hInstance2;
int nCmdShow2;
int realwidth;
int realheight;
HWND gamewindow;
MSG msg;
DWORD frameduration;
LARGE_INTEGER lasttime;
LARGE_INTEGER currenttime;
LARGE_INTEGER frequency;
POINT mouse;
float vw;
float vh;
unsigned char lastp[256] = { 0 };
unsigned char pressed[256] = { 0 };
unsigned char introplayed = 0;

ID2D1Factory* factory = NULL;
ID2D1HwndRenderTarget* rendertarget = NULL;
ID2D1SolidColorBrush* brush = NULL;
IDWriteFactory* textfactory = NULL;
IDWriteTextFormat* textformat = NULL;
D2D1_COLOR_F black = { 0,0,0,1 };
IWICImagingFactory* wicfactory = NULL;
IWICBitmapDecoder* bitdecoder = NULL;
IWICBitmapFrameDecode* frame = NULL;
IWICFormatConverter* converter = NULL;
IWICBitmapClipper* clipper = NULL;
IWICBitmapFlipRotator* flipper = NULL;
WICRect wrect;

SKR_WinRect* winrecthead = NULL;
SKR_WinRect* tmpwinrect = NULL;

SKR_WinBar* barhead = NULL;
SKR_WinBar* tmpbar = NULL;

SKR_WinGameText* gametexthead = NULL;
SKR_WinGameText* tmpgametext = NULL;

SKR_WinMenuText* menutexthead = NULL;
SKR_WinMenuText* tmpmenutext = NULL;

SKR_WinButton* buttonhead = NULL;
SKR_WinButton* tmpbutton = NULL;

SKR_WinGameImage* gameimagehead = NULL;
SKR_WinGameImage* tmpgameimage = NULL;

SKR_WinMenuImage* menuimagehead = NULL;
SKR_WinMenuImage* tmpmenuimage = NULL;

D2D1_RECT_F vposition;

SKR_Rect* focusrect = NULL;
float xcha = 0;
float ycha = 0;
float xchanged = 0;
float ychanged = 0;
float minx2, maxx2, miny2, maxy2;

SKR_WinGameImage* bastaki = NULL;
SKR_WinMenuImage* bastaki2 = NULL;
LARGE_INTEGER animation;

unsigned char buttonsinput = 0;

unsigned char laststatepre;

struct FollowRect {
	SKR_Rect* rect;
	unsigned int number;
	FollowRect* next;
};

FollowRect* followrecthead = NULL;

void addrecttofollowlist(SKR_Rect* rect) {
	FollowRect* tmp = followrecthead;
	while (tmp != NULL) {
		if (tmp->rect == rect) {
			tmp->number++;
			return;
		}
		tmp = tmp->next;
	}
	FollowRect* rectu = (FollowRect*)malloc(sizeof(FollowRect));
	rectu->rect = rect;
	rectu->number = 1;
	rectu->next = NULL;
	if (followrecthead == NULL) {
		followrecthead = rectu;
	}
	else {
		FollowRect* tmp = followrecthead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rectu;
	}
	return;
}

void deletefromfollowlist(SKR_Rect* rect) {
	FollowRect* tmp3 = followrecthead;
	while (tmp3 != NULL) {
		if (tmp3->rect == rect) {
			tmp3->number--;
			if (tmp3->number == 0) {
				if (followrecthead == tmp3) {
					followrecthead = followrecthead->next;
					free(tmp3);
					tmp3 = NULL;
					return;
				}
				FollowRect* tmp = followrecthead;
				FollowRect* tmp2 = NULL;
				while (tmp != NULL) {
					if (tmp == tmp3) {
						tmp2->next = tmp->next;
						free(tmp3);
						tmp3 = NULL;
						return;
					}
					tmp2 = tmp;
					tmp = tmp->next;
				}
			}
			return;
		}
		tmp3 = tmp3->next;
	}
	return;
}

void deleteallfollowlist(void) {
	FollowRect* tmp = followrecthead;
	FollowRect* tmp2 = NULL;
	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = tmp->next;
		free(tmp2);
		tmp2 = NULL;
	}
	followrecthead = NULL;
	return;
}

void movefollowlist(float x, float y) {
	FollowRect* tmp = followrecthead;
	while (tmp != NULL) {
		tmp->rect->x += x;
		tmp->rect->y += y;
		tmp = tmp->next;
	}
	return;
}

struct SKR_Texture {
	ID2D1Bitmap* bitmap;
};

struct SKR_WinRect {
	SKR_Rect* rposition;
	float width;
	float radiusx;
	float radiusy;
	D2D1_COLOR_F color;
	unsigned char* condition;
	SKR_WinRect* next;
};

struct SKR_WinGameImage {
	SKR_Rect* rposition;
	D2D1_RECT_F source;
	ID2D1Bitmap* bitmap;
	unsigned char* condition;
	float opacity;
	unsigned char played;
	int animation;
	unsigned int milliseconds;
	unsigned char copy = 0;
	LARGE_INTEGER time;
	SKR_WinGameImage* next;
};

struct SKR_WinGameText {
	SKR_Rect* rposition;
	D2D1_COLOR_F color;
	D2D1_POINT_2F point;
	IDWriteTextLayout* textlayout;
	unsigned char* condition;
	SKR_WinGameText* next;
};

struct SKR_WinBar {
	D2D1_RECT_F border;
	D2D1_RECT_F back;
	D2D1_RECT_F front;
	D2D1_COLOR_F bordercolor;
	D2D1_COLOR_F backcolor;
	D2D1_COLOR_F frontcolor;
	float width;
	float radius;
	unsigned char* condition;
	unsigned char direction;
	float* maxvalue;
	float* currentvalue;
	SKR_WinBar* next;
};

struct SKR_WinButton {
	D2D1_RECT_F border;
	D2D1_RECT_F back;
	D2D1_RECT_F front;
	D2D1_COLOR_F bordercolor;
	D2D1_COLOR_F backcolor;
	D2D1_COLOR_F frontcolor;
	float width;
	float radius;
	unsigned char* condition;
	unsigned char* clicked;
	SKR_WinMenuText* text;
	unsigned char oneshot;
	SKR_WinButton* next;
};

struct SKR_WinMenuImage {
	SKR_Rect* rposition;
	D2D1_RECT_F source;
	ID2D1Bitmap* bitmap;
	unsigned char* condition;
	float opacity;
	unsigned char played;
	int animation;
	unsigned int milliseconds;
	unsigned char copy = 0;
	LARGE_INTEGER time;
	SKR_WinMenuImage* next;
};

struct SKR_WinMenuText {
	SKR_Rect* rposition;
	D2D1_COLOR_F color;
	D2D1_POINT_2F point;
	IDWriteTextLayout* textlayout;
	unsigned char* condition;
	SKR_WinMenuText* next;
};

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	if (uMsg == WM_DESTROY || uMsg == WM_QUIT || uMsg == WM_CLOSE) {
		quitrequested = 1;
		return 0;
	}
	return DefWindowProcW(hwnd, uMsg, wParam, lParam);
}

void SKR_WinInit(float VirtualWidth, float VirtualHeight, unsigned long MaxFps, const unsigned short* WindowName, const char* CursorFile, const char* IconFile) {
	WNDCLASSA wc;
	memset(&wc, 0, sizeof(WNDCLASS));
	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance2;
	wc.lpszClassName = "gamewindow";
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	if (CursorFile == NULL) {
		wc.hCursor = LoadCursorW(hInstance2, IDC_ARROW);
		SetCursor(LoadCursorW(0, IDC_ARROW));
	}
	else {
		wc.hCursor = (HCURSOR)LoadImageA(hInstance2, CursorFile, IMAGE_CURSOR, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	}
	if (IconFile != NULL) {
		wc.hIcon = (HICON)LoadImageA(hInstance2, IconFile, IMAGE_ICON, 0, 0, LR_DEFAULTSIZE | LR_LOADFROMFILE);
	}
	RegisterClassA(&wc);
	realwidth = GetSystemMetricsForDpi(SM_CXSCREEN, GetDpiForSystem());
	realheight = GetSystemMetricsForDpi(SM_CYSCREEN, GetDpiForSystem());
	gamewindow = CreateWindowExA(WS_EX_APPWINDOW, wc.lpszClassName, (LPCSTR)WindowName, WS_VISIBLE | WS_POPUP, 0, 0, realwidth, realheight, NULL, NULL, hInstance2, NULL);
	ShowWindow(gamewindow, nCmdShow2);
	frameduration = 1000 / MaxFps;
	lasttime.QuadPart = 0;
	timeBeginPeriod(1);
	QueryPerformanceFrequency(&frequency);
	D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &factory);
	factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(gamewindow, D2D1::SizeU(realwidth, realheight)), &rendertarget);
	rendertarget->CreateSolidColorBrush(D2D1::ColorF(0.0f, 0.0f, 0.0f, 1.0f), &brush);
	DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&textfactory));
	CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_IWICImagingFactory, (LPVOID*)&wicfactory);
	for (int i = 0; i < 256; i++) {
		lastp[i] = 0;
	}
	vw = VirtualWidth;
	vh = VirtualHeight;
	return;
}

int WINAPI WinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ PSTR lpCmdLine, _In_ int nCmdShow) {
	hInstance2 = hInstance;
	nCmdShow2 = nCmdShow;
	return main();
}

void SKR_WinPollEvent(void) {
	PeekMessageA(&msg, gamewindow, 0, 0, PM_REMOVE);
	TranslateMessage(&msg);
	DispatchMessageA(&msg);
	for (xcha = 0; xcha < 256; xcha++) {
		if (GetAsyncKeyState((int)xcha) & 0x8000) {
			if (lastp[(int)xcha] == 0) {
				pressed[(int)xcha] = 1;
				lastp[(int)xcha] = 1;
			}
			else {
				pressed[(int)xcha] = 0;
			}
		}
		else {
			pressed[(int)xcha] = 0;
			lastp[(int)xcha] = 0;
		}
	}
	return;
}

void SKR_WinQuit(void) {
	SKR_DestroyAll();
	brush->Release();
	rendertarget->Release();
	factory->Release();
	textfactory->Release();
	wicfactory->Release();
	timeEndPeriod(1);
	DestroyWindow(gamewindow);
	return;
}

unsigned char SKR_isWinQuitRequested(void) {
	if (quitrequested == 1) {
		quitrequested = 0;
		return 1;
	}
	else {
		return 0;
	}
}

float SKR_WinMouseX(void) {
	GetCursorPos(&mouse);
	return (float)mouse.x;
}

float SKR_WinMouseY(void) {
	GetCursorPos(&mouse);
	return (float)mouse.y;
}

unsigned char SKR_WinKeyDown(unsigned char VirtualKeyCode) {
	return lastp[VirtualKeyCode];
}

unsigned char SKR_WinKeyPressed(unsigned char VirtualKeyCode) {
	return pressed[VirtualKeyCode];
}

void SKR_HandleGameAnimation(SKR_WinGameImage** tmp) {
	bastaki = *tmp;
	while ((*tmp) != NULL && (*tmp)->animation != -1) {
		if ((*tmp)->played == 1) {
			QueryPerformanceCounter(&animation);
			animation.QuadPart *= 1000;
			animation.QuadPart /= frequency.QuadPart;
			if ((animation.QuadPart - (*tmp)->time.QuadPart) >= (*tmp)->milliseconds) {
				(*tmp)->played = 0;
				if ((*tmp)->animation != 0) {
					(*tmp) = (*tmp)->next;
				}
				else {
					(*tmp) = bastaki;
				}
				(*tmp)->played = 1;
				(*tmp)->time.QuadPart = animation.QuadPart;
			}
			vposition.left = (*tmp)->rposition->x * (realwidth / vw);
			vposition.top = (*tmp)->rposition->y * (realheight / vh);
			vposition.right = vposition.left + ((*tmp)->rposition->w * (realwidth / vw));
			vposition.bottom = vposition.top + ((*tmp)->rposition->h * (realheight / vh));
			rendertarget->DrawBitmap((*tmp)->bitmap, vposition, (*tmp)->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, (*tmp)->source);
			while ((*tmp)->animation != 0) {
				(*tmp) = (*tmp)->next;
			}
			return;
		}
		(*tmp) = (*tmp)->next;
	}
	(*tmp) = bastaki;
	(*tmp)->played = 1;
	QueryPerformanceCounter(&((*tmp)->time));
	(*tmp)->time.QuadPart *= 1000;
	(*tmp)->time.QuadPart /= frequency.QuadPart;
	vposition.left = (*tmp)->rposition->x * (realwidth / vw);
	vposition.top = (*tmp)->rposition->y * (realheight / vh);
	vposition.right = vposition.left + ((*tmp)->rposition->w * (realwidth / vw));
	vposition.bottom = vposition.top + ((*tmp)->rposition->h * (realheight / vh));
	rendertarget->DrawBitmap((*tmp)->bitmap, vposition, (*tmp)->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, (*tmp)->source);
	while ((*tmp)->animation != 0) {
		(*tmp) = (*tmp)->next;
	}
	return;
}

void SKR_HandleMenuAnimation(SKR_WinMenuImage** tmp) {
	bastaki2 = *tmp;
	while ((*tmp) != NULL && (*tmp)->animation != -1) {
		if ((*tmp)->played == 1) {
			QueryPerformanceCounter(&animation);
			animation.QuadPart *= 1000;
			animation.QuadPart /= frequency.QuadPart;
			if ((animation.QuadPart - (*tmp)->time.QuadPart) >= (*tmp)->milliseconds) {
				(*tmp)->played = 0;
				if ((*tmp)->animation != 0) {
					(*tmp) = (*tmp)->next;
				}
				else {
					(*tmp) = bastaki2;
				}
				(*tmp)->played = 1;
				(*tmp)->time.QuadPart = animation.QuadPart;
			}
			vposition.left = (*tmp)->rposition->x * (realwidth / vw);
			vposition.top = (*tmp)->rposition->y * (realheight / vh);
			vposition.right = vposition.left + ((*tmp)->rposition->w * (realwidth / vw));
			vposition.bottom = vposition.top + ((*tmp)->rposition->h * (realheight / vh));
			rendertarget->DrawBitmap((*tmp)->bitmap, vposition, (*tmp)->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, (*tmp)->source);
			while ((*tmp)->animation != 0) {
				(*tmp) = (*tmp)->next;
			}
			return;
		}
		(*tmp) = (*tmp)->next;
	}
	(*tmp) = bastaki2;
	(*tmp)->played = 1;
	QueryPerformanceCounter(&((*tmp)->time));
	(*tmp)->time.QuadPart *= 1000;
	(*tmp)->time.QuadPart /= frequency.QuadPart;
	vposition.left = (*tmp)->rposition->x * (realwidth / vw);
	vposition.top = (*tmp)->rposition->y * (realheight / vh);
	vposition.right = vposition.left + ((*tmp)->rposition->w * (realwidth / vw));
	vposition.bottom = vposition.top + ((*tmp)->rposition->h * (realheight / vh));
	rendertarget->DrawBitmap((*tmp)->bitmap, vposition, (*tmp)->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, (*tmp)->source);
	while ((*tmp)->animation != 0) {
		(*tmp) = (*tmp)->next;
	}
	return;
}

void SKR_WinRenderAll(void) {
	if (focusrect != NULL) {
		xcha = ((vw - focusrect->w) / 2) - focusrect->x;
		ycha = ((vh - focusrect->h) / 2) - focusrect->y;
		if (xchanged + xcha > maxx2) {
			xcha = maxx2 - xchanged;
		}
		if (xchanged + xcha < minx2) {
			xcha = minx2 - xchanged;
		}
		if (ychanged + ycha > maxy2) {
			ycha = maxy2 - ychanged;
		}
		if (ychanged + ycha < miny2) {
			ycha = miny2 - ychanged;
		}
		movefollowlist(xcha, ycha);
		xchanged += xcha;
		ychanged += ycha;
	}
	SKR_WinPollEvent();
	if (!introplayed) {
		textfactory->CreateTextFormat(L"Georgia", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 100.0f * ((float)realwidth / 1920), L"tr-tr", &(textformat));
		textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		IDWriteTextLayout* textlayout;
		textfactory->CreateTextLayout(L"ÞÜKRÜ ÇÝRÝÞ ENGINE", wcslen(L"ÞÜKRÜ ÇÝRÝÞ ENGINE"), textformat, (float)realwidth, (float)realheight, &textlayout);
		textformat->Release();
		for (float i = 1; i >= 0; i = i - 0.01f) {
			SKR_WinPollEvent();
			rendertarget->BeginDraw();
			black.a = 1;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			rendertarget->Clear(black);
			black.a = 1;
			black.r = 1;
			black.g = 1;
			black.b = 1;
			brush->SetColor(black);
			rendertarget->DrawTextLayout(D2D1_POINT_2F{ 0,0 }, textlayout, brush);
			black.a = i;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			brush->SetColor(black);
			rendertarget->FillRectangle(D2D1_RECT_F{ 0,0,(float)realwidth,(float)realheight }, brush);
			rendertarget->EndDraw();
			QueryPerformanceCounter(&currenttime);
			currenttime.QuadPart *= 1000;
			currenttime.QuadPart /= frequency.QuadPart;
			if (15 > (currenttime.QuadPart - lasttime.QuadPart)) {
				Sleep(15 - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
			}
			QueryPerformanceCounter(&lasttime);
			lasttime.QuadPart *= 1000;
			lasttime.QuadPart /= frequency.QuadPart;
		}
		for (float i = 0; i <= 1; i = i + 0.01f) {
			SKR_WinPollEvent();
			rendertarget->BeginDraw();
			black.a = 1;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			rendertarget->Clear(black);
			black.a = 1;
			black.r = 1;
			black.g = 1;
			black.b = 1;
			brush->SetColor(black);
			rendertarget->DrawTextLayout(D2D1_POINT_2F{ 0,0 }, textlayout, brush);
			black.a = i;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			brush->SetColor(black);
			rendertarget->FillRectangle(D2D1_RECT_F{ 0,0,(float)realwidth,(float)realheight }, brush);
			rendertarget->EndDraw();
			QueryPerformanceCounter(&currenttime);
			currenttime.QuadPart *= 1000;
			currenttime.QuadPart /= frequency.QuadPart;
			if (15 > (currenttime.QuadPart - lasttime.QuadPart)) {
				Sleep(15 - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
			}
			QueryPerformanceCounter(&lasttime);
			lasttime.QuadPart *= 1000;
			lasttime.QuadPart /= frequency.QuadPart;
		}
		textlayout->Release();
		introplayed = 1;
		black.a = 1;
		black.r = 0;
		black.g = 0;
		black.b = 0;
	}
	rendertarget->BeginDraw();
	rendertarget->Clear(black);
	tmpwinrect = winrecthead;
	while (tmpwinrect != NULL) {
		if (tmpwinrect->condition == NULL || *(tmpwinrect->condition)) {
			vposition.left = tmpwinrect->rposition->x * (realwidth / vw) + (tmpwinrect->width / 2);
			vposition.top = tmpwinrect->rposition->y * (realheight / vh) + (tmpwinrect->width / 2);
			vposition.right = vposition.left + (tmpwinrect->rposition->w * (realwidth / vw)) - tmpwinrect->width;
			vposition.bottom = vposition.top + (tmpwinrect->rposition->h * (realheight / vh)) - tmpwinrect->width;
			brush->SetColor(tmpwinrect->color);
			if (tmpwinrect->radiusx != 0 || tmpwinrect->radiusy != 0) {
				if (tmpwinrect->width == 0) {
					rendertarget->FillRoundedRectangle(D2D1::RoundedRect(vposition, tmpwinrect->radiusx, tmpwinrect->radiusy), brush);
				}
				else {
					rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(vposition, tmpwinrect->radiusx, tmpwinrect->radiusy), brush, tmpwinrect->width);
				}
			}
			else {
				if (tmpwinrect->width == 0) {
					rendertarget->FillRectangle(vposition, brush);
				}
				else {
					rendertarget->DrawRectangle(vposition, brush, tmpwinrect->width);
				}
			}
		}
		tmpwinrect = tmpwinrect->next;
	}
	tmpgameimage = gameimagehead;
	while (tmpgameimage != NULL) {
		if (tmpgameimage->condition == NULL || *(tmpgameimage->condition)) {
			vposition.left = tmpgameimage->rposition->x * (realwidth / vw);
			vposition.top = tmpgameimage->rposition->y * (realheight / vh);
			vposition.right = vposition.left + (tmpgameimage->rposition->w * (realwidth / vw));
			vposition.bottom = vposition.top + (tmpgameimage->rposition->h * (realheight / vh));
			if (tmpgameimage->animation == -1) {
				rendertarget->DrawBitmap(tmpgameimage->bitmap, vposition, tmpgameimage->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, tmpgameimage->source);
			}
			else {
				SKR_HandleGameAnimation(&tmpgameimage);
			}
		}
		tmpgameimage = tmpgameimage->next;
	}
	tmpgametext = gametexthead;
	while (tmpgametext != NULL) {
		if (tmpgametext->condition == NULL || *(tmpgametext->condition)) {
			brush->SetColor(tmpgametext->color);
			tmpgametext->point.x = tmpgametext->rposition->x * (realwidth / vw);
			tmpgametext->point.y = tmpgametext->rposition->y * (realheight / vh);
			rendertarget->DrawTextLayout(tmpgametext->point, tmpgametext->textlayout, brush);
		}
		tmpgametext = tmpgametext->next;
	}
	tmpbar = barhead;
	while (tmpbar != NULL) {
		if (tmpbar->condition == NULL || *(tmpbar->condition)) {
			if (tmpbar->direction) {
				if (*(tmpbar->currentvalue) <= 0) {
					tmpbar->front.top = tmpbar->back.bottom;
				}
				else if (*(tmpbar->currentvalue) >= *(tmpbar->maxvalue)) {
					tmpbar->front.top = tmpbar->back.top;
				}
				else {
					tmpbar->front.top = tmpbar->back.bottom - ((tmpbar->back.bottom - tmpbar->back.top) * (*(tmpbar->currentvalue) / (*(tmpbar->maxvalue))));
				}
			}
			else {
				if (*(tmpbar->currentvalue) <= 0) {
					tmpbar->front.right = tmpbar->back.left;
				}
				else if (*(tmpbar->currentvalue) >= *(tmpbar->maxvalue)) {
					tmpbar->front.right = tmpbar->back.right;
				}
				else {
					tmpbar->front.right = tmpbar->back.left + ((tmpbar->back.right - tmpbar->back.left) * (*(tmpbar->currentvalue) / (*(tmpbar->maxvalue))));
				}
			}
			brush->SetColor(tmpbar->backcolor);
			rendertarget->FillRectangle(tmpbar->back, brush);
			brush->SetColor(tmpbar->frontcolor);
			rendertarget->FillRectangle(tmpbar->front, brush);
			brush->SetColor(tmpbar->bordercolor);
			rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(tmpbar->border, tmpbar->radius, tmpbar->radius), brush, tmpbar->width);
		}
		tmpbar = tmpbar->next;
	}
	tmpbutton = buttonhead;
	while (tmpbutton != NULL) {
		if (tmpbutton->condition == NULL || *(tmpbutton->condition)) {
			if (tmpbutton->oneshot) {
				*(tmpbutton->clicked) = 0;
			}
			buttonsinput = 0;
			if (tmpbutton->oneshot || !*(tmpbutton->clicked)) {
				if (SKR_WinMouseX() >= tmpbutton->border.left && SKR_WinMouseX() <= tmpbutton->border.right && SKR_WinMouseY() >= tmpbutton->border.top && SKR_WinMouseY() <= tmpbutton->border.bottom) {
					brush->SetColor(tmpbutton->frontcolor);
					rendertarget->FillRectangle(tmpbutton->front, brush);
					buttonsinput = 1;
				}
				else {
					brush->SetColor(tmpbutton->backcolor);
					rendertarget->FillRectangle(tmpbutton->back, brush);
				}
			}
			else {
				brush->SetColor(tmpbutton->frontcolor);
				rendertarget->FillRectangle(tmpbutton->front, brush);
			}
			if (SKR_WinKeyPressed(0x01) && buttonsinput) {
				*(tmpbutton->clicked) = 1;
			}
			brush->SetColor(tmpbutton->bordercolor);
			rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(tmpbutton->border, tmpbutton->radius, tmpbutton->radius), brush, tmpbutton->width);
		}
		tmpbutton = tmpbutton->next;
	}
	tmpmenuimage = menuimagehead;
	while (tmpmenuimage != NULL) {
		if (tmpmenuimage->condition == NULL || *(tmpmenuimage->condition)) {
			vposition.left = tmpmenuimage->rposition->x * (realwidth / vw);
			vposition.top = tmpmenuimage->rposition->y * (realheight / vh);
			vposition.right = vposition.left + (tmpmenuimage->rposition->w * (realwidth / vw));
			vposition.bottom = vposition.top + (tmpmenuimage->rposition->h * (realheight / vh));
			if (tmpmenuimage->animation == -1) {
				rendertarget->DrawBitmap(tmpmenuimage->bitmap, vposition, tmpmenuimage->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, tmpmenuimage->source);
			}
			else {
				SKR_HandleMenuAnimation(&tmpmenuimage);
			}
		}
		tmpmenuimage = tmpmenuimage->next;
	}
	tmpmenutext = menutexthead;
	while (tmpmenutext != NULL) {
		if (tmpmenutext->condition == NULL || *(tmpmenutext->condition)) {
			brush->SetColor(tmpmenutext->color);
			tmpmenutext->point.x = tmpmenutext->rposition->x * (realwidth / vw);
			tmpmenutext->point.y = tmpmenutext->rposition->y * (realheight / vh);
			rendertarget->DrawTextLayout(tmpmenutext->point, tmpmenutext->textlayout, brush);
		}
		tmpmenutext = tmpmenutext->next;
	}
	rendertarget->EndDraw();
	QueryPerformanceCounter(&currenttime);
	currenttime.QuadPart *= 1000;
	currenttime.QuadPart /= frequency.QuadPart;
	if (frameduration > (currenttime.QuadPart - lasttime.QuadPart)) {
		Sleep(frameduration - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
	}
	QueryPerformanceCounter(&lasttime);
	lasttime.QuadPart *= 1000;
	lasttime.QuadPart /= frequency.QuadPart;
	return;
}

void SKR_DestroyAll(void) {
	deleteallfollowlist();
	SKR_DestroyAllWinBars();
	SKR_DestroyAllWinRects();
	SKR_DestroyAllWinGameTexts();
	SKR_DestroyAllWinMenuTexts();
	SKR_DestroyAllWinButtons();
	SKR_DestroyAllWinGameImages();
	SKR_DestroyAllWinMenuImages();
}

SKR_WinRect* SKR_CreateWinRect(SKR_Rect* position, SKR_Color* color, float width, float radiusx, float radiusy, unsigned char* condition) {
	SKR_WinRect* rect = (SKR_WinRect*)malloc(sizeof(SKR_WinRect));
	rect->rposition = position;
	rect->color.a = color->a;
	rect->color.r = color->r;
	rect->color.g = color->g;
	rect->color.b = color->b;
	rect->width = width * (realheight / vh);
	rect->radiusx = radiusx * (realwidth / vw);
	rect->radiusy = radiusy * (realheight / vh);
	rect->condition = condition;
	addrecttofollowlist(rect->rposition);
	rect->next = NULL;
	if (winrecthead == NULL) {
		winrecthead = rect;
	}
	else {
		SKR_WinRect* tmp = winrecthead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_DestroyWinRect(SKR_WinRect* winrect) {
	if (winrect == NULL) {
		return;
	}
	if (winrecthead == winrect) {
		winrecthead = winrecthead->next;
		deletefromfollowlist(winrect->rposition);
		free(winrect);
		winrect = NULL;
		return;
	}
	SKR_WinRect* tmp = winrecthead;
	SKR_WinRect* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == winrect) {
			tmp2->next = tmp->next;
			deletefromfollowlist(winrect->rposition);
			free(winrect);
			winrect = NULL;
			return;
		}
		tmp2 = tmp;
		tmp = tmp->next;
	}
	return;
}

void SKR_DestroyAllWinRects(void) {
	SKR_WinRect* tmp = winrecthead;
	SKR_WinRect* tmp2 = NULL;
	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = tmp->next;
		deletefromfollowlist(tmp2->rposition);
		free(tmp2);
		tmp2 = NULL;
	}
	winrecthead = NULL;
	return;
}

SKR_WinBar* SKR_CreateWinBar(SKR_Rect* position, SKR_Color* bordercolor, SKR_Color* backcolor, SKR_Color* frontcolor, float* maxvalue, float* currentvalue, unsigned char* condition, unsigned char direction) {
	SKR_WinBar* rect = (SKR_WinBar*)malloc(sizeof(SKR_WinBar));
	rect->condition = condition;
	rect->maxvalue = maxvalue;
	rect->direction = direction;
	rect->currentvalue = currentvalue;
	rect->bordercolor.a = bordercolor->a;
	rect->bordercolor.r = bordercolor->r;
	rect->bordercolor.g = bordercolor->g;
	rect->bordercolor.b = bordercolor->b;
	rect->backcolor.a = backcolor->a;
	rect->backcolor.r = backcolor->r;
	rect->backcolor.g = backcolor->g;
	rect->backcolor.b = backcolor->b;
	rect->frontcolor.a = frontcolor->a;
	rect->frontcolor.r = frontcolor->r;
	rect->frontcolor.g = frontcolor->g;
	rect->frontcolor.b = frontcolor->b;
	if (position->w > position->h) {
		rect->width = (position->h / 7) * (realheight / vh);
	}
	else {
		rect->width = (position->w / 7) * (realwidth / vw);
	}
	rect->radius = rect->width;
	rect->border.left = (position->x * (realwidth / vw)) + (rect->width / 2);
	rect->border.right = rect->border.left + (position->w * (realwidth / vw)) - rect->width;
	rect->border.top = (position->y * (realheight / vh)) + (rect->width / 2);
	rect->border.bottom = rect->border.top + (position->h * (realheight / vh)) - rect->width;
	rect->back.left = rect->border.left + ((rect->border.right - rect->border.left) / 1000);
	rect->back.right = rect->border.right - ((rect->border.right - rect->border.left) / 1000);
	rect->back.top = rect->border.top + ((rect->border.bottom - rect->border.top) / 1000);
	rect->back.bottom = rect->border.bottom - ((rect->border.bottom - rect->border.top) / 1000);
	rect->front = rect->back;
	rect->next = NULL;
	if (barhead == NULL) {
		barhead = rect;
	}
	else {
		SKR_WinBar* tmp = barhead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_DestroyWinBar(SKR_WinBar* winbar) {
	if (winbar == NULL) {
		return;
	}
	if (barhead == winbar) {
		barhead = barhead->next;
		free(winbar);
		winbar = NULL;
		return;
	}
	SKR_WinBar* tmp = barhead;
	SKR_WinBar* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == winbar) {
			tmp2->next = tmp->next;
			free(winbar);
			winbar = NULL;
			return;
		}
		tmp2 = tmp;
		tmp = tmp->next;
	}
	return;
}

void SKR_DestroyAllWinBars(void) {
	SKR_WinBar* tmp = barhead;
	SKR_WinBar* tmp2 = NULL;
	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = tmp->next;
		free(tmp2);
		tmp2 = NULL;
	}
	barhead = NULL;
	return;
}

SKR_WinGameText* SKR_CreateWinGameText(SKR_Rect* position, SKR_Color* color, const wchar_t* string, const wchar_t* font, float size, unsigned char* condition) {
	SKR_WinGameText* rect = (SKR_WinGameText*)malloc(sizeof(SKR_WinGameText));
	rect->color.a = color->a;
	rect->color.r = color->r;
	rect->color.g = color->g;
	rect->color.b = color->b;
	rect->rposition = position;
	textfactory->CreateTextFormat(font, NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size * (realwidth / vw), L"tr-tr", &(textformat));
	textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	textfactory->CreateTextLayout(string, wcslen(string), textformat, position->w * (realwidth / vw), position->h * (realheight / vh), &(rect->textlayout));
	textformat->Release();
	rect->condition = condition;
	addrecttofollowlist(rect->rposition);
	rect->next = NULL;
	if (gametexthead == NULL) {
		gametexthead = rect;
	}
	else {
		SKR_WinGameText* tmp = gametexthead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_DestroyWinGameText(SKR_WinGameText* wingametext) {
	if (wingametext == NULL) {
		return;
	}
	if (gametexthead == wingametext) {
		gametexthead = gametexthead->next;
		wingametext->textlayout->Release();
		deletefromfollowlist(wingametext->rposition);
		free(wingametext);
		wingametext = NULL;
		return;
	}
	SKR_WinGameText* tmp = gametexthead;
	SKR_WinGameText* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == wingametext) {
			tmp2->next = tmp->next;
			wingametext->textlayout->Release();
			deletefromfollowlist(wingametext->rposition);
			free(wingametext);
			wingametext = NULL;
			return;
		}
		tmp2 = tmp;
		tmp = tmp->next;
	}
	return;
}

void SKR_DestroyAllWinGameTexts(void) {
	SKR_WinGameText* tmp = gametexthead;
	SKR_WinGameText* tmp2 = NULL;
	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = tmp->next;
		tmp2->textlayout->Release();
		deletefromfollowlist(tmp2->rposition);
		free(tmp2);
		tmp2 = NULL;
	}
	gametexthead = NULL;
	return;
}

SKR_WinMenuText* SKR_CreateWinMenuText(SKR_Rect* position, SKR_Color* color, const wchar_t* string, const wchar_t* font, float size, unsigned char* condition) {
	SKR_WinMenuText* rect = (SKR_WinMenuText*)malloc(sizeof(SKR_WinMenuText));
	rect->color.a = color->a;
	rect->color.r = color->r;
	rect->color.g = color->g;
	rect->color.b = color->b;
	rect->rposition = position;
	textfactory->CreateTextFormat(font, NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size * (realwidth / vw), L"tr-tr", &(textformat));
	textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	textfactory->CreateTextLayout(string, wcslen(string), textformat, position->w * (realwidth / vw), position->h * (realheight / vh), &(rect->textlayout));
	textformat->Release();
	rect->condition = condition;
	rect->next = NULL;
	if (menutexthead == NULL) {
		menutexthead = rect;
	}
	else {
		SKR_WinMenuText* tmp = menutexthead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_DestroyWinMenuText(SKR_WinMenuText* winmenutext) {
	if (winmenutext == NULL) {
		return;
	}
	if (menutexthead == winmenutext) {
		menutexthead = menutexthead->next;
		winmenutext->textlayout->Release();
		free(winmenutext);
		winmenutext = NULL;
		return;
	}
	SKR_WinMenuText* tmp = menutexthead;
	SKR_WinMenuText* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == winmenutext) {
			tmp2->next = tmp->next;
			winmenutext->textlayout->Release();
			free(winmenutext);
			winmenutext = NULL;
			return;
		}
		tmp2 = tmp;
		tmp = tmp->next;
	}
	return;
}

void SKR_DestroyAllWinMenuTexts(void) {
	SKR_WinMenuText* tmp = menutexthead;
	SKR_WinMenuText* tmp2 = NULL;
	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = tmp->next;
		tmp2->textlayout->Release();
		free(tmp2);
		tmp2 = NULL;
	}
	menutexthead = NULL;
	return;
}

SKR_WinButton* SKR_CreateWinButton(SKR_Rect* position, SKR_Color* bordercolor, SKR_Color* backcolor, SKR_Color* frontcolor, SKR_Color* textcolor, const wchar_t* string, const wchar_t* font, float size, unsigned char* clicked, unsigned char* condition, unsigned char oneshot) {
	SKR_WinButton* rect = (SKR_WinButton*)malloc(sizeof(SKR_WinButton));
	rect->bordercolor.a = bordercolor->a;
	rect->bordercolor.r = bordercolor->r;
	rect->bordercolor.g = bordercolor->g;
	rect->bordercolor.b = bordercolor->b;
	rect->backcolor.a = backcolor->a;
	rect->backcolor.r = backcolor->r;
	rect->backcolor.g = backcolor->g;
	rect->backcolor.b = backcolor->b;
	rect->frontcolor.a = frontcolor->a;
	rect->frontcolor.r = frontcolor->r;
	rect->frontcolor.g = frontcolor->g;
	rect->frontcolor.b = frontcolor->b;
	rect->clicked = clicked;
	rect->oneshot = oneshot;
	rect->condition = condition;
	if (position->w > position->h) {
		rect->width = (position->h / 7) * (realheight / vh);
	}
	else {
		rect->width = (position->w / 7) * (realwidth / vw);
	}
	rect->radius = rect->width;
	rect->border.left = (position->x * (realwidth / vw)) + (rect->width / 2);
	rect->border.right = rect->border.left + (position->w * (realwidth / vw)) - rect->width;
	rect->border.top = (position->y * (realheight / vh)) + (rect->width / 2);
	rect->border.bottom = rect->border.top + (position->h * (realheight / vh)) - rect->width;
	rect->back.left = rect->border.left + ((rect->border.right - rect->border.left) / 1000);
	rect->back.right = rect->border.right - ((rect->border.right - rect->border.left) / 1000);
	rect->back.top = rect->border.top + ((rect->border.bottom - rect->border.top) / 1000);
	rect->back.bottom = rect->border.bottom - ((rect->border.bottom - rect->border.top) / 1000);
	rect->front = rect->back;
	rect->text = SKR_CreateWinMenuText(position, textcolor, string, font, size, condition);
	rect->next = NULL;
	if (buttonhead == NULL) {
		buttonhead = rect;
	}
	else {
		SKR_WinButton* tmp = buttonhead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_DestroyWinButton(SKR_WinButton* winbutton) {
	if (winbutton == NULL) {
		return;
	}
	if (buttonhead == winbutton) {
		buttonhead = buttonhead->next;
		SKR_DestroyWinMenuText(winbutton->text);
		free(winbutton);
		winbutton = NULL;
		return;
	}
	SKR_WinButton* tmp = buttonhead;
	SKR_WinButton* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == winbutton) {
			tmp2->next = tmp->next;
			SKR_DestroyWinMenuText(winbutton->text);
			free(winbutton);
			winbutton = NULL;
			return;
		}
		tmp2 = tmp;
		tmp = tmp->next;
	}
	return;
}

void SKR_DestroyAllWinButtons(void) {
	SKR_WinButton* tmp = buttonhead;
	SKR_WinButton* tmp2 = NULL;
	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = tmp->next;
		SKR_DestroyWinMenuText(tmp2->text);
		free(tmp2);
		tmp2 = NULL;
	}
	buttonhead = NULL;
	return;
}

SKR_WinGameImage* SKR_CreateWinGameImage(SKR_Rect* dest, SKR_Rect* src, const wchar_t* file, unsigned char flip, unsigned char* condition, float opacity, int animation, unsigned int milliseconds) {
	SKR_WinGameImage* rect = (SKR_WinGameImage*)malloc(sizeof(SKR_WinGameImage));
	rect->condition = condition;
	rect->rposition = dest;
	rect->opacity = opacity;
	rect->animation = animation;
	rect->milliseconds = milliseconds;
	rect->played = 0;
	rect->time.QuadPart = 0;
	addrecttofollowlist(rect->rposition);
	rect->next = NULL;
	wicfactory->CreateDecoderFromFilename(file, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &bitdecoder);
	bitdecoder->GetFrame(0, &frame);
	wicfactory->CreateBitmapClipper(&clipper);
	wrect.X = (int)(src->x);
	wrect.Y = (int)(src->y);
	wrect.Width = (int)(src->w);
	wrect.Height = (int)(src->h);
	clipper->Initialize(frame, &wrect);
	if (flip != 0) {
		wicfactory->CreateBitmapFlipRotator(&flipper);
		if (flip == 1) {
			flipper->Initialize(clipper, WICBitmapTransformFlipHorizontal);
		}
		else if (flip == 2) {
			flipper->Initialize(clipper, WICBitmapTransformFlipVertical);
		}
	}
	wicfactory->CreateFormatConverter(&converter);
	if (flip != 0) {
		converter->Initialize(flipper, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
		flipper->Release();
	}
	else {
		converter->Initialize(clipper, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
	}
	rendertarget->CreateBitmapFromWicBitmap(converter, &(rect->bitmap));
	bitdecoder->Release();
	frame->Release();
	converter->Release();
	clipper->Release();
	rect->source.left = 0;
	rect->source.top = 0;
	rect->source.right = rect->bitmap->GetSize().width;
	rect->source.bottom = rect->bitmap->GetSize().height;
	if (gameimagehead == NULL) {
		gameimagehead = rect;
	}
	else {
		SKR_WinGameImage* tmp = gameimagehead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_DestroyWinGameImage(SKR_WinGameImage* image) {
	if (image == NULL) {
		return;
	}
	if (gameimagehead == image) {
		gameimagehead = gameimagehead->next;
		if (!image->copy) {
			image->bitmap->Release();
		}
		deletefromfollowlist(image->rposition);
		free(image);
		image = NULL;
		return;
	}
	SKR_WinGameImage* tmp = gameimagehead;
	SKR_WinGameImage* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == image || tmp->bitmap == NULL) {
			tmp2->next = tmp->next;
			if (!image->copy) {
				image->bitmap->Release();
				image->bitmap = NULL;
			}
			deletefromfollowlist(image->rposition);
			free(image);
			image = NULL;
		}
		tmp2 = tmp;
		tmp = tmp->next;
	}
	return;
}

void SKR_DestroyAllWinGameImages(void) {
	SKR_WinGameImage* tmp = gameimagehead;
	SKR_WinGameImage* tmp2 = NULL;
	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = tmp->next;
		if (!tmp2->copy) {
			tmp2->bitmap->Release();
		}
		deletefromfollowlist(tmp2->rposition);
		free(tmp2);
		tmp2 = NULL;
	}
	gameimagehead = NULL;
	return;
}

SKR_WinMenuImage* SKR_CreateWinMenuImage(SKR_Rect* dest, SKR_Rect* src, const wchar_t* file, unsigned char flip, unsigned char* condition, float opacity, int animation, unsigned int milliseconds) {
	SKR_WinMenuImage* rect = (SKR_WinMenuImage*)malloc(sizeof(SKR_WinMenuImage));
	rect->condition = condition;
	rect->rposition = dest;
	rect->opacity = opacity;
	rect->animation = animation;
	rect->milliseconds = milliseconds;
	rect->played = 0;
	rect->time.QuadPart = 0;
	rect->next = NULL;
	wicfactory->CreateDecoderFromFilename(file, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &bitdecoder);
	bitdecoder->GetFrame(0, &frame);
	wicfactory->CreateBitmapClipper(&clipper);
	wrect.X = (int)(src->x);
	wrect.Y = (int)(src->y);
	wrect.Width = (int)(src->w);
	wrect.Height = (int)(src->h);
	clipper->Initialize(frame, &wrect);
	if (flip != 0) {
		wicfactory->CreateBitmapFlipRotator(&flipper);
		if (flip == 1) {
			flipper->Initialize(clipper, WICBitmapTransformFlipHorizontal);
		}
		else if (flip == 2) {
			flipper->Initialize(clipper, WICBitmapTransformFlipVertical);
		}
	}
	wicfactory->CreateFormatConverter(&converter);
	if (flip != 0) {
		converter->Initialize(flipper, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
		flipper->Release();
	}
	else {
		converter->Initialize(clipper, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
	}
	rendertarget->CreateBitmapFromWicBitmap(converter, &(rect->bitmap));
	bitdecoder->Release();
	frame->Release();
	converter->Release();
	clipper->Release();
	rect->source.left = 0;
	rect->source.top = 0;
	rect->source.right = rect->bitmap->GetSize().width;
	rect->source.bottom = rect->bitmap->GetSize().height;
	if (menuimagehead == NULL) {
		menuimagehead = rect;
	}
	else {
		SKR_WinMenuImage* tmp = menuimagehead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_DestroyWinMenuImage(SKR_WinMenuImage* image) {
	if (image == NULL) {
		return;
	}
	if (menuimagehead == image) {
		menuimagehead = menuimagehead->next;
		if (!image->copy) {
			image->bitmap->Release();
		}
		free(image);
		image = NULL;
		return;
	}
	SKR_WinMenuImage* tmp = menuimagehead;
	SKR_WinMenuImage* tmp2 = NULL;
	while (tmp != NULL) {
		if (tmp == image || tmp->bitmap == NULL) {
			tmp2->next = tmp->next;
			if (!image->copy) {
				image->bitmap->Release();
				image->bitmap = NULL;
			}
			free(image);
			image = NULL;
		}
		tmp2 = tmp;
		tmp = tmp->next;
	}
	return;
}

void SKR_DestroyAllWinMenuImages(void) {
	SKR_WinMenuImage* tmp = menuimagehead;
	SKR_WinMenuImage* tmp2 = NULL;
	while (tmp != NULL) {
		tmp2 = tmp;
		tmp = tmp->next;
		if (!tmp2->copy) {
			tmp2->bitmap->Release();
		}
		free(tmp2);
		tmp2 = NULL;
	}
	menuimagehead = NULL;
	return;
}

void SKR_FadeIn(SKR_Color* color, float speed, const wchar_t* string, const wchar_t* font, float size, SKR_Rect* textrect, SKR_Color* textcolor) {
	IDWriteTextLayout* textlay = NULL;
	if (string != NULL && font != NULL) {
		textfactory->CreateTextFormat(font, NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size * ((float)realwidth / vw), L"tr-tr", &(textformat));
		textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		if (textrect == NULL) {
			textfactory->CreateTextLayout(string, wcslen(string), textformat, (float)realwidth, (float)realheight, &textlay);
		}
		else {
			textfactory->CreateTextLayout(string, wcslen(string), textformat, textrect->w, textrect->h, &textlay);
		}
		textformat->Release();
	}
	if (focusrect != NULL) {
		xcha = ((vw - focusrect->w) / 2) - focusrect->x;
		ycha = ((vh - focusrect->h) / 2) - focusrect->y;
		if (xchanged + xcha > maxx2) {
			xcha = maxx2 - xchanged;
		}
		if (xchanged + xcha < minx2) {
			xcha = minx2 - xchanged;
		}
		if (ychanged + ycha > maxy2) {
			ycha = maxy2 - ychanged;
		}
		if (ychanged + ycha < miny2) {
			ycha = miny2 - ychanged;
		}
		movefollowlist(xcha, ycha);
		xchanged += xcha;
		ychanged += ycha;
	}
	D2D1_COLOR_F fadecolor;
	if (color != NULL) {
		fadecolor = { color->r,color->g,color->b,1 };
	}
	else {
		fadecolor = { 0,0,0,1 };
	}
	D2D1_RECT_F screen = { 0,0,(float)realwidth,(float)realheight };
	SKR_WinPollEvent();
	if (!introplayed) {
		textfactory->CreateTextFormat(L"Georgia", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 100.0f * ((float)realwidth / 1920), L"tr-tr", &(textformat));
		textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		IDWriteTextLayout* textlayout;
		textfactory->CreateTextLayout(L"ÞÜKRÜ ÇÝRÝÞ ENGINE", wcslen(L"ÞÜKRÜ ÇÝRÝÞ ENGINE"), textformat, (float)realwidth, (float)realheight, &textlayout);
		textformat->Release();
		for (float i = 1; i >= 0; i = i - 0.01f) {
			SKR_WinPollEvent();
			rendertarget->BeginDraw();
			black.a = 1;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			rendertarget->Clear(black);
			black.a = 1;
			black.r = 1;
			black.g = 1;
			black.b = 1;
			brush->SetColor(black);
			rendertarget->DrawTextLayout(D2D1_POINT_2F{ 0,0 }, textlayout, brush);
			black.a = i;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			brush->SetColor(black);
			rendertarget->FillRectangle(D2D1_RECT_F{ 0,0,(float)realwidth,(float)realheight }, brush);
			rendertarget->EndDraw();
			QueryPerformanceCounter(&currenttime);
			currenttime.QuadPart *= 1000;
			currenttime.QuadPart /= frequency.QuadPart;
			if (15 > (currenttime.QuadPart - lasttime.QuadPart)) {
				Sleep(15 - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
			}
			QueryPerformanceCounter(&lasttime);
			lasttime.QuadPart *= 1000;
			lasttime.QuadPart /= frequency.QuadPart;
		}
		for (float i = 0; i <= 1; i = i + 0.01f) {
			SKR_WinPollEvent();
			rendertarget->BeginDraw();
			black.a = 1;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			rendertarget->Clear(black);
			black.a = 1;
			black.r = 1;
			black.g = 1;
			black.b = 1;
			brush->SetColor(black);
			rendertarget->DrawTextLayout(D2D1_POINT_2F{ 0,0 }, textlayout, brush);
			black.a = i;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			brush->SetColor(black);
			rendertarget->FillRectangle(D2D1_RECT_F{ 0,0,(float)realwidth,(float)realheight }, brush);
			rendertarget->EndDraw();
			QueryPerformanceCounter(&currenttime);
			currenttime.QuadPart *= 1000;
			currenttime.QuadPart /= frequency.QuadPart;
			if (15 > (currenttime.QuadPart - lasttime.QuadPart)) {
				Sleep(15 - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
			}
			QueryPerformanceCounter(&lasttime);
			lasttime.QuadPart *= 1000;
			lasttime.QuadPart /= frequency.QuadPart;
		}
		textlayout->Release();
		introplayed = 1;
		black.a = 1;
		black.r = 0;
		black.g = 0;
		black.b = 0;
	}
	for (float i = 1; i >= 0; i = i - speed) {
		SKR_WinPollEvent();
		rendertarget->BeginDraw();
		rendertarget->Clear(black);
		tmpwinrect = winrecthead;
		while (tmpwinrect != NULL) {
			if (tmpwinrect->condition == NULL || *(tmpwinrect->condition)) {
				vposition.left = tmpwinrect->rposition->x * (realwidth / vw) + (tmpwinrect->width / 2);
				vposition.top = tmpwinrect->rposition->y * (realheight / vh) + (tmpwinrect->width / 2);
				vposition.right = vposition.left + (tmpwinrect->rposition->w * (realwidth / vw)) - tmpwinrect->width;
				vposition.bottom = vposition.top + (tmpwinrect->rposition->h * (realheight / vh)) - tmpwinrect->width;
				brush->SetColor(tmpwinrect->color);
				if (tmpwinrect->radiusx != 0 || tmpwinrect->radiusy != 0) {
					if (tmpwinrect->width == 0) {
						rendertarget->FillRoundedRectangle(D2D1::RoundedRect(vposition, tmpwinrect->radiusx, tmpwinrect->radiusy), brush);
					}
					else {
						rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(vposition, tmpwinrect->radiusx, tmpwinrect->radiusy), brush, tmpwinrect->width);
					}
				}
				else {
					if (tmpwinrect->width == 0) {
						rendertarget->FillRectangle(vposition, brush);
					}
					else {
						rendertarget->DrawRectangle(vposition, brush, tmpwinrect->width);
					}
				}
			}
			tmpwinrect = tmpwinrect->next;
		}
		tmpgameimage = gameimagehead;
		while (tmpgameimage != NULL) {
			if (tmpgameimage->condition == NULL || *(tmpgameimage->condition)) {
				vposition.left = tmpgameimage->rposition->x * (realwidth / vw);
				vposition.top = tmpgameimage->rposition->y * (realheight / vh);
				vposition.right = vposition.left + (tmpgameimage->rposition->w * (realwidth / vw));
				vposition.bottom = vposition.top + (tmpgameimage->rposition->h * (realheight / vh));
				if (tmpgameimage->animation == -1) {
					rendertarget->DrawBitmap(tmpgameimage->bitmap, vposition, tmpgameimage->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, tmpgameimage->source);
				}
				else {
					SKR_HandleGameAnimation(&tmpgameimage);
				}
			}
			if (tmpgameimage != NULL) {
				tmpgameimage = tmpgameimage->next;
			}
		}
		tmpgametext = gametexthead;
		while (tmpgametext != NULL) {
			if (tmpgametext->condition == NULL || *(tmpgametext->condition)) {
				brush->SetColor(tmpgametext->color);
				tmpgametext->point.x = tmpgametext->rposition->x * (realwidth / vw);
				tmpgametext->point.y = tmpgametext->rposition->y * (realheight / vh);
				rendertarget->DrawTextLayout(tmpgametext->point, tmpgametext->textlayout, brush);
			}
			tmpgametext = tmpgametext->next;
		}
		tmpbar = barhead;
		while (tmpbar != NULL) {
			if (tmpbar->condition == NULL || *(tmpbar->condition)) {
				if (tmpbar->direction) {
					if (*(tmpbar->currentvalue) <= 0) {
						tmpbar->front.top = tmpbar->back.bottom;
					}
					else if (*(tmpbar->currentvalue) >= *(tmpbar->maxvalue)) {
						tmpbar->front.top = tmpbar->back.top;
					}
					else {
						tmpbar->front.top = tmpbar->back.bottom - ((tmpbar->back.bottom - tmpbar->back.top) * (*(tmpbar->currentvalue) / (*(tmpbar->maxvalue))));
					}
				}
				else {
					if (*(tmpbar->currentvalue) <= 0) {
						tmpbar->front.right = tmpbar->back.left;
					}
					else if (*(tmpbar->currentvalue) >= *(tmpbar->maxvalue)) {
						tmpbar->front.right = tmpbar->back.right;
					}
					else {
						tmpbar->front.right = tmpbar->back.left + ((tmpbar->back.right - tmpbar->back.left) * (*(tmpbar->currentvalue) / (*(tmpbar->maxvalue))));
					}
				}
				brush->SetColor(tmpbar->backcolor);
				rendertarget->FillRectangle(tmpbar->back, brush);
				brush->SetColor(tmpbar->frontcolor);
				rendertarget->FillRectangle(tmpbar->front, brush);
				brush->SetColor(tmpbar->bordercolor);
				rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(tmpbar->border, tmpbar->radius, tmpbar->radius), brush, tmpbar->width);
			}
			tmpbar = tmpbar->next;
		}
		tmpbutton = buttonhead;
		while (tmpbutton != NULL) {
			if (tmpbutton->condition == NULL || *(tmpbutton->condition)) {
				if (SKR_WinMouseX() >= tmpbutton->border.left && SKR_WinMouseX() <= tmpbutton->border.right && SKR_WinMouseY() >= tmpbutton->border.top && SKR_WinMouseY() <= tmpbutton->border.bottom) {
					brush->SetColor(tmpbutton->frontcolor);
					rendertarget->FillRectangle(tmpbutton->front, brush);
				}
				else {
					brush->SetColor(tmpbutton->backcolor);
					rendertarget->FillRectangle(tmpbutton->back, brush);
				}
				brush->SetColor(tmpbutton->bordercolor);
				rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(tmpbutton->border, tmpbutton->radius, tmpbutton->radius), brush, tmpbutton->width);
			}
			tmpbutton = tmpbutton->next;
		}
		tmpmenuimage = menuimagehead;
		while (tmpmenuimage != NULL) {
			if (tmpmenuimage->condition == NULL || *(tmpmenuimage->condition)) {
				vposition.left = tmpmenuimage->rposition->x * (realwidth / vw);
				vposition.top = tmpmenuimage->rposition->y * (realheight / vh);
				vposition.right = vposition.left + (tmpmenuimage->rposition->w * (realwidth / vw));
				vposition.bottom = vposition.top + (tmpmenuimage->rposition->h * (realheight / vh));
				if (tmpmenuimage->animation == -1) {
					rendertarget->DrawBitmap(tmpmenuimage->bitmap, vposition, tmpmenuimage->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, tmpmenuimage->source);
				}
				else {
					SKR_HandleMenuAnimation(&tmpmenuimage);
				}
			}
			if (tmpmenuimage != NULL) {
				tmpmenuimage = tmpmenuimage->next;
			}
		}
		tmpmenutext = menutexthead;
		while (tmpmenutext != NULL) {
			if (tmpmenutext->condition == NULL || *(tmpmenutext->condition)) {
				brush->SetColor(tmpmenutext->color);
				tmpmenutext->point.x = tmpmenutext->rposition->x * (realwidth / vw);
				tmpmenutext->point.y = tmpmenutext->rposition->y * (realheight / vh);
				rendertarget->DrawTextLayout(tmpmenutext->point, tmpmenutext->textlayout, brush);
			}
			tmpmenutext = tmpmenutext->next;
		}
		fadecolor.a = i;
		brush->SetColor(fadecolor);
		rendertarget->FillRectangle(screen, brush);
		if (textlay != NULL) {
			black.a = i;
			if (textcolor == NULL) {
				black.r = 1;
				black.g = 1;
				black.b = 1;
			}
			else {
				black.r = textcolor->r;
				black.g = textcolor->g;
				black.b = textcolor->b;
			}
			brush->SetColor(black);
			if (textrect == NULL) {
				rendertarget->DrawTextLayout(D2D1_POINT_2F{ 0,0 }, textlay, brush);
			}
			else {
				rendertarget->DrawTextLayout(D2D1_POINT_2F{ textrect->x,textrect->y }, textlay, brush);
			}
			black.a = 1;
			black.r = 0;
			black.g = 0;
			black.b = 0;
		}
		rendertarget->EndDraw();
		QueryPerformanceCounter(&currenttime);
		currenttime.QuadPart *= 1000;
		currenttime.QuadPart /= frequency.QuadPart;
		if (frameduration > (currenttime.QuadPart - lasttime.QuadPart)) {
			Sleep(frameduration - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
		}
		QueryPerformanceCounter(&lasttime);
		lasttime.QuadPart *= 1000;
		lasttime.QuadPart /= frequency.QuadPart;
	}
	if (textlay != NULL) {
		textlay->Release();
	}
	return;
}

void SKR_FadeOut(SKR_Color* color, float speed, const wchar_t* string, const wchar_t* font, float size, SKR_Rect* textrect, SKR_Color* textcolor) {
	IDWriteTextLayout* textlay = NULL;
	if (string != NULL && font != NULL) {
		textfactory->CreateTextFormat(font, NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, size * ((float)realwidth / vw), L"tr-tr", &(textformat));
		textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		if (textrect == NULL) {
			textfactory->CreateTextLayout(string, wcslen(string), textformat, (float)realwidth, (float)realheight, &textlay);
		}
		else {
			textfactory->CreateTextLayout(string, wcslen(string), textformat, textrect->w, textrect->h, &textlay);
		}
		textformat->Release();
	}
	if (focusrect != NULL) {
		xcha = ((vw - focusrect->w) / 2) - focusrect->x;
		ycha = ((vh - focusrect->h) / 2) - focusrect->y;
		if (xchanged + xcha > maxx2) {
			xcha = maxx2 - xchanged;
		}
		if (xchanged + xcha < minx2) {
			xcha = minx2 - xchanged;
		}
		if (ychanged + ycha > maxy2) {
			ycha = maxy2 - ychanged;
		}
		if (ychanged + ycha < miny2) {
			ycha = miny2 - ychanged;
		}
		movefollowlist(xcha, ycha);
		xchanged += xcha;
		ychanged += ycha;
	}
	D2D1_COLOR_F fadecolor;
	if (color != NULL) {
		fadecolor = { color->r,color->g,color->b,1 };
	}
	else {
		fadecolor = { 0,0,0,1 };
	}
	D2D1_RECT_F screen = { 0,0,(float)realwidth,(float)realheight };
	SKR_WinPollEvent();
	if (!introplayed) {
		textfactory->CreateTextFormat(L"Georgia", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 100.0f * ((float)realwidth / 1920), L"tr-tr", &(textformat));
		textformat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
		textformat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
		IDWriteTextLayout* textlayout;
		textfactory->CreateTextLayout(L"ÞÜKRÜ ÇÝRÝÞ ENGINE", wcslen(L"ÞÜKRÜ ÇÝRÝÞ ENGINE"), textformat, (float)realwidth, (float)realheight, &textlayout);
		textformat->Release();
		for (float i = 1; i >= 0; i = i - 0.01f) {
			SKR_WinPollEvent();
			rendertarget->BeginDraw();
			black.a = 1;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			rendertarget->Clear(black);
			black.a = 1;
			black.r = 1;
			black.g = 1;
			black.b = 1;
			brush->SetColor(black);
			rendertarget->DrawTextLayout(D2D1_POINT_2F{ 0,0 }, textlayout, brush);
			black.a = i;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			brush->SetColor(black);
			rendertarget->FillRectangle(D2D1_RECT_F{ 0,0,(float)realwidth,(float)realheight }, brush);
			rendertarget->EndDraw();
			QueryPerformanceCounter(&currenttime);
			currenttime.QuadPart *= 1000;
			currenttime.QuadPart /= frequency.QuadPart;
			if (15 > (currenttime.QuadPart - lasttime.QuadPart)) {
				Sleep(15 - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
			}
			QueryPerformanceCounter(&lasttime);
			lasttime.QuadPart *= 1000;
			lasttime.QuadPart /= frequency.QuadPart;
		}
		for (float i = 0; i <= 1; i = i + 0.01f) {
			SKR_WinPollEvent();
			rendertarget->BeginDraw();
			black.a = 1;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			rendertarget->Clear(black);
			black.a = 1;
			black.r = 1;
			black.g = 1;
			black.b = 1;
			brush->SetColor(black);
			rendertarget->DrawTextLayout(D2D1_POINT_2F{ 0,0 }, textlayout, brush);
			black.a = i;
			black.r = 0;
			black.g = 0;
			black.b = 0;
			brush->SetColor(black);
			rendertarget->FillRectangle(D2D1_RECT_F{ 0,0,(float)realwidth,(float)realheight }, brush);
			rendertarget->EndDraw();
			QueryPerformanceCounter(&currenttime);
			currenttime.QuadPart *= 1000;
			currenttime.QuadPart /= frequency.QuadPart;
			if (15 > (currenttime.QuadPart - lasttime.QuadPart)) {
				Sleep(15 - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
			}
			QueryPerformanceCounter(&lasttime);
			lasttime.QuadPart *= 1000;
			lasttime.QuadPart /= frequency.QuadPart;
		}
		textlayout->Release();
		introplayed = 1;
		black.a = 1;
		black.r = 0;
		black.g = 0;
		black.b = 0;
	}
	for (float i = 0; i <= 1; i = i + speed) {
		SKR_WinPollEvent();
		rendertarget->BeginDraw();
		rendertarget->Clear(black);
		tmpwinrect = winrecthead;
		while (tmpwinrect != NULL) {
			if (tmpwinrect->condition == NULL || *(tmpwinrect->condition)) {
				vposition.left = tmpwinrect->rposition->x * (realwidth / vw) + (tmpwinrect->width / 2);
				vposition.top = tmpwinrect->rposition->y * (realheight / vh) + (tmpwinrect->width / 2);
				vposition.right = vposition.left + (tmpwinrect->rposition->w * (realwidth / vw)) - tmpwinrect->width;
				vposition.bottom = vposition.top + (tmpwinrect->rposition->h * (realheight / vh)) - tmpwinrect->width;
				brush->SetColor(tmpwinrect->color);
				if (tmpwinrect->radiusx != 0 || tmpwinrect->radiusy != 0) {
					if (tmpwinrect->width == 0) {
						rendertarget->FillRoundedRectangle(D2D1::RoundedRect(vposition, tmpwinrect->radiusx, tmpwinrect->radiusy), brush);
					}
					else {
						rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(vposition, tmpwinrect->radiusx, tmpwinrect->radiusy), brush, tmpwinrect->width);
					}
				}
				else {
					if (tmpwinrect->width == 0) {
						rendertarget->FillRectangle(vposition, brush);
					}
					else {
						rendertarget->DrawRectangle(vposition, brush, tmpwinrect->width);
					}
				}
			}
			tmpwinrect = tmpwinrect->next;
		}
		tmpgameimage = gameimagehead;
		while (tmpgameimage != NULL) {
			if (tmpgameimage->condition == NULL || *(tmpgameimage->condition)) {
				vposition.left = tmpgameimage->rposition->x * (realwidth / vw);
				vposition.top = tmpgameimage->rposition->y * (realheight / vh);
				vposition.right = vposition.left + (tmpgameimage->rposition->w * (realwidth / vw));
				vposition.bottom = vposition.top + (tmpgameimage->rposition->h * (realheight / vh));
				if (tmpgameimage->animation == -1) {
					rendertarget->DrawBitmap(tmpgameimage->bitmap, vposition, tmpgameimage->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, tmpgameimage->source);
				}
				else {
					SKR_HandleGameAnimation(&tmpgameimage);
				}
			}
			tmpgameimage = tmpgameimage->next;
		}
		tmpgametext = gametexthead;
		while (tmpgametext != NULL) {
			if (tmpgametext->condition == NULL || *(tmpgametext->condition)) {
				brush->SetColor(tmpgametext->color);
				tmpgametext->point.x = tmpgametext->rposition->x * (realwidth / vw);
				tmpgametext->point.y = tmpgametext->rposition->y * (realheight / vh);
				rendertarget->DrawTextLayout(tmpgametext->point, tmpgametext->textlayout, brush);
			}
			tmpgametext = tmpgametext->next;
		}
		tmpbar = barhead;
		while (tmpbar != NULL) {
			if (tmpbar->condition == NULL || *(tmpbar->condition)) {
				if (tmpbar->direction) {
					if (*(tmpbar->currentvalue) <= 0) {
						tmpbar->front.top = tmpbar->back.bottom;
					}
					else if (*(tmpbar->currentvalue) >= *(tmpbar->maxvalue)) {
						tmpbar->front.top = tmpbar->back.top;
					}
					else {
						tmpbar->front.top = tmpbar->back.bottom - ((tmpbar->back.bottom - tmpbar->back.top) * (*(tmpbar->currentvalue) / (*(tmpbar->maxvalue))));
					}
				}
				else {
					if (*(tmpbar->currentvalue) <= 0) {
						tmpbar->front.right = tmpbar->back.left;
					}
					else if (*(tmpbar->currentvalue) >= *(tmpbar->maxvalue)) {
						tmpbar->front.right = tmpbar->back.right;
					}
					else {
						tmpbar->front.right = tmpbar->back.left + ((tmpbar->back.right - tmpbar->back.left) * (*(tmpbar->currentvalue) / (*(tmpbar->maxvalue))));
					}
				}
				brush->SetColor(tmpbar->backcolor);
				rendertarget->FillRectangle(tmpbar->back, brush);
				brush->SetColor(tmpbar->frontcolor);
				rendertarget->FillRectangle(tmpbar->front, brush);
				brush->SetColor(tmpbar->bordercolor);
				rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(tmpbar->border, tmpbar->radius, tmpbar->radius), brush, tmpbar->width);
			}
			tmpbar = tmpbar->next;
		}
		tmpbutton = buttonhead;
		while (tmpbutton != NULL) {
			if (tmpbutton->condition == NULL || *(tmpbutton->condition)) {
				if (SKR_WinMouseX() >= tmpbutton->border.left && SKR_WinMouseX() <= tmpbutton->border.right && SKR_WinMouseY() >= tmpbutton->border.top && SKR_WinMouseY() <= tmpbutton->border.bottom) {
					brush->SetColor(tmpbutton->frontcolor);
					rendertarget->FillRectangle(tmpbutton->front, brush);
				}
				else {
					brush->SetColor(tmpbutton->backcolor);
					rendertarget->FillRectangle(tmpbutton->back, brush);
				}
				brush->SetColor(tmpbutton->bordercolor);
				rendertarget->DrawRoundedRectangle(D2D1::RoundedRect(tmpbutton->border, tmpbutton->radius, tmpbutton->radius), brush, tmpbutton->width);
			}
			tmpbutton = tmpbutton->next;
		}
		tmpmenuimage = menuimagehead;
		while (tmpmenuimage != NULL) {
			if (tmpmenuimage->condition == NULL || *(tmpmenuimage->condition)) {
				vposition.left = tmpmenuimage->rposition->x * (realwidth / vw);
				vposition.top = tmpmenuimage->rposition->y * (realheight / vh);
				vposition.right = vposition.left + (tmpmenuimage->rposition->w * (realwidth / vw));
				vposition.bottom = vposition.top + (tmpmenuimage->rposition->h * (realheight / vh));
				if (tmpmenuimage->animation == -1) {
					rendertarget->DrawBitmap(tmpmenuimage->bitmap, vposition, tmpmenuimage->opacity, D2D1_BITMAP_INTERPOLATION_MODE_NEAREST_NEIGHBOR, tmpmenuimage->source);
				}
				else {
					SKR_HandleMenuAnimation(&tmpmenuimage);
				}
			}
			tmpmenuimage = tmpmenuimage->next;
		}
		tmpmenutext = menutexthead;
		while (tmpmenutext != NULL) {
			if (tmpmenutext->condition == NULL || *(tmpmenutext->condition)) {
				brush->SetColor(tmpmenutext->color);
				tmpmenutext->point.x = tmpmenutext->rposition->x * (realwidth / vw);
				tmpmenutext->point.y = tmpmenutext->rposition->y * (realheight / vh);
				rendertarget->DrawTextLayout(tmpmenutext->point, tmpmenutext->textlayout, brush);
			}
			tmpmenutext = tmpmenutext->next;
		}
		fadecolor.a = i;
		brush->SetColor(fadecolor);
		rendertarget->FillRectangle(screen, brush);
		if (textlay != NULL) {
			black.a = i;
			if (textcolor == NULL) {
				black.r = 1;
				black.g = 1;
				black.b = 1;
			}
			else {
				black.r = textcolor->r;
				black.g = textcolor->g;
				black.b = textcolor->b;
			}
			brush->SetColor(black);
			if (textrect == NULL) {
				rendertarget->DrawTextLayout(D2D1_POINT_2F{ 0,0 }, textlay, brush);
			}
			else {
				rendertarget->DrawTextLayout(D2D1_POINT_2F{ textrect->x,textrect->y }, textlay, brush);
			}
			black.a = 1;
			black.r = 0;
			black.g = 0;
			black.b = 0;
		}
		rendertarget->EndDraw();
		QueryPerformanceCounter(&currenttime);
		currenttime.QuadPart *= 1000;
		currenttime.QuadPart /= frequency.QuadPart;
		if (frameduration > (currenttime.QuadPart - lasttime.QuadPart)) {
			Sleep(frameduration - (DWORD)(currenttime.QuadPart - lasttime.QuadPart));
		}
		QueryPerformanceCounter(&lasttime);
		lasttime.QuadPart *= 1000;
		lasttime.QuadPart /= frequency.QuadPart;
	}
	if (textlay != NULL) {
		textlay->Release();
	}
	return;
}

void SKR_FocusCamera(SKR_Rect* focusedrect, float minx, float maxx, float miny, float maxy) {
	focusrect = focusedrect;
	minx2 = minx;
	maxx2 = maxx;
	miny2 = miny;
	maxy2 = maxy;
	return;
}

SKR_WinGameImage* SKR_CreateCopyWinGameImage(SKR_WinGameImage* image, SKR_Rect* dest, unsigned char* condition, float opacity, int animation, unsigned int milliseconds) {
	SKR_WinGameImage* rect = (SKR_WinGameImage*)malloc(sizeof(SKR_WinGameImage));
	rect->condition = condition;
	rect->rposition = dest;
	rect->opacity = opacity;
	rect->animation = animation;
	rect->milliseconds = milliseconds;
	rect->played = 0;
	rect->time.QuadPart = 0;
	addrecttofollowlist(rect->rposition);
	rect->next = NULL;
	rect->bitmap = image->bitmap;
	rect->source.left = 0;
	rect->source.top = 0;
	rect->source.right = rect->bitmap->GetSize().width;
	rect->source.bottom = rect->bitmap->GetSize().height;
	rect->copy = 1;
	if (gameimagehead == NULL) {
		gameimagehead = rect;
	}
	else {
		SKR_WinGameImage* tmp = gameimagehead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

SKR_WinMenuImage* SKR_CreateCopyWinMenuImage(SKR_WinMenuImage* image, SKR_Rect* dest, unsigned char* condition, float opacity, int animation, unsigned int milliseconds) {
	SKR_WinMenuImage* rect = (SKR_WinMenuImage*)malloc(sizeof(SKR_WinMenuImage));
	rect->condition = condition;
	rect->rposition = dest;
	rect->opacity = opacity;
	rect->animation = animation;
	rect->milliseconds = milliseconds;
	rect->played = 0;
	rect->time.QuadPart = 0;
	rect->next = NULL;
	rect->bitmap = image->bitmap;
	rect->source.left = 0;
	rect->source.top = 0;
	rect->source.right = rect->bitmap->GetSize().width;
	rect->source.bottom = rect->bitmap->GetSize().height;
	rect->copy = 1;
	if (menuimagehead == NULL) {
		menuimagehead = rect;
	}
	else {
		SKR_WinMenuImage* tmp = menuimagehead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_ShowCursor(unsigned char show) {
	if (show == 0) {
		while (ShowCursor(0) >= 0);
	}
	else {
		while (ShowCursor(1) <= 0);
	}
	return;
}

float SKR_GetResMultiplierX(void) {
	return realwidth / vw;
}

float SKR_GetResMultiplierY(void) {
	return realheight / vh;
}

SKR_Texture* SKR_CreateTexture(SKR_Rect* src, const wchar_t* file, unsigned char flip) {
	SKR_Texture* text = (SKR_Texture*)malloc(sizeof(SKR_Texture));
	wicfactory->CreateDecoderFromFilename(file, NULL, GENERIC_READ, WICDecodeMetadataCacheOnLoad, &bitdecoder);
	bitdecoder->GetFrame(0, &frame);
	wicfactory->CreateBitmapClipper(&clipper);
	wrect.X = (int)(src->x);
	wrect.Y = (int)(src->y);
	wrect.Width = (int)(src->w);
	wrect.Height = (int)(src->h);
	clipper->Initialize(frame, &wrect);
	if (flip != 0) {
		wicfactory->CreateBitmapFlipRotator(&flipper);
		if (flip == 1) {
			flipper->Initialize(clipper, WICBitmapTransformFlipHorizontal);
		}
		else if (flip == 2) {
			flipper->Initialize(clipper, WICBitmapTransformFlipVertical);
		}
	}
	wicfactory->CreateFormatConverter(&converter);
	if (flip != 0) {
		converter->Initialize(flipper, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
		flipper->Release();
	}
	else {
		converter->Initialize(clipper, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeCustom);
	}
	rendertarget->CreateBitmapFromWicBitmap(converter, &(text->bitmap));
	bitdecoder->Release();
	frame->Release();
	converter->Release();
	clipper->Release();
	return text;
}

void SKR_SetRectWidth(SKR_WinRect* winrect, float width) {
	winrect->width = width;
	return;
}

SKR_WinGameImage* SKR_CreateWinGameImagebyTexture(SKR_Texture* texture, SKR_Rect* dest, unsigned char* condition, float opacity, int animation, unsigned int milliseconds) {
	SKR_WinGameImage* rect = (SKR_WinGameImage*)malloc(sizeof(SKR_WinGameImage));
	rect->condition = condition;
	rect->rposition = dest;
	rect->opacity = opacity;
	rect->animation = animation;
	rect->milliseconds = milliseconds;
	rect->played = 0;
	rect->time.QuadPart = 0;
	addrecttofollowlist(rect->rposition);
	rect->next = NULL;
	rect->bitmap = texture->bitmap;
	rect->source.left = 0;
	rect->source.top = 0;
	rect->source.right = rect->bitmap->GetSize().width;
	rect->source.bottom = rect->bitmap->GetSize().height;
	rect->copy = 1;
	if (gameimagehead == NULL) {
		gameimagehead = rect;
	}
	else {
		SKR_WinGameImage* tmp = gameimagehead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

SKR_WinMenuImage* SKR_CreateWinMenuImagebyTexture(SKR_Texture* texture, SKR_Rect* dest, unsigned char* condition, float opacity, int animation, unsigned int milliseconds) {
	SKR_WinMenuImage* rect = (SKR_WinMenuImage*)malloc(sizeof(SKR_WinMenuImage));
	rect->condition = condition;
	rect->rposition = dest;
	rect->opacity = opacity;
	rect->animation = animation;
	rect->milliseconds = milliseconds;
	rect->played = 0;
	rect->time.QuadPart = 0;
	rect->next = NULL;
	rect->bitmap = texture->bitmap;
	rect->source.left = 0;
	rect->source.top = 0;
	rect->source.right = rect->bitmap->GetSize().width;
	rect->source.bottom = rect->bitmap->GetSize().height;
	rect->copy = 1;
	if (menuimagehead == NULL) {
		menuimagehead = rect;
	}
	else {
		SKR_WinMenuImage* tmp = menuimagehead;
		while (tmp->next != NULL) {
			tmp = tmp->next;
		}
		tmp->next = rect;
	}
	return rect;
}

void SKR_DestroyTexture(SKR_Texture* texture) {
	texture->bitmap->Release();
	texture->bitmap = NULL;
	free(texture);
	texture = NULL;
	return;
}

void SKR_SetOpacityGameImage(SKR_WinGameImage* image, float opacity) {
	image->opacity = opacity;
	return;
}

void SKR_SetOpacityMenuImage(SKR_WinMenuImage* image, float opacity) {
	image->opacity = opacity;
	return;
}

void SKR_SetColorGameText(SKR_WinGameText* text, SKR_Color* color) {
	text->color.a = color->a;
	text->color.r = color->r;
	text->color.g = color->g;
	text->color.b = color->b;
	return;
}

void SKR_SetColorMenuText(SKR_WinMenuText* text, SKR_Color* color) {
	text->color.a = color->a;
	text->color.r = color->r;
	text->color.g = color->g;
	text->color.b = color->b;
	return;
}