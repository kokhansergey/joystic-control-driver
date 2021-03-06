// Win32App.cpp : Defines the entry point for the application.
//

#include "stdafx.h"
#include "Win32App.h"
#include <windows.h>
#include <dinput.h>
#include <shellapi.h>


#define MAX_LOADSTRING 100
#define	WM_USER_SHELLICON WM_USER + 1
#define TIMER_ID 1
#define CURSOR_SPEED 10
#define SCROLL_SPEED 45
//use set properties
#define MIDDLE_VALUE 32767
#define MOVEMENT_DELTA 3000

// Global Variables:
HINSTANCE hInst;	// current instance
NOTIFYICONDATA nidApp;
HMENU hPopMenu;
TCHAR szTitle[MAX_LOADSTRING];					// The title bar text
TCHAR szWindowClass[MAX_LOADSTRING];			// the main window class name
TCHAR szApplicationToolTip[MAX_LOADSTRING];	    // the main window class name
BOOL bDisable = FALSE;							// keep application state
LPDIRECTINPUT8 di;
HRESULT hr;
LPDIRECTINPUTDEVICE8 joystick;
DIDEVCAPS capabilities;
DIJOYSTATE2 js;
int cursorJoystickSpeed = 5;
BOOL LMBClicked = false;
BOOL RMBClicked = false;
BOOL MMBClicked = false;

BOOL UKClicked = false;
BOOL DKClicked = false;
BOOL LKClicked = false;
BOOL RKClicked = false;
BOOL CKClicked = false;
BOOL SKClicked = false;


// Functions protos
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

BOOL CALLBACK       enumCallback(const DIDEVICEINSTANCE* instance, VOID* context);
HRESULT             JoyStickProp();
HRESULT             JoyStickPoll(DIJOYSTATE2 *js);
void                KeyboardUse();
void                MoveCursor(POINT p);
void                ProcessButtons(POINT p);
void                Scroll(POINT p);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_WIN32APP, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_WIN32APP));

    MSG msg;

    while (GetMessage(&msg, nullptr, 0, 0))
    {
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
    }

    return (int) msg.wParam;
}

ATOM MyRegisterClass(HINSTANCE hInstance)
{	
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_WIN32APP));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_WIN32APP);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));


    return RegisterClassExW(&wcex);
}

BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	HICON hMainIcon;

   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   hMainIcon = LoadIcon(hInstance, (LPCTSTR)MAKEINTRESOURCE(IDI_WIN32APP));

   nidApp.cbSize = sizeof(NOTIFYICONDATA); // sizeof the struct in bytes 
   nidApp.hWnd = (HWND)hWnd;              //handle of the window which will process this app. messages 
   nidApp.uID = IDI_WIN32APP;           //ID of the icon that willl appear in the system tray 
   nidApp.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP; //ORing of all the flags 
   nidApp.hIcon = hMainIcon; // handle of the Icon to be displayed, obtained from LoadIcon 
   nidApp.uCallbackMessage = WM_USER_SHELLICON;
   Shell_NotifyIcon(NIM_ADD, &nidApp);

   return TRUE;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	POINT lpClickPoint;

    switch (message)
    {
		case WM_CREATE:
		{
			//Create a DirectInput device
			if (FAILED(hr = DirectInput8Create(GetModuleHandle(NULL), DIRECTINPUT_VERSION,
				IID_IDirectInput8, (VOID**)&di, NULL))) {
				return hr;
			}

			// Look for the first simple joystick we can find.
			if (FAILED(hr = di->EnumDevices(DI8DEVCLASS_GAMECTRL, enumCallback,
				NULL, DIEDFL_ATTACHEDONLY))) {
				return hr;
			}
			// Make sure we got a joystick
			if (joystick == NULL) {
				MessageBox(NULL, TEXT("Joystick not found."), TEXT("Error!"),
					MB_ICONERROR | MB_OK);
				DestroyWindow(hWnd);
				break;
			}

			JoyStickProp();

			SetTimer(hWnd, TIMER_ID, 10, NULL);
			return TRUE;
		}
		case WM_USER_SHELLICON:
			// systray msg callback 
			switch (LOWORD(lParam))
			{
			case WM_RBUTTONDOWN:
				UINT uFlag = MF_BYPOSITION | MF_STRING;
				GetCursorPos(&lpClickPoint);
				hPopMenu = CreatePopupMenu();
				InsertMenu(hPopMenu, 0xFFFFFFFF, MF_BYPOSITION | MF_STRING, IDM_EXIT, _T("Exit"));

				SetForegroundWindow(hWnd);
				TrackPopupMenu(hPopMenu, TPM_LEFTALIGN | TPM_LEFTBUTTON | TPM_BOTTOMALIGN, lpClickPoint.x, lpClickPoint.y, 0, hWnd, NULL);
				return TRUE;
			}
			break;
		case WM_TIMER:
			if (!FAILED(JoyStickPoll(&js)))
			{
				POINT p;
				GetCursorPos(&p);
				
				if (js.lZ < 1000) {
					KeyboardUse();
					ProcessButtons(p);
				}
				else {
					Scroll(p);
					MoveCursor(p);
					ProcessButtons(p);
				}
			}
			else
			{
				KillTimer(hWnd, TIMER_ID);

				MessageBox(NULL, TEXT("Error reading input state."), TEXT("Error!"),
					MB_ICONERROR | MB_OK);
				DestroyWindow(hWnd);
			}
			break;
		case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
			case IDM_EXIT:
				Shell_NotifyIcon(NIM_DELETE, &nidApp);
				DestroyWindow(hWnd);
				break;
			default:
				return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
			break;
		case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
		case WM_DESTROY:
			PostQuitMessage(0);
			break;
		default:
			return DefWindowProc(hWnd, message, wParam, lParam);
	}
    return 0;
}

BOOL CALLBACK enumCallback(const DIDEVICEINSTANCE* instance, VOID* context)
{
    HRESULT hr;
    hr = di->CreateDevice(instance->guidInstance, &joystick, NULL);
    if (FAILED(hr)) {
        return DIENUM_CONTINUE;
    }
    return DIENUM_STOP;
}

HRESULT JoyStickProp()
{
    if (FAILED(hr = joystick->SetDataFormat(&c_dfDIJoystick2))) {
        return hr;
    }
    if (FAILED(hr = joystick->SetCooperativeLevel(NULL, DISCL_EXCLUSIVE | DISCL_FOREGROUND))) {
        return hr;
    }
    capabilities.dwSize = sizeof(DIDEVCAPS);
    if (FAILED(hr = joystick->GetCapabilities(&capabilities))) {
        return hr;
    }
    return S_OK;
}

HRESULT JoyStickPoll(DIJOYSTATE2 *js)
{
    HRESULT hr;

    if (joystick == NULL) {
        return S_OK;
    }

    // Poll the device to read the current state
    hr = joystick->Poll();
    if (FAILED(hr)) {
        hr = joystick->Acquire();
        while (hr == DIERR_INPUTLOST) {
            hr = joystick->Acquire();
        }

        if ((hr == DIERR_INVALIDPARAM) || (hr == DIERR_NOTINITIALIZED)) {
            return E_FAIL;
        }

        // If another application has control of this device, return successfully.
        // We'll just have to wait our turn to use the joystick.
        if (hr == DIERR_OTHERAPPHASPRIO) {
            return S_OK;
        }
    }

    // Get the input's device state
    if (FAILED(hr = joystick->GetDeviceState(sizeof(DIJOYSTATE2), js))) {
        return hr; // The device should have been acquired during the Poll()
    }

    return S_OK;
}

