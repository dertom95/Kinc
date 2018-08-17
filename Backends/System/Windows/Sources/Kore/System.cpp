#include "pch.h"

#ifdef KORE_G4ONG5
#include <Kore/Graphics5/Graphics.h>
#elif KORE_G4
#include <Kore/Graphics4/Graphics.h>
#else
#include <Kore/Graphics3/Graphics.h>
#endif

#include <Kore/Display.h>
#include <Kore/Input/Gamepad.h>
#include <Kore/Input/Keyboard.h>
#include <Kore/Input/Mouse.h>
#include <Kore/Input/Pen.h>
#include <Kore/Window.h>
#include <Kore/Windows.h>

#include <C/Kore/Log.h>
#include <C/Kore/System.h>
#include <C/Kore/Window.h>

#define DIRECTINPUT_VERSION 0x0800
#ifdef WIN32_LEAN_AND_MEAN
#undef WIN32_LEAN_AND_MEAN
#endif
#include <dinput.h>
#include <oleauto.h>
#include <stdio.h>
#include <wbemidl.h>

#ifdef KORE_OCULUS
#include "Kore/Vr/VrInterface.h"
#endif

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <Windows.h>
#include <Windowsx.h>
#include <XInput.h>
#include <exception>
#include <shlobj.h>

#ifdef KORE_G4ONG5
#define Graphics Graphics5
#elif KORE_G4
#define Graphics Graphics4
#else
#define Graphics Graphics3
#endif

extern "C" __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
extern "C" __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;

typedef BOOL(WINAPI *GetPointerInfoType)(UINT32 pointerId, POINTER_INFO *pointerInfo);
static GetPointerInfoType MyGetPointerInfo = NULL;
typedef BOOL(WINAPI *GetPointerPenInfoType)(UINT32 pointerId, POINTER_PEN_INFO *penInfo);
static GetPointerPenInfoType MyGetPointerPenInfo = NULL;
typedef BOOL(WINAPI *EnableNonClientDpiScalingType)(HWND hwnd);
static EnableNonClientDpiScalingType MyEnableNonClientDpiScaling = NULL;

static int mouseX, mouseY;
static bool keyPressed[256];
Kore::KeyCode keyTranslated[256]; // http://msdn.microsoft.com/en-us/library/windows/desktop/dd375731(v=vs.85).aspx

