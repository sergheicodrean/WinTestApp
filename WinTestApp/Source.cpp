#ifndef UNICODE
#define UNICODE
#endif // !UNICODE

#pragma comment (lib, "dwmapi.lib")
#pragma comment(lib, "d2d1")
#pragma warning(disable : 4996)

#include <windows.h>
#include <ShObjIdl.h>
#include <atlbase.h>
#include <dwmapi.h>
#include <d2d1.h>
#include <chrono>
#include <ctime>
#include <windowsx.h>


struct winstate {
	wchar_t name[30];
	int number;
};

void paintCircle(HWND);
void itoaw(int, wchar_t*);
void GetText(wchar_t*,int,int,int);
LRESULT CALLBACK WindowProc(HWND,UINT,WPARAM,LPARAM);
winstate* getState(HWND);
void initDirect(HWND);
void releaseDirect(void);
void showDialog(HWND);

void onLButtonDown(int, int, DWORD, HWND);
void onLButtonUp(HWND);
void onMouseMove(int, int, DWORD, HWND);

ID2D1Factory* pFactory;
ID2D1HwndRenderTarget* pRenderTarget;


void appendToTitle(HWND hwnd, LPCWSTR text)
{
	winstate* state = getState(hwnd);
	wchar_t local[30];
	wcscpy(local, text);

	wcscat(state->name, L" ");
	wcscat(state->name, local);

	SetWindowTextW(hwnd, state->name);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE, PWSTR pCmdLine, int nCmdShow)
{

	const wchar_t CLASS_NAME[] = L"Sample Window Class";

	WNDCLASS wc = { };

	winstate *state = (winstate*)malloc(sizeof(winstate));
	state->number = 0;
	wcscpy(state->name, L"");

	wc.lpfnWndProc = WindowProc;
	wc.hInstance = hInstance;
	wc.lpszClassName = CLASS_NAME;

	RegisterClass(&wc);


	HWND hwnd = CreateWindowEx(
		0
		, CLASS_NAME
		, NULL
		, WS_OVERLAPPEDWINDOW | BS_OWNERDRAW

		, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT

		, NULL, NULL, hInstance, state
	);


	if (hwnd == NULL)
		return 0;

	ShowWindow(hwnd, nCmdShow);

	MSG msg = { };
	while (GetMessage(&msg, NULL, 0, 0))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return 0;
}

class DPIScale
{
static float scaleX;
static float scaleY;

public:
	static void Initialize(ID2D1Factory* pFactory)
	{
		float dpiX, dpiY;
		pFactory->GetDesktopDpi(&dpiX, &dpiY);
		scaleX = dpiX / 96.0F;
		scaleY = dpiY / 96.0F;
	}

	template<typename T>
	static D2D1_POINT_2F PixelsToDips(T x, T y)
	{
		return D2D1::Point2F(static_cast<float>(x) / scaleX, static_cast<float>(y) / scaleY);
	}
};

float DPIScale::scaleX = 1.0F;
float DPIScale::scaleY = 1.0F;

int color = 18;
D2D1_POINT_2F ptMouse;

LRESULT CALLBACK WindowProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	LRESULT dwmres;
	if (DwmDefWindowProc(hWnd, uMsg, wParam, lParam, &dwmres))
		return dwmres;

	switch (uMsg)
	{
	case WM_CREATE:
	{
		initDirect(hWnd);
		DPIScale::Initialize(pFactory);
		appendToTitle(hWnd, L"WM_CREATE");

		RECT rcClient;
		GetWindowRect(hWnd, &rcClient);

		// Inform the application of the frame change.
		SetWindowPos(hWnd,
			NULL,
			rcClient.left, rcClient.top,
			rcClient.right - rcClient.left, rcClient.bottom - rcClient.top,
			SWP_FRAMECHANGED);

		break;
	}
	case WM_NCCALCSIZE:
	{
		if (wParam == TRUE && 0)
		{
			NCCALCSIZE_PARAMS* calc = (NCCALCSIZE_PARAMS*)lParam;
			calc->rgrc[0].left += 10;
			calc->rgrc[0].top += 10;
			calc->rgrc[0].right -= 0;
			calc->rgrc[0].bottom -= 0;

			return 0;
		}
		break;
	}
	case WM_ACTIVATE:
	{
		/*MARGINS margins;
		margins.cxLeftWidth = 0;
		margins.cxRightWidth = 0;
		margins.cyBottomHeight = 0;
		margins.cyTopHeight = 0;

		HRESULT hr = DwmExtendFrameIntoClientArea(hWnd, &margins);

		if (FAILED(hr))
			MessageBox(hWnd, L"fail extending frame", L"", MB_OK);*/

		break;
	}
	case WM_MOUSEMOVE:
	{
		onMouseMove(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam, hWnd);
		paintCircle(hWnd);
		break;
	}
	case WM_RBUTTONDOWN:
	{
		paintCircle(hWnd);
		break;
	}
	case WM_LBUTTONUP:
	{
		onLButtonUp(hWnd);
		break;
	}
	case WM_LBUTTONDOWN:
	{
		onLButtonDown(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam), (DWORD)wParam, hWnd);
		break;
		showDialog(hWnd);
		break;
	}
	case WM_NCCREATE:
	{
		LRESULT res = DefWindowProc(hWnd, uMsg, wParam, lParam);

		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		winstate* ws = (winstate*)cs->lpCreateParams;
		SetWindowLongPtr(hWnd, GWLP_USERDATA, (LONG_PTR)ws);

		appendToTitle(hWnd, L"WM_NCCREATE");
		return res;
	}
	case WM_CLOSE:
		/*if (MessageBoxW(hWnd, L"Bruh", L"FR?", MB_OKCANCEL) == IDOK)
			DestroyWindow(hWnd);
		return 0;*/
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		releaseDirect();
		return 0;
	case WM_PAINT:
	{
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		FillRect(hdc, &ps.rcPaint, (HBRUSH)(30));
		EndPaint(hWnd, &ps);

		paintCircle(hWnd);

		return 0;
	}
	case WM_SIZE:
		int width = LOWORD(lParam);
		int height = HIWORD(lParam);

		wchar_t text[50];
		GetText(text, height, width, color);

		winstate* state = getState(hWnd);
		//wcscpy(state->name, L"");
		wchar_t num[30];
		itoaw(state->number, num);
		state->number++;

		//appendToTitle(hWnd, L"Serghei");
		//appendToTitle(hWnd, num);

		return 0;
	}

	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}


