#include "Headers/include.h"
#include "Data/basic_data.h"
#include "font.h"
#include "icons.h"



int screen_width;
int screen_height;

static HWND Window = NULL;
static ID3D11Device* pD3dDevice = NULL;
static ID3D11DeviceContext* pD3dDeviceContext = NULL;
static IDXGISwapChain* pSwapChain = NULL;
static ID3D11RenderTargetView* pMainRenderTargetView = NULL;

static LRESULT CALLBACK Windower(HWND hWnd, UINT Message, WPARAM wParam, LPARAM lParam);
extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
const MARGINS Margin = { -1 };
MSG Message = { NULL };
static int aimkey;
static int hitbox;
static void render();

FARPROC GetFuncAddress(HMODULE hModule, LPCSTR lpProcName)
{
	return GetProcAddress(hModule, lpProcName);
}

__forceinline bool HideThread(HANDLE hThread)
{
	typedef NTSTATUS(NTAPI* tNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);
	tNtSetInformationThread _NtSetInformationThread = (tNtSetInformationThread)GetFuncAddress(GetModuleHandle("ntdll.dll"), ("NtSetInformationThread"));

	if (_NtSetInformationThread == NULL)
		return false;

	NTSTATUS Status;

	if (hThread == NULL)
		Status = _NtSetInformationThread(GetCurrentThread(), 0x11, 0, 0);
	else
		Status = _NtSetInformationThread(hThread, 0x11, 0, 0);

	if (Status != 0x00000000)
		return false;
	else
		return true;
}


ImVec2 operator+(const ImVec2& lhs, const ImVec2& rhs)
{
	return ImVec2(lhs.x + rhs.x, lhs.y + rhs.y);
}
float DrawOutlinedText(ImFont* pFont, const std::string& text, const ImVec2& pos, float size, ImU32 color, bool center)
{
	std::stringstream stream(text);
	std::string line;

	float y = 0.0f;
	int i = 0;

	while (std::getline(stream, line))
	{
		ImVec2 textSize = pFont->CalcTextSizeA(size, FLT_MAX, 0.0f, line.c_str());

		if (center)
		{
			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2((pos.x - textSize.x / 2.0f) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2(pos.x - textSize.x / 2.0f, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
		}
		else
		{
			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2((pos.x) + 1, (pos.y + textSize.y * i) - 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());
			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2((pos.x) - 1, (pos.y + textSize.y * i) + 1), ImGui::GetColorU32(ImVec4(0, 0, 0, 255)), line.c_str());

			ImGui::GetBackgroundDrawList()->AddText(pFont, size, ImVec2(pos.x, pos.y + textSize.y * i), ImGui::GetColorU32(color), line.c_str());
		}

		y = pos.y + textSize.y * (i + 1);
		i++;
	}
	return y;
}

std::wstring MBytesToWString(const char* lpcszString)
{
	int len = strlen(lpcszString);
	int unicodeLen = ::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, NULL, 0);
	wchar_t* pUnicode = new wchar_t[unicodeLen + 1];
	memset(pUnicode, 0, (unicodeLen + 1) * sizeof(wchar_t));
	::MultiByteToWideChar(CP_ACP, 0, lpcszString, -1, (LPWSTR)pUnicode, unicodeLen);
	std::wstring wString = (wchar_t*)pUnicode;
	delete[] pUnicode;
	return wString;
}
std::string WStringToUTF8(const wchar_t* lpwcszWString)
{
	char* pElementText;
	int iTextLen = ::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, NULL, 0, NULL, NULL);
	pElementText = new char[iTextLen + 1];
	memset((void*)pElementText, 0, (iTextLen + 1) * sizeof(char));
	::WideCharToMultiByte(CP_UTF8, 0, (LPWSTR)lpwcszWString, -1, pElementText, iTextLen, NULL, NULL);
	std::string strReturn(pElementText);
	delete[] pElementText;
	return strReturn;
}