static void initKeyTranslation() {
	for (int i = 0; i < 256; ++i) keyTranslated[i] = Kore::KeyUnknown;

	keyTranslated[VK_BACK] = Kore::KeyBackspace;
	keyTranslated[VK_TAB] = Kore::KeyTab;
	keyTranslated[VK_CLEAR] = Kore::KeyClear;
	keyTranslated[VK_RETURN] = Kore::KeyReturn;
	keyTranslated[VK_SHIFT] = Kore::KeyShift;
	keyTranslated[VK_CONTROL] = Kore::KeyControl;
	keyTranslated[VK_MENU] = Kore::KeyAlt;
	keyTranslated[VK_PAUSE] = Kore::KeyPause;
	keyTranslated[VK_CAPITAL] = Kore::KeyCapsLock;
	keyTranslated[VK_KANA] = Kore::KeyKana;
	// keyTranslated[VK_HANGUEL]
	keyTranslated[VK_HANGUL] = Kore::KeyHangul;
	keyTranslated[VK_JUNJA] = Kore::KeyJunja;
	keyTranslated[VK_FINAL] = Kore::KeyFinal;
	keyTranslated[VK_HANJA] = Kore::KeyHanja;
	keyTranslated[VK_KANJI] = Kore::KeyKanji;
	keyTranslated[VK_ESCAPE] = Kore::KeyEscape;
	// keyTranslated[VK_CONVERT]
	// keyTranslated[VK_NONCONVERT
	// keyTranslated[VK_ACCEPT
	// keyTranslated[VK_MODECHANGE
	keyTranslated[VK_SPACE] = Kore::KeySpace;
	keyTranslated[VK_PRIOR] = Kore::KeyPageUp;
	keyTranslated[VK_NEXT] = Kore::KeyPageDown;
	keyTranslated[VK_END] = Kore::KeyEnd;
	keyTranslated[VK_HOME] = Kore::KeyHome;
	keyTranslated[VK_LEFT] = Kore::KeyLeft;
	keyTranslated[VK_UP] = Kore::KeyUp;
	keyTranslated[VK_RIGHT] = Kore::KeyRight;
	keyTranslated[VK_DOWN] = Kore::KeyDown;
	// keyTranslated[VK_SELECT
	keyTranslated[VK_PRINT] = Kore::KeyPrint;
	// keyTranslated[VK_EXECUTE
	// keyTranslated[VK_SNAPSHOT
	keyTranslated[VK_INSERT] = Kore::KeyInsert;
	keyTranslated[VK_DELETE] = Kore::KeyDelete;
	keyTranslated[VK_HELP] = Kore::KeyHelp;
	keyTranslated[0x30] = Kore::Key0;
	keyTranslated[0x31] = Kore::Key1;
	keyTranslated[0x32] = Kore::Key2;
	keyTranslated[0x33] = Kore::Key3;
	keyTranslated[0x34] = Kore::Key4;
	keyTranslated[0x35] = Kore::Key5;
	keyTranslated[0x36] = Kore::Key6;
	keyTranslated[0x37] = Kore::Key7;
	keyTranslated[0x38] = Kore::Key8;
	keyTranslated[0x39] = Kore::Key9;
	keyTranslated[0x41] = Kore::KeyA;
	keyTranslated[0x42] = Kore::KeyB;
	keyTranslated[0x43] = Kore::KeyC;
	keyTranslated[0x44] = Kore::KeyD;
	keyTranslated[0x45] = Kore::KeyE;
	keyTranslated[0x46] = Kore::KeyF;
	keyTranslated[0x47] = Kore::KeyG;
	keyTranslated[0x48] = Kore::KeyH;
	keyTranslated[0x49] = Kore::KeyI;
	keyTranslated[0x4A] = Kore::KeyJ;
	keyTranslated[0x4B] = Kore::KeyK;
	keyTranslated[0x4C] = Kore::KeyL;
	keyTranslated[0x4D] = Kore::KeyM;
	keyTranslated[0x4E] = Kore::KeyN;
	keyTranslated[0x4F] = Kore::KeyO;
	keyTranslated[0x50] = Kore::KeyP;
	keyTranslated[0x51] = Kore::KeyQ;
	keyTranslated[0x52] = Kore::KeyR;
	keyTranslated[0x53] = Kore::KeyS;
	keyTranslated[0x54] = Kore::KeyT;
	keyTranslated[0x55] = Kore::KeyU;
	keyTranslated[0x56] = Kore::KeyV;
	keyTranslated[0x57] = Kore::KeyW;
	keyTranslated[0x58] = Kore::KeyX;
	keyTranslated[0x59] = Kore::KeyY;
	keyTranslated[0x5A] = Kore::KeyZ;
	// keyTranslated[VK_LWIN
	// keyTranslated[VK_RWIN
	// keyTranslated[VK_APPS
	// keyTranslated[VK_SLEEP
	keyTranslated[VK_NUMPAD0] = Kore::KeyNumpad0;
	keyTranslated[VK_NUMPAD1] = Kore::KeyNumpad1;
	keyTranslated[VK_NUMPAD2] = Kore::KeyNumpad2;
	keyTranslated[VK_NUMPAD3] = Kore::KeyNumpad3;
	keyTranslated[VK_NUMPAD4] = Kore::KeyNumpad4;
	keyTranslated[VK_NUMPAD5] = Kore::KeyNumpad5;
	keyTranslated[VK_NUMPAD6] = Kore::KeyNumpad6;
	keyTranslated[VK_NUMPAD7] = Kore::KeyNumpad7;
	keyTranslated[VK_NUMPAD8] = Kore::KeyNumpad8;
	keyTranslated[VK_NUMPAD9] = Kore::KeyNumpad9;
	keyTranslated[VK_MULTIPLY] = Kore::KeyMultiply;
	// keyTranslated[VK_ADD]
	// keyTranslated[VK_SEPARATOR
	// keyTranslated[VK_SUBTRACT
	// keyTranslated[VK_DECIMAL
	// keyTranslated[VK_DIVIDE
	keyTranslated[VK_F1] = Kore::KeyF1;
	keyTranslated[VK_F2] = Kore::KeyF2;
	keyTranslated[VK_F3] = Kore::KeyF3;
	keyTranslated[VK_F4] = Kore::KeyF4;
	keyTranslated[VK_F5] = Kore::KeyF5;
	keyTranslated[VK_F6] = Kore::KeyF6;
	keyTranslated[VK_F7] = Kore::KeyF7;
	keyTranslated[VK_F8] = Kore::KeyF8;
	keyTranslated[VK_F9] = Kore::KeyF9;
	keyTranslated[VK_F10] = Kore::KeyF10;
	keyTranslated[VK_F11] = Kore::KeyF11;
	keyTranslated[VK_F12] = Kore::KeyF12;
	keyTranslated[VK_F13] = Kore::KeyF13;
	keyTranslated[VK_F14] = Kore::KeyF14;
	keyTranslated[VK_F15] = Kore::KeyF15;
	keyTranslated[VK_F16] = Kore::KeyF16;
	keyTranslated[VK_F17] = Kore::KeyF17;
	keyTranslated[VK_F18] = Kore::KeyF18;
	keyTranslated[VK_F19] = Kore::KeyF19;
	keyTranslated[VK_F20] = Kore::KeyF20;
	keyTranslated[VK_F21] = Kore::KeyF21;
	keyTranslated[VK_F22] = Kore::KeyF22;
	keyTranslated[VK_F23] = Kore::KeyF23;
	keyTranslated[VK_F24] = Kore::KeyF24;
	keyTranslated[VK_NUMLOCK] = Kore::KeyNumLock;
	keyTranslated[VK_SCROLL] = Kore::KeyScrollLock;
	// 0x92-96 //OEM specific
	keyTranslated[VK_LSHIFT] = Kore::KeyShift;
	keyTranslated[VK_RSHIFT] = Kore::KeyShift;
	keyTranslated[VK_LCONTROL] = Kore::KeyControl;
	keyTranslated[VK_RCONTROL] = Kore::KeyControl;
	// keyTranslated[VK_LMENU
	// keyTranslated[VK_RMENU
	// keyTranslated[VK_BROWSER_BACK
	// keyTranslated[VK_BROWSER_FORWARD
	// keyTranslated[VK_BROWSER_REFRESH
	// keyTranslated[VK_BROWSER_STOP
	// keyTranslated[VK_BROWSER_SEARCH
	// keyTranslated[VK_BROWSER_FAVORITES
	// keyTranslated[VK_BROWSER_HOME
	// keyTranslated[VK_VOLUME_MUTE
	// keyTranslated[VK_VOLUME_DOWN
	// keyTranslated[VK_VOLUME_UP
	// keyTranslated[VK_MEDIA_NEXT_TRACK
	// keyTranslated[VK_MEDIA_PREV_TRACK
	// keyTranslated[VK_MEDIA_STOP
	// keyTranslated[VK_MEDIA_PLAY_PAUSE
	// keyTranslated[VK_LAUNCH_MAIL
	// keyTranslated[VK_LAUNCH_MEDIA_SELECT
	// keyTranslated[VK_LAUNCH_APP1
	// keyTranslated[VK_LAUNCH_APP2
	// keyTranslated[VK_OEM_1 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ';:' key
	keyTranslated[VK_OEM_PLUS] = Kore::KeyPlus;
	keyTranslated[VK_OEM_COMMA] = Kore::KeyComma;
	keyTranslated[VK_OEM_MINUS] = Kore::KeyHyphenMinus;
	keyTranslated[VK_OEM_PERIOD] = Kore::KeyPeriod;
	// keyTranslated[VK_OEM_2 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '/?' key
	// keyTranslated[VK_OEM_3 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '`~' key
	// keyTranslated[VK_OEM_4 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '[{' key
	// keyTranslated[VK_OEM_5 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the '\|' key
	// keyTranslated[VK_OEM_6 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the ']}' key
	// keyTranslated[VK_OEM_7 //Used for miscellaneous characters; it can vary by keyboard. For the US standard keyboard, the 'single-quote/double-quote'
	// key
	// keyTranslated[VK_OEM_8 //Used for miscellaneous characters; it can vary by keyboard.
	// keyTranslated[0xE1 //OEM specific
	// keyTranslated[VK_OEM_102 //Either the angle bracket key or the backslash key on the RT 102-key keyboard
	// 0xE3-E4 //OEM specific
	// keyTranslated[VK_PROCESSKEY
	// 0xE6 //OEM specific
	// keyTranslated[VK_PACKET //Used to pass Unicode characters as if they were keystrokes. The VK_PACKET key is the low word of a 32-bit Virtual Key value
	// used for non-keyboard input methods.
	// 0xE9-F5 //OEM specific
	// keyTranslated[VK_ATTN
	// keyTranslated[VK_CRSEL
	// keyTranslated[VK_EXSEL
	// keyTranslated[VK_EREOF
	// keyTranslated[VK_PLAY
	// keyTranslated[VK_ZOOM
	// keyTranslated[VK_NONAME
	// keyTranslated[VK_PA1
	// keyTranslated[PA1 key
	// keyTranslated[VK_OEM_CLEAR
}