void showDialog(HWND hWnd)
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED | COINIT_DISABLE_OLE1DDE);
	if (FAILED(hr))
		MessageBoxW(hWnd, L"no Com NOT init", L"COM", MB_OK);
	else
	{
		CComPtr<IFileOpenDialog> pDialog;
		hr = pDialog.CoCreateInstance(__uuidof(FileOpenDialog));

		if (SUCCEEDED(hr))
		{
			hr = pDialog->Show(hWnd);
			if (SUCCEEDED(hr))
			{
				CComPtr<IShellItem> pItem;
				hr = pDialog->GetResult(&pItem);
				if (SUCCEEDED(hr))
				{
					PWSTR pszFilePath;
					hr = pItem->GetDisplayName(SIGDN_FILESYSPATH, &pszFilePath);
					if (SUCCEEDED(hr))
					{
						MessageBox(NULL, pszFilePath, L"file path", MB_OK);
						CoTaskMemFree(pszFilePath);
					}
				}
			}
		}

		CoUninitialize();
	}
}

D2D1_ELLIPSE ellipseDrawing;

void onLButtonDown(int pixelX, int pixelY, DWORD, HWND hwnd)
{
	SetCapture(hwnd);
	ellipseDrawing.point = ptMouse = DPIScale::PixelsToDips(pixelX, pixelY);
	ellipseDrawing.radiusX = ellipseDrawing.radiusY = 1.0F;
	InvalidateRect(hwnd, NULL, FALSE);
}

void onMouseMove(int pixelX, int pixelY, DWORD flags, HWND hwnd)
{
	if (flags & MK_LBUTTON)
	{
		const D2D1_POINT_2F dips = DPIScale::PixelsToDips(pixelX, pixelY);

		const float width = (dips.x - ptMouse.x) / 2;
		const float height = (dips.y - ptMouse.y) / 2;

		const float x1 = ptMouse.x + width;
		const float y1 = ptMouse.y + height;

		ellipseDrawing = D2D1::Ellipse(D2D1::Point2F(x1, y1), width, height);
		InvalidateRect(hwnd, NULL, FALSE);
	}
}

void onLButtonUp(HWND)
{
	ReleaseCapture();
}

D2D1_SIZE_U getTargetSize(HWND hwnd)
{
	RECT rc;
	GetClientRect(hwnd, &rc);

	LONG r = rc.right;
	LONG b = rc.bottom;

	FLOAT dpiX, dpiY;
	pFactory->GetDesktopDpi(&dpiX, &dpiY);
	float scaleX = dpiX / 96.0f;
	float scaleY = dpiY / 96.0f;

	r = (float)r / scaleX;
	b = (float)b / scaleY;

	return D2D1::SizeU(r, b);
}

void initDirect(HWND hwnd)
{
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	if (FAILED(hr))
		MessageBox(NULL, L"Failed to init factory", L"", MB_OK);

	D2D1_SIZE_U size = getTargetSize(hwnd);

	hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hwnd, size), &pRenderTarget);
	if (FAILED(hr))
		MessageBox(NULL, L"Failed to init render target", L"", MB_OK);

}