void OutlinedText(int x, int y, ImColor Color, const char* text)
{
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(x - 1, y), ImColor(0, 0, 0), text);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(x + 1, y), ImColor(0, 0, 0), text);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y - 1), ImColor(0, 0, 0), text);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y + 1), ImColor(0, 0, 0), text);
	ImGui::GetBackgroundDrawList()->AddText(ImVec2(x, y), Color, text);
}

void DrawString(float fontSize, int x, int y, RGBA* color, bool bCenter, bool stroke, const char* pText, ...)
{
	va_list va_alist;
	char buf[1024] = { 0 };
	va_start(va_alist, pText);
	_vsnprintf_s(buf, sizeof(buf), pText, va_alist);
	va_end(va_alist);
	std::string text = WStringToUTF8(MBytesToWString(buf).c_str());
	if (bCenter)
	{
		ImVec2 textSize = ImGui::CalcTextSize(text.c_str());
		x = x - textSize.x / 4;
		y = y - textSize.y;
	}
	if (stroke)
	{
		ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x + 1, y - 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
		ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x - 1, y + 1), ImGui::ColorConvertFloat4ToU32(ImVec4(0, 0, 0, 1)), text.c_str());
	}
	ImGui::GetBackgroundDrawList()->AddText(ImGui::GetFont(), fontSize, ImVec2(x, y), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), text.c_str());
	
}

float radar_pos_x = 20.f;
float radar_pos_y = 50.f;
float radar_size = 330.f;
float radar_range = 60.f;

void RadarRange(float* x, float* y, float range)
{
	if (fabs((*x)) > range || fabs((*y)) > range)
	{
		if ((*y) > (*x))
		{
			if ((*y) > -(*x))
			{
				(*x) = range * (*x) / (*y);
				(*y) = range;
			}
			else
			{
				(*y) = -range * (*y) / (*x);
				(*x) = -range;
			}
		}
		else
		{
			if ((*y) > -(*x))
			{
				(*y) = range * (*y) / (*x);
				(*x) = range;
			}
			else
			{
				(*x) = -range * (*x) / (*y);
				(*y) = -range;
			}
		}
	}
}

void CalcRadarPoint(Vector3 vOrigin, int& screenx, int& screeny)
{
	Vector3 vAngle = W2SCam.Rotation;
	auto fYaw = vAngle.y * M_PI / 180.0f;
	float dx = vOrigin.x - W2SCam.Location.x;
	float dy = vOrigin.y - W2SCam.Location.y;

	//x' = x * cos(p) - y * sin(p)
	//y' = y * sin(p) - y * -cos(p)
	float fsin_yaw = sinf(fYaw);
	float fminus_cos_yaw = -cosf(fYaw);

	float x = dy * fminus_cos_yaw + dx * fsin_yaw;
	x = -x;
	float y = dx * fminus_cos_yaw - dy * fsin_yaw;

	float range = (float)radar_range * 1000.f;

	RadarRange(&x, &y, range);

	ImVec2 DrawPos = ImVec2(radar_pos_x, radar_pos_y);
	ImVec2 DrawSize = ImVec2(radar_size, radar_size);

	int rad_x = (int)DrawPos.x;
	int rad_y = (int)DrawPos.y;

	float r_siz_x = DrawSize.x;
	float r_siz_y = DrawSize.y;

	int x_max = (int)r_siz_x + rad_x - 5;
	int y_max = (int)r_siz_y + rad_y - 5;

	screenx = rad_x + ((int)r_siz_x / 2 + int(x / range * r_siz_x));
	screeny = rad_y + ((int)r_siz_y / 2 + int(y / range * r_siz_y));

	if (screenx > x_max)
		screenx = x_max;

	if (screenx < rad_x)
		screenx = rad_x;

	if (screeny > y_max)
		screeny = y_max;

	if (screeny < rad_y)
		screeny = rad_y;
}