static unsigned r = 0;

static wchar_t toUnicode(WPARAM wParam, LPARAM lParam) {
	wchar_t buffer[11];
	BYTE state[256];
	GetKeyboardState(state);
	ToUnicode(wParam, (lParam >> 8) & 0xFFFFFF00, state, buffer, 10, 0);
	return buffer[0];
}

extern "C" LRESULT WINAPI KoreWindowsMessageProcedure(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	int windowId;
	DWORD pointerId;
	POINTER_INFO pointerInfo = {NULL};
	POINTER_PEN_INFO penInfo = {NULL};
	static bool controlDown = false;

	switch (msg) {
	case WM_NCCREATE:
		if (MyEnableNonClientDpiScaling != nullptr) {
			MyEnableNonClientDpiScaling(hWnd);
		}
		break;
	case WM_DPICHANGED: {
		int window = Kore_Windows_WindowIndexFromHWND(hWnd);
		if (window >= 0) {
			Kore_Internal_CallPpiChangedCallback(window, LOWORD(wParam));
		}
		break;
	}
	case WM_MOVE:
	case WM_MOVING:
	case WM_SIZING:
		// Scheduler::breakTime();
		break;
	case WM_SIZE: {
		int window = Kore_Windows_WindowIndexFromHWND(hWnd);
		if (window >= 0) {
			int width = LOWORD(lParam);
			int height = HIWORD(lParam);
			Kore::Graphics::_resize(window, width, height);
			Kore_Internal_CallResizeCallback(window, width, height);
		}
		break;
	}
	case WM_DESTROY:
		Kore_Stop();
		return 0;
	case WM_ERASEBKGND:
		return 1;
	case WM_ACTIVATE:
		if (LOWORD(wParam) == WA_ACTIVE)
			Kore::Mouse::the()->_activated(Kore_Windows_WindowIndexFromHWND(hWnd), true);
		else
			Kore::Mouse::the()->_activated(Kore_Windows_WindowIndexFromHWND(hWnd), false);
		break;
	case WM_MOUSELEAVE:
		windowId = Kore_Windows_WindowIndexFromHWND(hWnd);
		//**windows[windowId]->isMouseInside = false;
		Kore::Mouse::the()->___leave(windowId);
		break;
	case WM_MOUSEMOVE:
		windowId = Kore_Windows_WindowIndexFromHWND(hWnd);
		/*if (!windows[windowId]->isMouseInside) {
		    windows[windowId]->isMouseInside = true;
		    TRACKMOUSEEVENT tme;
		    tme.cbSize = sizeof(TRACKMOUSEEVENT);
		    tme.dwFlags = TME_LEAVE;
		    tme.hwndTrack = hWnd;
		    TrackMouseEvent(&tme);
		}*/
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_move(windowId, mouseX, mouseY);
		break;
	case WM_LBUTTONDOWN:
		if (!Kore::Mouse::the()->isLocked(Kore_Windows_WindowIndexFromHWND(hWnd))) SetCapture(hWnd);
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_press(Kore_Windows_WindowIndexFromHWND(hWnd), 0, mouseX, mouseY);
		break;
	case WM_LBUTTONUP:
		if (!Kore::Mouse::the()->isLocked(Kore_Windows_WindowIndexFromHWND(hWnd))) ReleaseCapture();
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_release(Kore_Windows_WindowIndexFromHWND(hWnd), 0, mouseX, mouseY);
		break;
	case WM_RBUTTONDOWN:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_press(Kore_Windows_WindowIndexFromHWND(hWnd), 1, mouseX, mouseY);
		break;
	case WM_RBUTTONUP:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_release(Kore_Windows_WindowIndexFromHWND(hWnd), 1, mouseX, mouseY);
		break;
	case WM_MBUTTONDOWN:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_press(Kore_Windows_WindowIndexFromHWND(hWnd), 2, mouseX, mouseY);
		break;
	case WM_MBUTTONUP:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_release(Kore_Windows_WindowIndexFromHWND(hWnd), 2, mouseX, mouseY);
		break;
	case WM_XBUTTONDOWN:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_press(Kore_Windows_WindowIndexFromHWND(hWnd), HIWORD(wParam) + 2, mouseX, mouseY);
		break;
	case WM_XBUTTONUP:
		mouseX = GET_X_LPARAM(lParam);
		mouseY = GET_Y_LPARAM(lParam);
		Kore::Mouse::the()->_release(Kore_Windows_WindowIndexFromHWND(hWnd), HIWORD(wParam) + 2, mouseX, mouseY);
		break;
	case WM_MOUSEWHEEL:
		Kore::Mouse::the()->_scroll(Kore_Windows_WindowIndexFromHWND(hWnd), GET_WHEEL_DELTA_WPARAM(wParam) / -120);
		break;
	case WM_POINTERDOWN:
		pointerId = GET_POINTERID_WPARAM(wParam);
		MyGetPointerInfo(pointerId, &pointerInfo);
		if (pointerInfo.pointerType == PT_PEN) {
			MyGetPointerPenInfo(pointerId, &penInfo);
			ScreenToClient(hWnd, &pointerInfo.ptPixelLocation);
			Kore::Pen::the()->_press(Kore_Windows_WindowIndexFromHWND(hWnd), pointerInfo.ptPixelLocation.x, pointerInfo.ptPixelLocation.y,
			                         float(penInfo.pressure) / 1024.0f);
		}
		break;
	case WM_POINTERUP:
		pointerId = GET_POINTERID_WPARAM(wParam);
		MyGetPointerInfo(pointerId, &pointerInfo);
		if (pointerInfo.pointerType == PT_PEN) {
			MyGetPointerPenInfo(pointerId, &penInfo);
			ScreenToClient(hWnd, &pointerInfo.ptPixelLocation);
			Kore::Pen::the()->_release(Kore_Windows_WindowIndexFromHWND(hWnd), pointerInfo.ptPixelLocation.x, pointerInfo.ptPixelLocation.y,
			                           float(penInfo.pressure) / 1024.0f);
		}
		break;
	case WM_POINTERUPDATE:
		pointerId = GET_POINTERID_WPARAM(wParam);
		MyGetPointerInfo(pointerId, &pointerInfo);
		if (pointerInfo.pointerType == PT_PEN) {
			MyGetPointerPenInfo(pointerId, &penInfo);
			ScreenToClient(hWnd, &pointerInfo.ptPixelLocation);
			Kore::Pen::the()->_move(Kore_Windows_WindowIndexFromHWND(hWnd), pointerInfo.ptPixelLocation.x, pointerInfo.ptPixelLocation.y,
			                        float(penInfo.pressure) / 1024.0f);
		}
		break;
	case WM_KEYDOWN:
	case WM_SYSKEYDOWN:
		if (!keyPressed[wParam]) {
			keyPressed[wParam] = true;

			if (keyTranslated[wParam] == Kore::KeyControl) {
				controlDown = true;
			}
			else {
				if (controlDown && keyTranslated[wParam] == Kore::KeyX) {
					char *text = Kore_Internal_CutCallback();
					if (text != nullptr) {
						wchar_t wtext[4096];
						MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, 4096);
						OpenClipboard(hWnd);
						EmptyClipboard();
						int size = (wcslen(wtext) + 1) * sizeof(wchar_t);
						HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, size);
						void *data = GlobalLock(handle);
						memcpy(data, wtext, size);
						GlobalUnlock(handle);
						SetClipboardData(CF_UNICODETEXT, handle);
						CloseClipboard();
					}
				}

				if (controlDown && keyTranslated[wParam] == Kore::KeyC) {
					char *text = Kore_Internal_CopyCallback();
					if (text != nullptr) {
						wchar_t wtext[4096];
						MultiByteToWideChar(CP_UTF8, 0, text, -1, wtext, 4096);
						OpenClipboard(hWnd);
						EmptyClipboard();
						int size = (wcslen(wtext) + 1) * sizeof(wchar_t);
						HANDLE handle = GlobalAlloc(GMEM_MOVEABLE, size);
						void *data = GlobalLock(handle);
						memcpy(data, wtext, size);
						GlobalUnlock(handle);
						SetClipboardData(CF_UNICODETEXT, handle);
						CloseClipboard();
					}
				}

				if (controlDown && keyTranslated[wParam] == Kore::KeyV) {
					if (IsClipboardFormatAvailable(CF_UNICODETEXT)) {
						OpenClipboard(hWnd);
						HANDLE handle = GetClipboardData(CF_UNICODETEXT);
						if (handle != nullptr) {
							wchar_t *wtext = (wchar_t *)GlobalLock(handle);
							if (wtext != nullptr) {
								char text[4096];
								WideCharToMultiByte(CP_UTF8, 0, wtext, -1, text, 4096, nullptr, nullptr);
								Kore_Internal_PasteCallback(text);
								GlobalUnlock(handle);
							}
						}
						CloseClipboard();
					}
				}
			}

			Kore::Keyboard::the()->_keydown(keyTranslated[wParam]);
		}
		break;
	case WM_KEYUP:
	case WM_SYSKEYUP:
		keyPressed[wParam] = false;

		if (keyTranslated[wParam] == Kore::KeyControl) {
			controlDown = false;
		}

		Kore::Keyboard::the()->_keyup(keyTranslated[wParam]);
		break;
	case WM_CHAR:
		switch (wParam) {
		case 0x08: // backspace
			break;
		case 0x0A: // linefeed
			Kore::Keyboard::the()->_keypress(L'\n');
			break;
		case 0x1B: // escape
			break;
		case 0x09: // tab
			Kore::Keyboard::the()->_keypress(L'\t');
			break;
		case 0x0D: // carriage return
			Kore::Keyboard::the()->_keypress(L'\r');
			break;
		default:
			Kore::Keyboard::the()->_keypress((wchar_t)wParam);
			break;
		}
		break;
	case WM_SYSCOMMAND:
		switch (wParam) {
		case SC_KEYMENU:
			return 0; // Prevent from happening
		case SC_SCREENSAVE:
		case SC_MONITORPOWER:
			return 0; // Prevent from happening

		// Pause game when window is minimized, continue when it's restored or maximized.
		//
		// Unfortunately, if the game would continue to run when minimized, the graphics in
		// the Windows Vista/7 taskbar would not be updated, even when Direct3DDevice::Present()
		// is called without error. I do not know why.
		case SC_MINIMIZE:
			// Scheduler::haltTime(); // haltTime()/unhaltTime() is incremental, meaning that this doesn't interfere with when the game itself calls these
			// functions
			break;
		case SC_RESTORE:
		case SC_MAXIMIZE:
			// Scheduler::unhaltTime();
			break;
		}
		break;
	case WM_DROPFILES:
		HDROP hDrop = (HDROP)wParam;
		unsigned count = DragQueryFile(hDrop, 0xFFFFFFFF, NULL, NULL);
		if (count == 1) { // Single file only for now
			wchar_t filePath[260];
			if (DragQueryFile(hDrop, 0, filePath, 260)) {
				Kore_Internal_DropFilesCallback(filePath);
			}
		}
		DragFinish(hDrop);
		break;
	}
	return DefWindowProc(hWnd, msg, wParam, lParam);
}

