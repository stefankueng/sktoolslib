// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013 - Stefan Kueng

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
#include "GDIHelpers.h"

enum
{
    AlphaShift = 24,
    RedShift = 16,
    GreenShift = 8,
    BlueShift = 0
};

COLORREF GDIHelpers::Darker(COLORREF crBase, float fFactor)
{
    ASSERT(fFactor < 1.0f && fFactor > 0.0f);

    fFactor = min(fFactor, 1.0f);
    fFactor = max(fFactor, 0.0f);

    const BYTE bRed = GetRValue(crBase);
    const BYTE bBlue = GetBValue(crBase);
    const BYTE bGreen = GetGValue(crBase);

    const BYTE bRedShadow = (BYTE)(bRed * fFactor);
    const BYTE bBlueShadow = (BYTE)(bBlue * fFactor);
    const BYTE bGreenShadow = (BYTE)(bGreen * fFactor);

    return RGB(bRedShadow, bGreenShadow, bBlueShadow);
}

COLORREF GDIHelpers::Lighter(COLORREF crBase, float fFactor)
{
    ASSERT(fFactor > 1.0f);

    fFactor = max(fFactor, 1.0f);

    const BYTE bRed = GetRValue(crBase);
    const BYTE bBlue = GetBValue(crBase);
    const BYTE bGreen = GetGValue(crBase);

    const BYTE bRedHilite = (BYTE)min((int)(bRed * fFactor), 255);
    const BYTE bBlueHilite = (BYTE)min((int)(bBlue * fFactor), 255);
    const BYTE bGreenHilite = (BYTE)min((int)(bGreen * fFactor), 255);

    return RGB(bRedHilite, bGreenHilite, bBlueHilite);
}

void GDIHelpers::FillSolidRect(HDC hDC, int left, int top, int right, int bottom, COLORREF clr)
{
    ::SetBkColor(hDC, clr);
    RECT rect;
    rect.left = left;
    rect.top = top;
    rect.right = right;
    rect.bottom = bottom;
    ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, &rect, nullptr, 0, nullptr);
}

void GDIHelpers::FillSolidRect(HDC hDC, const RECT* rc, COLORREF clr)
{
    ::SetBkColor(hDC, clr);
    ::ExtTextOut(hDC, 0, 0, ETO_OPAQUE, rc, nullptr, 0, nullptr);
}

Gdiplus::ARGB GDIHelpers::MakeARGB(IN BYTE a, IN BYTE r, IN BYTE g, IN BYTE b)
{
    return (((Gdiplus::ARGB)(b) << BlueShift) |
            ((Gdiplus::ARGB)(g) << GreenShift) |
            ((Gdiplus::ARGB)(r) << RedShift) |
            ((Gdiplus::ARGB)(a) << AlphaShift));
}

