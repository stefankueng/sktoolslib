﻿// sktoolslib - common files for SK tools

// Copyright (C) 2024 - Stefan Kueng

// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.

// You should have received a copy of the GNU General Public License
// along with this program; if not, write to the Free Software Foundation,
// 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
//
#include "stdafx.h"
#include "AutoCloakWindow.h"

#include <dwmapi.h>

CAutoCloakWindow::CAutoCloakWindow(HWND hWnd)
: m_hWnd(hWnd)
{
    BOOL cloak = TRUE;
    DwmSetWindowAttribute(m_hWnd, DWMWA_CLOAK, &cloak, sizeof(cloak)); // hide the window until we're ready
}

CAutoCloakWindow::~CAutoCloakWindow()
{
    BOOL cloak = FALSE;
    DwmSetWindowAttribute(m_hWnd, DWMWA_CLOAK, &cloak, sizeof(cloak)); // show the window again
}