namespace {
	float axes[12 * 6];
	float buttons[12 * 16];

	typedef DWORD(WINAPI *XInputGetStateType)(DWORD dwUserIndex, XINPUT_STATE *pState);
	XInputGetStateType InputGetState = nullptr;
}

void loadXInput() {
	HMODULE lib = LoadLibrary(L"xinput1_4.dll");
	if (lib == nullptr) {
		lib = LoadLibrary(L"xinput1_3.dll");
	}
	if (lib == nullptr) {
		lib = LoadLibrary(L"xinput9_1_0.dll");
	}

	if (lib != nullptr) {
		InputGetState = (XInputGetStateType)GetProcAddress(lib, "XInputGetState");
	}
}

namespace {
	IDirectInput8 *di_instance = nullptr;
	IDirectInputDevice8 *di_pads[XUSER_MAX_COUNT];
	DIJOYSTATE2 di_padState[XUSER_MAX_COUNT];
	DIJOYSTATE2 di_lastPadState[XUSER_MAX_COUNT];
	DIDEVCAPS di_deviceCaps[XUSER_MAX_COUNT];
	int padCount = 0;

	void cleanupPad(int padIndex) {
		if (di_pads[padIndex] != nullptr) {
			di_pads[padIndex]->Unacquire();
			di_pads[padIndex]->Release();
			di_pads[padIndex] = 0;
		}
	}

#ifndef SAFE_RELEASE
#define SAFE_RELEASE(x)                                                                                                                                        \
	if (x != NULL) {                                                                                                                                           \
		x->Release();                                                                                                                                          \
		x = NULL;                                                                                                                                              \
	}
#endif