void render_radar_main() {
    ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(radar_pos_x, radar_pos_y), ImVec2(radar_pos_x + radar_size, radar_pos_y + radar_size), ImGui::GetColorU32({ 0.13f, 0.13f, 0.13f, 0.4f }), 0.f, 0);
    ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(radar_pos_x + (radar_size / 2), radar_pos_y + (radar_size / 2)), radar_size / 2, ImGui::GetColorU32({ 1.f, 1.f, 1.f, 1.f }), 64, 1.f);

    // Add the orange circle
    ImVec2 center = ImVec2(radar_pos_x + (radar_size / 2), radar_pos_y + (radar_size / 2));
    float orangeCircleRadius = 6.f;
    ImGui::GetBackgroundDrawList()->AddCircleFilled(center, orangeCircleRadius, ImGui::GetColorU32({ 1.f, 0.5f, 0.f, 1.f }), 12);
	ImGui::GetBackgroundDrawList()->AddCircle(center, 6.f, ImGui::GetColorU32({ 0.f, 0.f, 0.f, 1.f }), 12, 1.f);
	ImGui::GetBackgroundDrawList()->AddCircle(center, 12.f, ImGui::GetColorU32({ 1.f, 1.f, 1.f, 1.f }), 12, 1.f);
}

void add_to_radar(Vector3 WorldLocation, float fDistance, bool visible) {
	int ScreenX, ScreenY = 0;
	CalcRadarPoint(WorldLocation, ScreenX, ScreenY);

	if (!visible)
	{
		ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(ScreenX, ScreenY), 6.f, ImGui::GetColorU32({ 1.f, 0.f, 0.f, 1.f }), 12);
	}
	else
	{
		ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(ScreenX, ScreenY), 6.f, ImGui::GetColorU32({ 0.f, 1.f, 0.f, 1.f }), 12);
	}
	
	ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(ScreenX, ScreenY), 6.f, ImGui::GetColorU32({ 0.f, 0.f, 0.f, 1.f }), 12, 1.f);
	ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(ScreenX, ScreenY), 12.f, ImGui::GetColorU32({ 1.f, 1.f, 1.f, 1.f }), 12, 1.f);

}

static ImColor Color;

void DrawCircleFilled(int x, int y, int radius, RGBA* color, int segments)
{
	ImGui::GetBackgroundDrawList()->AddCircleFilled(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), segments);
}

void DrawLine(int x1, int y1, int x2, int y2, RGBA* color, int thickness)
{
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), thickness);
}


void DrawLine2(int x1, int y1, int x2, int y2, const ImU32& color, int thickness)
{
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x1, y1), ImVec2(x2, y2), ImGui::GetColorU32(color), thickness);
}

void DrawCircle(int x, int y, int radius, RGBA* color, int segments)
{
	ImGui::GetBackgroundDrawList()->AddCircle(ImVec2(x, y), radius, ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 255.0, color->B / 255.0, color->A / 255.0)), segments);
}


void DrawBox(float X, float Y, float W, float H, const ImU32& color, int thickness)
{
	ImGui::GetBackgroundDrawList()->AddRect(ImVec2(X + 1, Y + 1), ImVec2(((X + W) - 1), ((Y + H) - 1)), ImGui::GetColorU32(color), thickness);
	ImGui::GetBackgroundDrawList()->AddRect(ImVec2(X, Y), ImVec2(X + W, Y + H), ImGui::GetColorU32(color), thickness);
}

