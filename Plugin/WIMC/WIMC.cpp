﻿#include "stdafx.h"
#include "WIMC.h"
#include <CDEvents.h>
#include <CDAPI.h>
#include "Config.h"
#define _USE_MATH_DEFINES
#include <math.h>
#include <random>


WIMC::WIMC(HMODULE hModule) : 
	m_module(hModule)
{
	m_fakeCursors.resize(g_config.m_nCursors);

	cd::g_postDrawIconEvent.AddListener(std::bind(&WIMC::OnPostDrawIcon, this, std::placeholders::_1), m_module);
	cd::g_fileListWndProcEvent.AddListener([](UINT message, WPARAM, LPARAM, LRESULT& res){
		if (message == WM_MOUSEMOVE)
			cd::RedrawDesktop();
		return true;
	}, m_module);
	
	cd::RedrawDesktop();
}

bool WIMC::OnPostDrawIcon(HDC& hdc)
{
	CURSORINFO info;
	info.cbSize = sizeof(info);
	if (!GetCursorInfo(&info))
		return true;

	const auto& pos = info.ptScreenPos;
	const float distance = sqrtf(float((pos.x - m_cursorOrigin.x) * (pos.x - m_cursorOrigin.x) 
		+ (pos.y - m_cursorOrigin.y) * (pos.y - m_cursorOrigin.y)));
	const float angle = atan2f(float(pos.y - m_cursorOrigin.y), float(pos.x - m_cursorOrigin.x));

	const int width = GetSystemMetrics(SM_CXCURSOR);
	const int height = GetSystemMetrics(SM_CYCURSOR);

	for (auto& i : m_fakeCursors)
		i.Draw(hdc, info.hCursor, distance, angle, width, height);

	return true;
}


namespace
{
	float GetRandomFloat(float min, float max)
	{
		static std::random_device s_randomDevice;
		return std::uniform_real_distribution<float>(min, max)(s_randomDevice);
	}
}

WIMC::FakeCursor::FakeCursor()
{
	SIZE size;
	cd::GetDesktopSize(size);
	origin.x = static_cast<int>(GetRandomFloat(-size.cx * 0.75f, size.cx * 1.75f));
	origin.y = static_cast<int>(GetRandomFloat(-size.cy * 0.75f, size.cy * 1.75f));
	angle = GetRandomFloat(0.0f, 2.0f * float(M_PI));
}

void WIMC::FakeCursor::Draw(HDC hdc, HCURSOR cursor, float cursorDistance, float cursorAngle, int width, int height)
{
	const auto x = int(origin.x + cursorDistance * cosf(cursorAngle + angle));
	const auto y = int(origin.y + cursorDistance * sinf(cursorAngle + angle));
	// 没有对准热点，不过没什么影响
	DrawIconEx(hdc, x, y, cursor, width, height, 0, NULL, DI_NORMAL);
}