	// From
	//-----------------------------------------------------------------------------
	// Enum each PNP device using WMI and check each device ID to see if it contains
	// "IG_" (ex. "VID_045E&PID_028E&IG_00").  If it does, then it's an XInput device
	// Unfortunately this information can not be found by just using DirectInput
	//-----------------------------------------------------------------------------
	BOOL IsXInputDevice(const GUID *pGuidProductFromDirectInput) {
		IWbemLocator *pIWbemLocator = NULL;
		IEnumWbemClassObject *pEnumDevices = NULL;
		IWbemClassObject *pDevices[20] = {0};
		IWbemServices *pIWbemServices = NULL;
		BSTR bstrNamespace = NULL;
		BSTR bstrDeviceID = NULL;
		BSTR bstrClassName = NULL;
		DWORD uReturned = 0;
		bool bIsXinputDevice = false;
		UINT iDevice = 0;
		VARIANT var;
		HRESULT hr;

		// CoInit if needed
		hr = CoInitialize(NULL);
		bool bCleanupCOM = SUCCEEDED(hr);

		// Create WMI
		hr = CoCreateInstance(__uuidof(WbemLocator), NULL, CLSCTX_INPROC_SERVER, __uuidof(IWbemLocator), (LPVOID *)&pIWbemLocator);
		if (FAILED(hr) || pIWbemLocator == NULL) goto LCleanup;

		bstrNamespace = SysAllocString(L"\\\\.\\root\\cimv2");
		if (bstrNamespace == NULL) goto LCleanup;
		bstrClassName = SysAllocString(L"Win32_PNPEntity");
		if (bstrClassName == NULL) goto LCleanup;
		bstrDeviceID = SysAllocString(L"DeviceID");
		if (bstrDeviceID == NULL) goto LCleanup;

		// Connect to WMI
		hr = pIWbemLocator->ConnectServer(bstrNamespace, NULL, NULL, 0L, 0L, NULL, NULL, &pIWbemServices);
		if (FAILED(hr) || pIWbemServices == NULL) goto LCleanup;

		// Switch security level to IMPERSONATE.
		CoSetProxyBlanket(pIWbemServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

		hr = pIWbemServices->CreateInstanceEnum(bstrClassName, 0, NULL, &pEnumDevices);
		if (FAILED(hr) || pEnumDevices == NULL) goto LCleanup;

		// Loop over all devices
		for (;;) {
			// Get 20 at a time
			hr = pEnumDevices->Next(10000, 20, pDevices, &uReturned);
			if (FAILED(hr)) goto LCleanup;
			if (uReturned == 0) break;

			for (iDevice = 0; iDevice < uReturned; iDevice++) {
				// For each device, get its device ID
				hr = pDevices[iDevice]->Get(bstrDeviceID, 0L, &var, NULL, NULL);
				if (SUCCEEDED(hr) && var.vt == VT_BSTR && var.bstrVal != NULL) {
					// Check if the device ID contains "IG_".  If it does, then it's an XInput device
					// This information can not be found from DirectInput
					if (wcsstr(var.bstrVal, L"IG_")) {
						// If it does, then get the VID/PID from var.bstrVal
						DWORD dwPid = 0, dwVid = 0;
						WCHAR *strVid = wcsstr(var.bstrVal, L"VID_");
						if (strVid && swscanf(strVid, L"VID_%4X", &dwVid) != 1) dwVid = 0;
						WCHAR *strPid = wcsstr(var.bstrVal, L"PID_");
						if (strPid && swscanf(strPid, L"PID_%4X", &dwPid) != 1) dwPid = 0;

						// Compare the VID/PID to the DInput device
						DWORD dwVidPid = MAKELONG(dwVid, dwPid);
						if (dwVidPid == pGuidProductFromDirectInput->Data1) {
							bIsXinputDevice = true;
							goto LCleanup;
						}
					}
				}
				SAFE_RELEASE(pDevices[iDevice]);
			}
		}

	LCleanup:
		if (bstrNamespace) SysFreeString(bstrNamespace);
		if (bstrDeviceID) SysFreeString(bstrDeviceID);
		if (bstrClassName) SysFreeString(bstrClassName);
		for (iDevice = 0; iDevice < 20; iDevice++) SAFE_RELEASE(pDevices[iDevice]);
		SAFE_RELEASE(pEnumDevices);
		SAFE_RELEASE(pIWbemLocator);
		SAFE_RELEASE(pIWbemServices);

		if (bCleanupCOM) CoUninitialize();

		return bIsXinputDevice;
	}

	// TODO (DK) this should probably be called from somewhere?
	void cleanupDirectInput() {
		for (int padIndex = 0; padIndex < XUSER_MAX_COUNT; ++padIndex) {
			cleanupPad(padIndex);
		}

		if (di_instance != nullptr) {
			di_instance->Release();
			di_instance = nullptr;
		}
	}

	BOOL CALLBACK enumerateJoystickAxesCallback(LPCDIDEVICEOBJECTINSTANCEW ddoi, LPVOID context) {
		HWND hwnd = (HWND)context;

		DIPROPRANGE propertyRange;
		propertyRange.diph.dwSize = sizeof(DIPROPRANGE);
		propertyRange.diph.dwHeaderSize = sizeof(DIPROPHEADER);
		propertyRange.diph.dwHow = DIPH_BYID;
		propertyRange.diph.dwObj = ddoi->dwType;
		propertyRange.lMin = -32768;
		propertyRange.lMax = 32768;

		HRESULT hr = di_pads[padCount]->SetProperty(DIPROP_RANGE, &propertyRange.diph);

		if (FAILED(hr)) {
			Kore_Log(KORE_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / SetProperty() failed (HRESULT=0x%x)", padCount, hr);

			// TODO (DK) cleanup?
			// cleanupPad(padCount);

			return DIENUM_STOP;
		}

		return DIENUM_CONTINUE;
	}

	BOOL CALLBACK enumerateJoysticksCallback(LPCDIDEVICEINSTANCEW ddi, LPVOID context) {
		if (IsXInputDevice(&ddi->guidProduct)) return DIENUM_CONTINUE;

		HRESULT hr = di_instance->CreateDevice(ddi->guidInstance, &di_pads[padCount], nullptr);

		if (SUCCEEDED(hr)) {
			hr = di_pads[padCount]->SetDataFormat(&c_dfDIJoystick2);

			// TODO (DK) required?
			// hr = di_pads[padCount]->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND);

			if (SUCCEEDED(hr)) {
				di_deviceCaps[padCount].dwSize = sizeof(DIDEVCAPS);
				hr = di_pads[padCount]->GetCapabilities(&di_deviceCaps[padCount]);

				if (SUCCEEDED(hr)) {
					hr = di_pads[padCount]->EnumObjects(enumerateJoystickAxesCallback, nullptr, DIDFT_AXIS);

					if (SUCCEEDED(hr)) {
						hr = di_pads[padCount]->Acquire();

						if (SUCCEEDED(hr)) {
							memset(&di_padState[padCount], 0, sizeof(DIJOYSTATE2));
							hr = di_pads[padCount]->GetDeviceState(sizeof(DIJOYSTATE2), &di_padState[padCount]);

							if (SUCCEEDED(hr)) {
								Kore_Log(KORE_LOG_LEVEL_INFO, "DirectInput8 / Pad%i / initialized", padCount);
							}
							else {
								Kore_Log(KORE_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / GetDeviceState() failed (HRESULT=0x%x)", padCount, hr);
								// cleanupPad(padCount); // (DK) don't kill it, we try again in handleDirectInputPad()
							}
						}
						else {
							Kore_Log(KORE_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / Acquire() failed (HRESULT=0x%x)", padCount, hr);
							cleanupPad(padCount);
						}
					}
					else {
						Kore_Log(KORE_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / EnumObjects(DIDFT_AXIS) failed (HRESULT=0x%x)", padCount, hr);
						cleanupPad(padCount);
					}
				}
				else {
					Kore_Log(KORE_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / GetCapabilities() failed (HRESULT=0x%x)", padCount, hr);
					cleanupPad(padCount);
				}
			}
			else {
				Kore_Log(KORE_LOG_LEVEL_WARNING, "DirectInput8 / Pad%i / SetDataFormat() failed (HRESULT=0x%x)", padCount, hr);
				cleanupPad(padCount);
			}

			++padCount;

			if (padCount >= XUSER_MAX_COUNT) {
				return DIENUM_STOP;
			}
		}

		return DIENUM_CONTINUE;
	}
}

static void initializeDirectInput() {
	HINSTANCE hinstance = GetModuleHandle(nullptr);

	memset(&di_pads, 0, sizeof(IDirectInputDevice8) * XUSER_MAX_COUNT);
	memset(&di_padState, 0, sizeof(DIJOYSTATE2) * XUSER_MAX_COUNT);
	memset(&di_lastPadState, 0, sizeof(DIJOYSTATE2) * XUSER_MAX_COUNT);
	memset(&di_deviceCaps, 0, sizeof(DIDEVCAPS) * XUSER_MAX_COUNT);

	HRESULT hr = DirectInput8Create(hinstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void **)&di_instance, nullptr);

	if (SUCCEEDED(hr)) {
		hr = di_instance->EnumDevices(DI8DEVCLASS_GAMECTRL, enumerateJoysticksCallback, nullptr, DIEDFL_ATTACHEDONLY);

		if (SUCCEEDED(hr)) {
		}
		else {
			cleanupDirectInput();
		}
	}
	else {
		Kore_Log(KORE_LOG_LEVEL_WARNING, "DirectInput8Create failed (HRESULT=0x%x)", hr);
	}
}

void handleDirectInputPad(int padIndex) {
	if (di_pads[padIndex] == nullptr) {
		return;
	}

	// TODO (DK) code is copied from xinput stuff, why is it set every frame?
	Kore::Gamepad::get(padIndex)->vendor = "DirectInput8";         // TODO (DK) figure out how to get vendor name
	Kore::Gamepad::get(padIndex)->productName = "Generic Gamepad"; // TODO (DK) figure out how to get product name

	HRESULT hr = di_pads[padIndex]->GetDeviceState(sizeof(DIJOYSTATE2), &di_padState[padIndex]);

	switch (hr) {
	case S_OK: {
		if (Kore::Gamepad::get(padIndex)->Axis != nullptr) {
			// TODO (DK) there is a lot more to handle
			for (int axisIndex = 0; axisIndex < 2; ++axisIndex) {
				LONG *now = nullptr;
				LONG *last = nullptr;

				switch (axisIndex) {
				case 0: {
					now = &di_padState[padIndex].lX;
					last = &di_lastPadState[padIndex].lX;
				} break;
				case 1: {
					now = &di_padState[padIndex].lY;
					last = &di_lastPadState[padIndex].lY;
				} break;
				case 2: {
					now = &di_padState[padIndex].lZ;
					last = &di_lastPadState[padIndex].lZ;
				} break;
				}

				if (*now != *last) {
					Kore::Gamepad::get(padIndex)->Axis(axisIndex, *now / 32768.0f);
				}
			}

			if (Kore::Gamepad::get(padIndex)->Button != nullptr) {
				for (int buttonIndex = 0; buttonIndex < 128; ++buttonIndex) {
					BYTE *now = &di_padState[padIndex].rgbButtons[buttonIndex];
					BYTE *last = &di_lastPadState[padIndex].rgbButtons[buttonIndex];

					if (*now != *last) {
						Kore::Gamepad::get(padIndex)->Button(buttonIndex, *now / 255.0f);
					}
				}
			}
		}

		memcpy(&di_lastPadState[padIndex], &di_padState[padIndex], sizeof(DIJOYSTATE2));
		break;
	}
	case DIERR_INPUTLOST: // fall through
	case DIERR_NOTACQUIRED: {
		hr = di_pads[padIndex]->Acquire();
		break;
	}
	}
}

bool Kore_Internal_HandleMessages() {
	MSG message;

	while (PeekMessage(&message, 0, 0, 0, PM_REMOVE)) {
		TranslateMessage(&message);
		DispatchMessage(&message);
	}

	if (InputGetState != nullptr) {
		DWORD dwResult;
		for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i) {
			XINPUT_STATE state;
			ZeroMemory(&state, sizeof(XINPUT_STATE));
			dwResult = InputGetState(i, &state);

			if (dwResult == ERROR_SUCCESS) {
				Kore::Gamepad::get(i)->vendor = "Microsoft";
				Kore::Gamepad::get(i)->productName = "Xbox 360 Controller";

				float newaxes[6];
				newaxes[0] = state.Gamepad.sThumbLX / 32768.0f;
				newaxes[1] = state.Gamepad.sThumbLY / 32768.0f;
				newaxes[2] = state.Gamepad.sThumbRX / 32768.0f;
				newaxes[3] = state.Gamepad.sThumbRY / 32768.0f;
				newaxes[4] = state.Gamepad.bLeftTrigger / 255.0f;
				newaxes[5] = state.Gamepad.bRightTrigger / 255.0f;
				for (int i2 = 0; i2 < 6; ++i2) {
					if (axes[i * 6 + i2] != newaxes[i2]) {
						if (Kore::Gamepad::get(i)->Axis != nullptr) Kore::Gamepad::get(i)->Axis(i2, newaxes[i2]);
						axes[i * 6 + i2] = newaxes[i2];
					}
				}
				float newbuttons[16];
				newbuttons[0] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_A) ? 1.0f : 0.0f;
				newbuttons[1] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_B) ? 1.0f : 0.0f;
				newbuttons[2] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_X) ? 1.0f : 0.0f;
				newbuttons[3] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_Y) ? 1.0f : 0.0f;
				newbuttons[4] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER) ? 1.0f : 0.0f;
				newbuttons[5] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER) ? 1.0f : 0.0f;
				newbuttons[6] = state.Gamepad.bLeftTrigger / 255.0f;
				newbuttons[7] = state.Gamepad.bRightTrigger / 255.0f;
				newbuttons[8] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_BACK) ? 1.0f : 0.0f;
				newbuttons[9] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_START) ? 1.0f : 0.0f;
				newbuttons[10] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_LEFT_THUMB) ? 1.0f : 0.0f;
				newbuttons[11] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_RIGHT_THUMB) ? 1.0f : 0.0f;
				newbuttons[12] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_UP) ? 1.0f : 0.0f;
				newbuttons[13] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_DOWN) ? 1.0f : 0.0f;
				newbuttons[14] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_LEFT) ? 1.0f : 0.0f;
				newbuttons[15] = (state.Gamepad.wButtons & XINPUT_GAMEPAD_DPAD_RIGHT) ? 1.0f : 0.0f;
				for (int i2 = 0; i2 < 16; ++i2) {
					if (buttons[i * 16 + i2] != newbuttons[i2]) {
						if (Kore::Gamepad::get(i)->Button != nullptr) Kore::Gamepad::get(i)->Button(i2, newbuttons[i2]);
						buttons[i * 16 + i2] = newbuttons[i2];
					}
				}
			}

			handleDirectInputPad(i);
		}
	}

	return true;
}