void DrawCorneredBox(int x, int y, int w, int h, const ImU32& color, int thickness) {
	float lineW = (w / 3);
	float lineH = (h / 3);

	//oben links nach unten
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x, y + lineH), ImGui::GetColorU32(color), thickness);

	//oben links nach rechts (l-mittig)
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y), ImVec2(x + lineW, y), ImGui::GetColorU32(color), thickness);

	//oben rechts (r-mittig) nach rechts
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w - lineW, y), ImVec2(x + w, y), ImGui::GetColorU32(color), thickness);

	//oben rechts nach vert-rechts (oberhalb)
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w, y), ImVec2(x + w, y + lineH), ImGui::GetColorU32(color), thickness);

	//unten vert-links (unterhalb) nach unten links
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y + h - lineH), ImVec2(x, y + h), ImGui::GetColorU32(color), thickness);

	//unten links nach rechts (mittig)
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x, y + h), ImVec2(x + lineW, y + h), ImGui::GetColorU32(color), thickness);

	//unten rechts (mittig) nach rechts
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w - lineW, y + h), ImVec2(x + w, y + h), ImGui::GetColorU32(color), thickness);

	//unten rechts nach vert-rechts (unterhalb)
	ImGui::GetBackgroundDrawList()->AddLine(ImVec2(x + w, y + h - lineH), ImVec2(x + w, y + h), ImGui::GetColorU32(color), thickness);
}

bool showmenu = true;

void UpdateWindower()
{
	while (true)
	{
		if (Hopesar::Game::hwnd)
		{
			if (Hopesar::Settings::Aimbot::hitboxpos == 0)
			{
				hitbox = 68; //head
			}
			else if (Hopesar::Settings::Aimbot::hitboxpos == 1)
			{
				hitbox = 65; //neck
			}
			else if (Hopesar::Settings::Aimbot::hitboxpos == 2)
			{
				hitbox = 7; //chest
			}
			else if (Hopesar::Settings::Aimbot::hitboxpos == 3)
			{
				hitbox = 2; //pelvis
			}

			
			if (GetAsyncKeyState(VK_F5) & 1)
			{
				exit(0);
				*(uintptr_t*)(0) = 0;
			}

			ZeroMemory(&Hopesar::Overlay::GRect, sizeof(Hopesar::Overlay::GRect));
			GetWindowRect(Hopesar::Game::hwnd, &Hopesar::Overlay::GRect);
			Hopesar::Overlay::Width = Hopesar::Overlay::GRect.right - Hopesar::Overlay::GRect.left;
			Hopesar::Overlay::Height = Hopesar::Overlay::GRect.bottom - Hopesar::Overlay::GRect.top;
			DWORD dwStyle = GetWindowLongA_Spoofed(Hopesar::Game::hwnd, GWL_STYLE);

			if (dwStyle & WS_BORDER)
			{
				Hopesar::Overlay::GRect.top += 32;
				Hopesar::Overlay::Height -= 39;
			}
			Hopesar::Overlay::CenterX = Hopesar::Overlay::Width / 2;
			Hopesar::Overlay::CenterY = Hopesar::Overlay::Height / 2;
			MoveWindow_Spoofed(Window, Hopesar::Overlay::GRect.left, Hopesar::Overlay::GRect.top, Hopesar::Overlay::Width, Hopesar::Overlay::Height, true);
		}
		else
		{
			exit(0);
		}
		Sleep(2);
	}
}


