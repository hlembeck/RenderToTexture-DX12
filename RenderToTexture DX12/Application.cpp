#include "Application.h"

int Application::Run(HINSTANCE hInstance, int nCmdShow, UINT width, UINT height, DXInstance* pInstance) {

#if defined(_DEBUG)
	ComPtr<ID3D12Debug> debugInterface;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugInterface)));
	debugInterface->EnableDebugLayer();
#endif

	HCURSOR hCursor = LoadCursor(nullptr, IDC_ARROW);
	WNDCLASSEXW wndClass = {
		sizeof(WNDCLASSEX),
		CS_HREDRAW | CS_VREDRAW,
		WndProc,
		0,
		0,
		hInstance,
		NULL,
		NULL,
		(HBRUSH)(COLOR_WINDOW + 1),
		NULL,
		wndClassName,
		NULL
	};

	if (!RegisterClassExW(&wndClass)) {
		return 1;
	}

	RECT rect = { 200,100,width,height };
	AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, FALSE);

	m_hWnd = CreateWindowExW(
		0,
		wndClassName,
		wndClassName,
		WS_OVERLAPPEDWINDOW,
		rect.left,
		rect.top,
		rect.right - rect.left,
		rect.bottom - rect.top,
		NULL,
		NULL,
		hInstance,
		pInstance
	);

	if (!m_hWnd) {
		return 2;
	}

	pInstance->OnInit();

	ShowWindow(m_hWnd, nCmdShow);

	MSG msg = { 0 };
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, 0, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	pInstance->OnDestroy();
	return 0;
}

HWND Application::GetHWND() { return m_hWnd; }

LRESULT CALLBACK Application::WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	DXInstance* pInstance = reinterpret_cast<DXInstance*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	switch (msg) {
	case WM_CREATE:
	{
		LPCREATESTRUCT pCreateStruct = reinterpret_cast<LPCREATESTRUCT>(lParam);
		SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pCreateStruct->lpCreateParams));
		pInstance = reinterpret_cast<DXInstance*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
	}
	break;
	case WM_PAINT:
		pInstance->OnUpdate();
		pInstance->OnRender();
		break;
	case WM_SIZE:
		break;
	case WM_DESTROY:
		PostQuitMessage(0);
		break;
	default:
		return DefWindowProc(hWnd, msg, wParam, lParam);
	}
	return 0;
}