//**vec2i Kore::System::mousePos() {
//**	return vec2i(mouseX, mouseY);
//**}

namespace {
	bool keyboardshown = false;
}

//**void Kore::System::showKeyboard() {
//**	keyboardshown = true;
//**}

//**void Kore::System::hideKeyboard() {
//**	keyboardshown = false;
//**}

//**bool Kore::System::showsKeyboard() {
//**	return keyboardshown;
//**}

void Kore_LoadURL(const char *url) {}

void Kore_SetKeepScreenOn(bool on) {}

const char *Kore_SystemId() {
	return "Windows";
}

namespace {
	wchar_t savePathw[2048] = {0};
	char savePath[2048] = {0};

	void findSavePath() {
		// CoInitialize(NULL);
		IKnownFolderManager *folders = nullptr;
		CoCreateInstance(CLSID_KnownFolderManager, nullptr, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&folders));
		IKnownFolder *folder = nullptr;
		folders->GetFolder(FOLDERID_SavedGames, &folder);

		LPWSTR path;
		folder->GetPath(0, &path);

		wcscpy(savePathw, path);
		wcscat(savePathw, L"\\");
		wchar_t name[1024];
		MultiByteToWideChar(CP_UTF8, 0, Kore_ApplicationName(), -1, name, 1024);
		wcscat(savePathw, name);
		wcscat(savePathw, L"\\");