void DrawFilledRect(int x, int y, int w, int h, RGBA* color)
{
	ImGui::GetBackgroundDrawList()->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + h), ImGui::ColorConvertFloat4ToU32(ImVec4(color->R / 255.0, color->G / 153.0, color->B / 51.0, color->A / 255.0)), 0, 0);

}
void DrawNormalBox(int x, int y, int w, int h, int borderPx, RGBA* color, RGBA* color2)
{
	DrawFilledRect(x + borderPx, y, w, borderPx, color); //top 
	DrawFilledRect(x + w - w + borderPx, y, w, borderPx, color); //top 
	DrawFilledRect(x, y, borderPx, h, color); //left 
	DrawFilledRect(x, y + h - h + borderPx * 2, borderPx, h, color2); //left 
	DrawFilledRect(x + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
	DrawFilledRect(x + w - w + borderPx, y + h + borderPx, w, borderPx, color); //bottom 
	DrawFilledRect(x + w + borderPx, y, borderPx, h, color);//right 
	DrawFilledRect(x + w + borderPx, y + h - h + borderPx * 2, borderPx, h, color2);//right 
}

void CleanupRenderTarget()
{
	if (pMainRenderTargetView) { pMainRenderTargetView->Release(); pMainRenderTargetView = NULL; }
}

void CreateRenderTarget()
{
	ID3D11Texture2D* pBackBuffer;
	pSwapChain->GetBuffer(0, IID_PPV_ARGS(&pBackBuffer));
	pD3dDevice->CreateRenderTargetView(pBackBuffer, NULL, &pMainRenderTargetView);
	pBackBuffer->Release();
}

LRESULT CALLBACK Windower(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{

	if (ImGui_ImplWin32_WndProcHandler(hWnd, msg, wParam, lParam))
		return true;

	switch (msg)
	{
	case WM_SIZE:
		if (pD3dDevice != nullptr && wParam != SIZE_MINIMIZED)
		{
			CleanupRenderTarget();
			pSwapChain->ResizeBuffers(0, (UINT)LOWORD(lParam), (UINT)HIWORD(lParam), DXGI_FORMAT_UNKNOWN, 0);
			CreateRenderTarget();
		}
		return 0;

	case WM_SYSCOMMAND:
		if ((wParam & 0xfff0) == SC_KEYMENU)
			return 0;
		break;

	case WM_DESTROY:
		::PostQuitMessage(0);
		return 0;

	case WM_DPICHANGED:
		if (ImGui::GetIO().ConfigFlags & ImGuiConfigFlags_DpiEnableScaleViewports)
		{
			const RECT* suggested_rect = (RECT*)lParam;
			::SetWindowPos(hWnd, nullptr, suggested_rect->left, suggested_rect->top, suggested_rect->right - suggested_rect->left, suggested_rect->bottom - suggested_rect->top, SWP_NOZORDER | SWP_NOACTIVATE);
		}
		break;

	default:
		break;
	}
	return ::DefWindowProc(hWnd, msg, wParam, lParam);
}


DWORD MenuLoop(LPVOID in)
{
	while (1)
	{
		if (GetAsyncKeyState_Spoofed(VK_INSERT) & 1) {
			Hopesar::Settings::GUI::ShowGui = !Hopesar::Settings::GUI::ShowGui;
		}
		Sleep(2);
	}
}


void Windowa()
{
	CreateThread_Spoofed(0, 0, (LPTHREAD_START_ROUTINE)UpdateWindower, 0, 0, 0);
	WNDCLASSEX win_class = { sizeof(WNDCLASSEX), CS_CLASSDC, Windower, 0L, 0L, GetModuleHandle(nullptr), nullptr, nullptr, nullptr, nullptr, skCrypt("Discord Token Protector"), nullptr };

	if (!RegisterClassExA_Spoofed(&win_class))
		exit(1);


	
	Window = CreateWindowExA_Spoofed(NULL, skCrypt("Discord Token Protector"), skCrypt("Discord Token Protector"), WS_POPUP | WS_VISIBLE, Hopesar::Overlay::Width + 10, Hopesar::Overlay::Height + 5, Hopesar::Overlay::Width + 5, Hopesar::Overlay::Height + 3, NULL, NULL, 0, NULL);
	DwmExtendFrameIntoClientArea(Window, &Margin);
	SetWindowLongA_Spoofed(Window, GWL_EXSTYLE, (int)GetWindowLongA_Spoofed(Window, GWL_EXSTYLE) | WS_EX_TOOLWINDOW | WS_EX_LAYERED | WS_EX_TRANSPARENT);
	SetLayeredWindowAttributes_Spoofed(Window, RGB(0, 0, 0), 0, ULW_COLORKEY);
	SetLayeredWindowAttributes_Spoofed(Window, 0, 255, LWA_ALPHA);
	ShowWindow_Spoofed(Window, SW_SHOW);
	UpdateWindow(Window);
}
ImFont* fontpoppin;

ImFont* cristianafont;


boolean in_rect(double centerX, double centerY, double radius, double x, double y) {
	return x >= centerX - radius && x <= centerX + radius &&
		y >= centerY - radius && y <= centerY + radius;
}

void InitDesign()
{
	DXGI_SWAP_CHAIN_DESC sd;
	ZeroMemory(&sd, sizeof(sd));
	sd.BufferCount = 2;
	sd.BufferDesc.Width = 0;
	sd.BufferDesc.Height = 0;
	sd.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	sd.BufferDesc.RefreshRate.Numerator = 60;
	sd.BufferDesc.RefreshRate.Denominator = 1;
	sd.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH;
	sd.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	sd.OutputWindow = Window;
	sd.SampleDesc.Count = 1;
	sd.SampleDesc.Quality = 0;
	sd.Windowed = TRUE;
	sd.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;

	UINT createDeviceFlags = 0;
	//createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
	D3D_FEATURE_LEVEL featureLevel;
	const D3D_FEATURE_LEVEL featureLevelArray[2] = { D3D_FEATURE_LEVEL_11_0, D3D_FEATURE_LEVEL_10_0, };
	D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, createDeviceFlags, featureLevelArray, 2, D3D11_SDK_VERSION, &sd, &pSwapChain, &pD3dDevice, &featureLevel, &pD3dDeviceContext);
	CreateRenderTarget();

	IMGUI_CHECKVERSION();

	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui_ImplWin32_Init(Window);
	ImGui_ImplDX11_Init(pD3dDevice, pD3dDeviceContext);

	ImFontAtlas* fontAtlas = ImGui::GetIO().Fonts;
	cristianafont = io.Fonts->AddFontFromFileTTF(skCrypt("C:\\Windows\\Fonts\\Bahnschrift.ttf"), 18.0f, nullptr, io.Fonts->GetGlyphRangesDefault());
	const ImVec4 activeColor = ImVec4(0.1f, 0.7f, 0.4f, 1.0f);

	// set background color
	
	ImGui::StyleColorsDark();
	ImVec4* colors = ImGui::GetStyle().Colors;


	colors[ImGuiCol_WindowBg] = ImColor(23, 23, 23);
	colors[ImGuiCol_ChildBg] = ImColor(37, 37, 37);

	colors[ImGuiCol_CheckMark] = activeColor;


	colors[ImGuiCol_Text] = ImVec4(1.0f, 1.0f, 1.0f, 5.5f);
	colors[ImGuiCol_Button] = ImColor(37, 37, 37);
	colors[ImGuiCol_SliderGrab] = ImColor(253, 201, 58);
	colors[ImGuiCol_SliderGrabActive] = ImColor(249, 184, 7);
	colors[ImGuiCol_ButtonActive] = ImColor(37, 37, 37);
	colors[ImGuiCol_ButtonHovered] = ImColor(37, 37, 37);
	colors[ImGuiCol_FrameBg] = ImColor(75, 75, 75);
	colors[ImGuiCol_FrameBgHovered] = ImColor(75, 75, 75);
	colors[ImGuiCol_FrameBgActive] = ImColor(75, 75, 75);
	colors[ImGuiCol_Border] = ImVec4(1.0f, 0.5f, 0.0f, 1.0f);

	ImGuiStyle* style = &ImGui::GetStyle();
	style->WindowPadding = ImVec2(5, 5);
	style->WindowRounding = 5.0f;
	style->FramePadding = ImVec2(0, 0);
	style->FrameRounding = 0.0f;
	style->ItemSpacing = ImVec2(8, 8);
	style->ItemInnerSpacing = ImVec2(8, 6);
	style->IndentSpacing = 25.0f;
	style->ScrollbarSize = 15.0f;
	style->ScrollbarRounding = 0.0f;
	style->GrabMinSize = 10.0f;
	style->GrabRounding = 0.0f;
	style->ChildRounding = 5.f;
	style->ButtonTextAlign = { .05, .5 };
	style->WindowTitleAlign.x = 0.50f;
	style->FrameRounding = 2.0f;

	style->Colors[ImGuiCol_ScrollbarBg] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrab] = ImVec4(0.80f, 0.80f, 0.83f, 0.31f);
	style->Colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ScrollbarGrabActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_Header] = ImVec4(0.10f, 0.09f, 0.12f, 1.00f);
	style->Colors[ImGuiCol_HeaderHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_HeaderActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_ResizeGrip] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
	style->Colors[ImGuiCol_ResizeGripHovered] = ImVec4(0.56f, 0.56f, 0.58f, 1.00f);
	style->Colors[ImGuiCol_ResizeGripActive] = ImVec4(0.06f, 0.05f, 0.07f, 1.00f);
	style->Colors[ImGuiCol_PlotLines] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotLinesHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);
	style->Colors[ImGuiCol_PlotHistogram] = ImVec4(0.40f, 0.39f, 0.38f, 0.63f);
	style->Colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.25f, 1.00f, 0.00f, 1.00f);

	pD3dDevice->Release();
}