void releaseDirect()
{
	pRenderTarget->Release();
	pFactory->Release();
}

void paintCircle(HWND hwnd)
{
	D2D1_SIZE_U size = getTargetSize(hwnd);
	pRenderTarget->Resize(size);

	D2D1_COLOR_F color = D2D1::ColorF(1.0F, 1.0F, 0, 1.0f);
	ID2D1SolidColorBrush *pBrush;
	pRenderTarget->CreateSolidColorBrush(color, &pBrush);

	//ellipse:
	D2D1_SIZE_F sizeF = pRenderTarget->GetSize();
	float x = sizeF.width / 2;
	float y = sizeF.height / 2;
	float radius = min(x, y);
	D2D1_ELLIPSE ellipse = D2D1::Ellipse(D2D1::Point2F(x, y), radius, radius);

	D2D1_ELLIPSE centerEllipse = D2D1::Ellipse(D2D1::Point2F(x, y), 20, 20);

	ID2D1SolidColorBrush* pSecondBrush;
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.4f, 0.1f, 0.15f, 1.0f), &pSecondBrush);
	
	ID2D1SolidColorBrush* pMinuteBrush;
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.3f, 0.5f, 0.15f, 1.0f), &pMinuteBrush);
	
	ID2D1SolidColorBrush* pHourBrush;
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.4f, 0.55f, 1.0f), &pHourBrush);
	
	ID2D1SolidColorBrush* pEllipseDrawingBrush;
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.4f, 0.8f, 0.55f, 0.7f), &pEllipseDrawingBrush);

	D2D_POINT_2F secondEndPoint = D2D1::Point2F(ellipse.point.x, ellipse.point.y - (ellipse.radiusY * 0.9));
	D2D_POINT_2F hourEndPoint = D2D1::Point2F(ellipse.point.x, ellipse.point.y - (ellipse.radiusY * 0.55));
	D2D_POINT_2F minuteEndPoint = D2D1::Point2F(ellipse.point.x, ellipse.point.y - (ellipse.radiusY * 0.65));

	std::time_t t = std::time(0);
	std::tm* now = std::localtime(&t);

	ID2D1SolidColorBrush* pCenterBrush;
	pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(0.2f, 0.2f, 0.2f, 0.975f), &pCenterBrush);

	PAINTSTRUCT ps;
	BeginPaint(hwnd, &ps);
	//RedrawWindow(hwnd, NULL, NULL, RDW_NOERASE);
	pRenderTarget->BeginDraw();
	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::SkyBlue, 0.0f));
	pRenderTarget->FillEllipse(ellipse, pBrush);
	pRenderTarget->Flush();
	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(now->tm_hour * 30.0F, ellipse.point));
	pRenderTarget->DrawLine(ellipse.point, hourEndPoint, pHourBrush, 9.0F);

	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(now->tm_min * 6.0F, ellipse.point));
	pRenderTarget->DrawLine(ellipse.point, minuteEndPoint, pMinuteBrush, 5.5F);

	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(now->tm_sec * 6.0F, ellipse.point));
	pRenderTarget->DrawLine(ellipse.point, secondEndPoint, pSecondBrush, 2.0F);

	pRenderTarget->FillEllipse(centerEllipse, pCenterBrush);


	pRenderTarget->SetTransform(D2D1::Matrix3x2F::Rotation(0.0F, ellipse.point));
	pRenderTarget->FillEllipse(ellipseDrawing, pEllipseDrawingBrush);

	pRenderTarget->EndDraw();
	EndPaint(hwnd, &ps);

	//release:
	pCenterBrush->Release();
	pHourBrush->Release();
	pMinuteBrush->Release();
	pSecondBrush->Release();
	pBrush->Release();
}

void GetText(wchar_t *destination, int height, int width, int col)
{
	char sheight[20];
	char swidth[20];
	char scol[20];
	itoa(height, sheight, 10);
	itoa(width, swidth, 10);
	itoa(col, scol, 10);

	wchar_t wsheight[20];
	wchar_t wswidth[20];
	wchar_t wscol[20];

	mbstowcs(wsheight, sheight, 20);
	mbstowcs(wswidth, swidth, 20);
	mbstowcs(wscol, scol, 20);

	wchar_t buf[50];
	wcscpy(buf, L"");

	wcscat(buf, L"height: ");
	wcscat(buf, wsheight);
	wcscat(buf, L", width: ");
	wcscat(buf, wswidth);
	wcscat(buf, L", color: ");
	wcscat(buf, wscol);

	wcscpy(destination, buf);
}

winstate* getState(HWND hwnd)
{
	return (winstate*)GetWindowLongPtr(hwnd, GWLP_USERDATA);
}

void itoaw(int value, wchar_t* dest)
{
	char buf[20];
	itoa(value, buf, 10);
	mbstowcs(dest, buf, 30);
}