		SHCreateDirectoryEx(nullptr, savePathw, nullptr);
		WideCharToMultiByte(CP_UTF8, 0, savePathw, -1, savePath, 1024, nullptr, nullptr);

		CoTaskMemFree(path);
		folder->Release();
		folders->Release();
		// CoUninitialize();
	}
}

const char *Kore_Internal_SavePath() {
	if (::savePath[0] == 0) findSavePath();
	return ::savePath;
}

namespace {
	const char *videoFormats[] = {"ogv", nullptr};
	LARGE_INTEGER frequency;
	LARGE_INTEGER startCount;
}

const char **Kore_VideoFormats() {
	return ::videoFormats;
}

double Kore_Frequency() {
	return (double)::frequency.QuadPart;
}

Kore_ticks Kore_Timestamp() {
	LARGE_INTEGER stamp;
	QueryPerformanceCounter(&stamp);
	return stamp.QuadPart - startCount.QuadPart;
}

double Kore_Time() {
	LARGE_INTEGER stamp;
	QueryPerformanceCounter(&stamp);
	return double(stamp.QuadPart - startCount.QuadPart) / (double)::frequency.QuadPart;
}

int WINAPI WinMain(HINSTANCE /*hInstance*/, HINSTANCE /*hPrevInstance*/, LPSTR lpCmdLine, int /*nCmdShow*/) {
	// Pen functions are only in Windows 8 and later, so load them dynamically
	HMODULE user32 = LoadLibraryA("user32.dll");
	MyGetPointerInfo = (GetPointerInfoType)GetProcAddress(user32, "GetPointerInfo");
	MyGetPointerPenInfo = (GetPointerPenInfoType)GetProcAddress(user32, "GetPointerPenInfo");
	MyEnableNonClientDpiScaling = (EnableNonClientDpiScalingType)GetProcAddress(user32, "EnableNonClientDpiScaling");
	initKeyTranslation();
	for (int i = 0; i < 256; ++i) keyPressed[i] = false;

	Kore_Windows_InitDisplays();

	QueryPerformanceCounter(&startCount);
	QueryPerformanceFrequency(&::frequency);

	int ret = 0;
#ifndef _DEBUG
	try {
#endif
		for (int i = 0; i < 256; ++i) keyPressed[i] = false;

		ret = kore(__argc, __argv);
#ifndef _DEBUG
	} catch (std::exception &ex) {
		ret = 1;
		MessageBoxA(0, ex.what(), "Exception", MB_OK);
	} catch (...) {
		ret = 1;
		MessageBox(0, L"Unknown Exception", L"Exception", MB_OK);
	}
#endif

	return ret;
}

int Kore_Init(const char *name, int width, int height, Kore_WindowOptions *win, Kore_FramebufferOptions *frame) {
	Kore_WindowOptions defaultWin;
	if (win == nullptr) {
		Kore_Internal_InitWindowOptions(&defaultWin);
		win = &defaultWin;
	}
	win->width = width;
	win->height = height;
	int window = Kore_WindowCreate(win, frame);
	loadXInput();
	initializeDirectInput();
	return window;
}

void Kore_Internal_Shutdown() {
	Kore_Windows_HideWindows();
	Kore_Internal_ShutdownCallback();
	Kore_Windows_DestroyWindows();
	Kore_Windows_RestoreDisplays();
}