void StartCheat()
{
	static RECT old_rc;
	ZeroMemory(&Message, sizeof(MSG));

	while (Message.message != WM_QUIT)
	{

		if (PeekMessageA_Spoofed(&Message, Window, 0, 0, PM_REMOVE))
		{
			TranslateMessage(&Message);
			DispatchMessage(&Message);
		}
		
		HWND hwnd_active = GetForegroundWindow_Spoofed();

		
			HWND hwndtest = GetWindow(Hopesar::Game::hwnd, GW_HWNDPREV);
			SetWindowPos_Spoofed(Window, hwndtest, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		

		if (GetAsyncKeyState(0x23) & 1)
			exit(8);


		RECT rc;
		POINT xy;

		ZeroMemory(&rc, sizeof(RECT));
		ZeroMemory(&xy, sizeof(POINT));
		GetClientRect(Hopesar::Game::hwnd, &rc);
		ClientToScreen(Hopesar::Game::hwnd, &xy);
		rc.left = xy.x;
		rc.top = xy.y;

		ImGuiIO& io = ImGui::GetIO();
		io.ImeWindowHandle = Hopesar::Game::hwnd;
		io.DeltaTime = 1.0f / 60.0f;


		POINT p;
		GetCursorPosA(&p);
		io.MousePos.x = p.x - xy.x;
		io.MousePos.y = p.y - xy.y;

		if (GetAsyncKeyState_Spoofed(VK_LBUTTON)) {
			io.MouseDown[0] = true;
			io.MouseClicked[0] = true;
			io.MouseClickedPos[0].x = io.MousePos.x;
			io.MouseClickedPos[0].x = io.MousePos.y;
		}
		else
			io.MouseDown[0] = false;

		if (rc.left != old_rc.left || rc.right != old_rc.right || rc.top != old_rc.top || rc.bottom != old_rc.bottom)
		{
			old_rc = rc;

			Hopesar::Overlay::Width = rc.right;
			Hopesar::Overlay::Height = rc.bottom;

			Hopesar::Overlay::Parameters.BackBufferWidth = Hopesar::Overlay::Width;
			Hopesar::Overlay::Parameters.BackBufferHeight = Hopesar::Overlay::Height;
			SetWindowPos_Spoofed(Window, (HWND)0, xy.x, xy.y, Hopesar::Overlay::Width, Hopesar::Overlay::Height, SWP_NOREDRAW);
			//pD3dDevice->Reset(&Hopesar::Overlay::Parameters);
		}
		render();
	}
	ImGui_ImplDX11_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();

	DestroyWindow(Window);
}

void SetUp()
{

	Windowa();
	InitDesign();
	StartCheat();

	
}