void MoveCursor(POINT p)
{
	SetCursorPos(p.x - ((MIDDLE_VALUE - js.lX) / MOVEMENT_DELTA), p.y - ((MIDDLE_VALUE - js.lY) / MOVEMENT_DELTA));
	
	//switch (js.rgdwPOV[0])
 //   {
 //   case 0:
 //       SetCursorPos(p.x, p.y - CURSOR_SPEED);
 //       break;
 //   case 4500:
 //       SetCursorPos(p.x + CURSOR_SPEED/2, p.y - CURSOR_SPEED/2);
 //       break;
 //   case 9000:
 //       SetCursorPos(p.x + CURSOR_SPEED, p.y);
 //       break;
 //   case 13500:
 //       SetCursorPos(p.x + CURSOR_SPEED/2, p.y + CURSOR_SPEED/2);
 //       break;
 //   case 18000:
 //       SetCursorPos(p.x, p.y + CURSOR_SPEED);
 //       break;
 //   case 22500:
 //       SetCursorPos(p.x - CURSOR_SPEED/2, p.y + CURSOR_SPEED/2);
 //       break;
 //   case 27000:
 //       SetCursorPos(p.x - CURSOR_SPEED, p.y);
 //       break;
 //   case 31500:
 //       SetCursorPos(p.x - CURSOR_SPEED/2, p.y - CURSOR_SPEED/2);
 //       break;
 //   }
}

void ProcessButtons(POINT p)
{
    if ((js.rgbButtons[0] & 0x80) && !LMBClicked) // click
    {
        mouse_event(MOUSEEVENTF_LEFTDOWN, p.x, p.y, NULL, NULL);
        LMBClicked = true;
    }
    if (js.rgbButtons[0] == 0 && LMBClicked)
    {
        mouse_event(MOUSEEVENTF_LEFTUP, p.x, p.y, NULL, NULL);
        LMBClicked = false;
    }

    if ((js.rgbButtons[3] & 0x80) && !RMBClicked) // 4
    {
        mouse_event(MOUSEEVENTF_RIGHTDOWN, p.x, p.y, NULL, NULL);
        RMBClicked = true;
    }
    if (js.rgbButtons[3] == 0 && RMBClicked)
    {
        mouse_event(MOUSEEVENTF_RIGHTUP, p.x, p.y, NULL, NULL);
        RMBClicked = false;
    }

	if (js.rgbButtons[1] & 0x80) // 2
	{
		keybd_event(VK_ESCAPE, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
	}
}

void KeyboardUse() 
{
	switch (js.rgdwPOV[0])
	{
	case 4500:
		keybd_event(VK_RIGHT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_UP, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		break;
   
	case 13500:
		keybd_event(VK_RIGHT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_DOWN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		break;
	case 22500:
		keybd_event(VK_LEFT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_DOWN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		break;
	case 31500:
		keybd_event(VK_LEFT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_UP, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		break;
	}

	if (js.lX > 40000 || js.rgdwPOV[0] == 9000)
	{
		keybd_event(VK_RIGHT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		keybd_event(VK_RIGHT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	if (js.lX < 20000 || js.rgdwPOV[0] == 27000)
	{
		// Simulate a key press
		keybd_event(VK_LEFT, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		// Simulate a key release
		keybd_event(VK_LEFT, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	if (js.lY > 40000 || js.rgdwPOV[0] == 18000)
	{
		// Simulate a key press
		keybd_event(VK_DOWN, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		// Simulate a key release
		keybd_event(VK_DOWN, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
	if (js.lY < 20000 || js.rgdwPOV[0] == 0)
	{
		// Simulate a key press
		keybd_event(VK_UP, 0x45, KEYEVENTF_EXTENDEDKEY | 0, 0);
		// Simulate a key release
		keybd_event(VK_UP, 0x45, KEYEVENTF_EXTENDEDKEY | KEYEVENTF_KEYUP, 0);
	}
}

void Scroll(POINT p)
{
    switch (js.rgdwPOV[0])
    {
        case 0:
            mouse_event(MOUSEEVENTF_WHEEL, p.x, p.y, SCROLL_SPEED, NULL); //up
            break;
        case 18000:
            mouse_event(MOUSEEVENTF_WHEEL, p.x, p.y, -SCROLL_SPEED, NULL); //down
            break;
    }
}
