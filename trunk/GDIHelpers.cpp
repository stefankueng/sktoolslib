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

void GDIHelpers::RGBToHSB(COLORREF rgb, BYTE& hue, BYTE& saturation, BYTE& brightness)
{
    BYTE r = GetRValue(rgb);
    BYTE g = GetGValue(rgb);
    BYTE b = GetBValue(rgb);
    BYTE minRGB = min(min(r, g), b);
    BYTE maxRGB = max(max(r, g), b);
    BYTE delta = maxRGB - minRGB;
    double l = double(maxRGB);
    double s = 0.0;
    double h = 0.0;
    if (maxRGB)
        s = (255.0 * delta) / maxRGB;

    if (s != 0.0)
    {
        if (r == maxRGB)
            h = double(g - b) / delta;
        else if (g == maxRGB)
            h = 2.0 + double(b - r) / delta;
        else if (b == maxRGB)
            h = 4.0 + double(r - g) / delta;
    }
    else
        h = -1.0;
    h = h * 60.0;
    if (h < 0.0)
        h = h + 360.0;

    hue = BYTE(h);
    saturation = BYTE(s * 100.0 / 255.0);
    brightness = BYTE(l * 100.0 / 255.0);
}

void GDIHelpers::RGBtoHSL(COLORREF color, float& h, float& s, float& l)
{
    const float r_percent = float(GetRValue(color)) / 255;
    const float g_percent = float(GetGValue(color)) / 255;
    const float b_percent = float(GetBValue(color)) / 255;

    float max_color = 0;
    if ((r_percent >= g_percent) && (r_percent >= b_percent))
        max_color = r_percent;
    else if ((g_percent >= r_percent) && (g_percent >= b_percent))
        max_color = g_percent;
    else if ((b_percent >= r_percent) && (b_percent >= g_percent))
        max_color = b_percent;

    float min_color = 0;
    if ((r_percent <= g_percent) && (r_percent <= b_percent))
        min_color = r_percent;
    else if ((g_percent <= r_percent) && (g_percent <= b_percent))
        min_color = g_percent;
    else if ((b_percent <= r_percent) && (b_percent <= g_percent))
        min_color = b_percent;

    float L = 0, S = 0, H = 0;

    L = (max_color + min_color) / 2;

    if (max_color == min_color)
    {
        S = 0;
        H = 0;
    }
    else
    {
        auto d = max_color - min_color;
        if (L < .50)
            S = d / (max_color + min_color);
        else
            S = d / ((2.0f - max_color) - min_color);

        if (max_color == r_percent)
            H = (g_percent - b_percent) / d;

        else if (max_color == g_percent)
            H = 2.0f + (b_percent - r_percent) / d;

        else if (max_color == b_percent)
            H = 4.0f + (r_percent - g_percent) / d;
    }
    H = H * 60;
    if (H < 0)
        H += 360;
    s = S * 100;
    l = L * 100;
    h = H;
}

static float HSLtoRGB_Subfunction(float temp1, float temp2, float temp3)
{
    if ((temp3 * 6) < 1)
        return (temp2 + (temp1 - temp2) * 6 * temp3) * 100;
    else if ((temp3 * 2) < 1)
        return temp1 * 100;
    else if ((temp3 * 3) < 2)
        return (temp2 + (temp1 - temp2)*(.66666f - temp3) * 6) * 100;
    else
        return temp2 * 100;
}

COLORREF GDIHelpers::HSLtoRGB(float h, float s, float l)
{
    if (s == 0)
    {
        BYTE t = BYTE(l / 100 * 255);
        return RGB(t, t, t);
    }
    const float L = l / 100;
    const float S = s / 100;
    const float H = h / 360;
    const float temp1 = (L < .50) ? L*(1 + S) : L + S - (L*S);
    const float temp2 = 2 * L - temp1;
    float temp3 = 0;
    temp3 = H + .33333f;
    if (temp3 > 1)
        temp3 -= 1;
    const float pcr = HSLtoRGB_Subfunction(temp1, temp2, temp3);
    temp3 = H;
    const float pcg = HSLtoRGB_Subfunction(temp1, temp2, temp3);
    temp3 = H - .33333f;
    if (temp3 < 0)
        temp3 += 1;
    const float pcb = HSLtoRGB_Subfunction(temp1, temp2, temp3);
    BYTE r = BYTE(pcr / 100 * 255);
    BYTE g = BYTE(pcg / 100 * 255);
    BYTE b = BYTE(pcb / 100 * 255);
    return RGB(r, g, b);
}
