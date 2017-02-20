// sktoolslib - common files for SK tools

// Copyright (C) 2017 Stefan Kueng

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
#include "RichStatusBar.h"
#include "GDIHelpers.h"

constexpr int icon_width = 24;
constexpr int border_width = 3;


CRichStatusBar::CRichStatusBar(HINSTANCE hInst)
    : CWindow(hInst)
    , m_hFont(nullptr)
    , m_tooltip(nullptr)
{
}


CRichStatusBar::~CRichStatusBar()
{
    DeleteObject(m_hFont);
}

bool CRichStatusBar::Init(HWND hParent)
{
    WNDCLASSEX wcx = { sizeof(WNDCLASSEX) };

    wcx.lpfnWndProc = CWindow::stWinMsgHandler;
    wcx.hInstance = hResource;
    wcx.lpszClassName = L"RichStatusBar_{226E35DD-FFAC-4D97-A040-B94AF5BE39EC}";
    wcx.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    if (RegisterWindow(&wcx))
    {
        if (CreateEx(0, WS_CHILD | WS_VISIBLE, hParent))
        {
            NONCLIENTMETRICS ncm;
            ncm.cbSize = sizeof(NONCLIENTMETRICS);
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0U);
            m_hFont = CreateFontIndirect(&ncm.lfStatusFont);


            // create the tooltip window
            m_tooltip = CreateWindowEx(NULL, TOOLTIPS_CLASS, NULL,
                                       WS_POPUP | TTS_NOPREFIX | TTS_ALWAYSTIP,
                                       CW_USEDEFAULT, CW_USEDEFAULT,
                                       CW_USEDEFAULT, CW_USEDEFAULT,
                                       *this, NULL, hResource,
                                       NULL);
            SendMessage(m_tooltip, TTM_SETMAXTIPWIDTH, 0, 600);
            return true;
        }
    }
    return false;
}

bool CRichStatusBar::SetPart(int index, CRichStatusBarItem item, bool redraw, bool replace)
{
    if (index >= (int)m_parts.size())
        return false;
    if (index < 0)
    {
        m_parts.push_back(item);
        m_partwidths.push_back({});
        index = (int)m_parts.size() - 1;
    }
    else if (replace)
    {
        m_parts[index] = item;
    }
    else
    {
        m_parts.insert(m_parts.begin() + index - 1, item);
        m_partwidths.insert(m_partwidths.begin() + index - 1, {});
    }
    CalcRequestedWidths(index);
    if (redraw)
    {
        CalcWidths();
        UpdateWindow(*this);
    }

    return true;
}

bool CRichStatusBar::SetPart(int index, const std::wstring & text, const std::wstring & shortText, const std::wstring & tooltip, int width, int align, bool fixedWidth, HICON icon, HICON collapsedIcon)
{
    CRichStatusBarItem part;
    part.text = text;
    part.shortText = shortText;
    part.tooltip = tooltip;
    part.width = width;
    part.align = align;
    part.fixedWidth = fixedWidth;
    part.icon = icon;
    part.collapsedIcon = collapsedIcon;
    return SetPart(index, part, false);
}

LRESULT CRichStatusBar::WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    switch (uMsg)
    {
        case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            RECT rect;
            GetClientRect(*this, &rect);
            GDIHelpers::FillSolidRect(hdc, &rect, GetSysColor(COLOR_3DFACE));
            auto oldFont = SelectObject(hdc, m_hFont);

            const auto faceClr = GetSysColor(COLOR_3DFACE);

            auto textFgc = GetSysColor(COLOR_WINDOWTEXT);
            auto textBgc = faceClr;


            RECT partRect = rect;
            int right = 0;
            for (decltype(m_parts.size()) i = 0; i < m_parts.size(); ++i)
            {
                partRect = rect;
                const auto& part = m_parts[i];
                partRect.left = right;
                partRect.right = partRect.left + m_partwidths[i].calculatedWidth;
                right = partRect.right;
                DrawEdge(hdc, &partRect, BDR_SUNKENOUTER, BF_RECT | BF_ADJUST);
                int x = 0;
                if (part.icon && !m_partwidths[i].collapsed)
                {
                    auto cy = partRect.bottom - partRect.top;
                    DrawIconEx(hdc, partRect.left + 2, partRect.top, part.icon, 0, 0, 0, 0, DI_NORMAL);
                    x = 2 + cy;
                }
                partRect.left += x;
                SetTextColor(hdc, textFgc);
                SetBkColor(hdc, textBgc);
                RECT temprect = partRect;
                InflateRect(&temprect, -2, 0);
                if (!m_partwidths[i].collapsed || !part.collapsedIcon)
                {
                    auto text = m_partwidths[i].shortened && !part.shortText.empty() ? part.shortText : part.text;
                    UINT format = DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER;
                    if (part.align == 1)
                        format |= DT_CENTER;
                    if (part.align == 2)
                        format |= DT_RIGHT;
                    DrawText(hdc, text.c_str(), -1, &temprect, format);
                }
                else
                {
                    auto cy = temprect.bottom - temprect.top;
                    DrawIconEx(hdc, temprect.left + 2, temprect.top, part.icon, 0, 0, 0, 0, DI_NORMAL);
                    x = 2 + cy;
                }
            }

            SelectObject(hdc, oldFont);
            EndPaint(hwnd, &ps);
            return 0;
        }
        break;
        case WM_ERASEBKGND:
        return TRUE;
        case WM_SIZE:
        {
            CalcWidths();
        }
        break;
    }
    if (prevWndProc)
        return prevWndProc(hwnd, uMsg, wParam, lParam);
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

void CRichStatusBar::CalcRequestedWidths(int index)
{
    auto hdc = GetDC(*this);
    auto oldFont = SelectObject(hdc, m_hFont);
    RECT rect = { 0 };
    GetClientRect(*this, &rect);
    auto& part = m_parts[index];

    PartWidths w;
    if ((part.fixedWidth) && (part.width > 0))
    {
        w.calculatedWidth = 0;
        w.shortened = false;
        w.collapsed = false;
        w.canCollapse = part.collapsedIcon != nullptr;
        w.defaultWidth = part.width;
        w.shortWidth = part.width;
        w.fixed = part.fixedWidth;
    }
    else
    {
        w.calculatedWidth = 0;
        w.collapsed = false;
        w.canCollapse = part.collapsedIcon != nullptr;
        w.shortened = false;
        w.fixed = part.fixedWidth;

        RECT rc = rect;
        DrawText(hdc, part.shortText.c_str(), -1, &rc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
        w.shortWidth = rc.right - rc.left;
        rc = rect;
        DrawText(hdc, part.text.c_str(), -1, &rc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
        w.defaultWidth = rc.right - rc.left;
        if (part.icon)
        {
            w.defaultWidth += icon_width;
            w.shortWidth += icon_width;
        }
        w.shortWidth += (2 * border_width);
        w.defaultWidth += (2 * border_width);
    }
    m_partwidths[index] = w;
    SelectObject(hdc, oldFont);
    ReleaseDC(*this, hdc);
}

void CRichStatusBar::CalcWidths()
{
    if (m_partwidths.empty())
        return;

    for (auto& p : m_partwidths)
    {
        p.calculatedWidth = 0;
        p.collapsed = false;
        p.shortened = false;
    }

    RECT rect;
    GetClientRect(*this, &rect);
    int maxWidth = rect.right - rect.left;
    maxWidth -= (2 * border_width);
    bool bAdjusted = false;
    do
    {
        bAdjusted = false;
        int total = 0;
        int nonFixed = 0;
        for (auto& p : m_partwidths)
        {
            if (p.calculatedWidth == 0)
                p.calculatedWidth = p.defaultWidth;
            if (p.shortened)
                p.calculatedWidth = p.shortWidth;
            if (p.collapsed)
                p.calculatedWidth = icon_width + (2 * border_width);
            total += p.calculatedWidth;
            if (!p.fixed)
                ++nonFixed;
        }
        if ((total < maxWidth) && nonFixed)
        {
            int ext = (maxWidth - total) / nonFixed;
            int tWidth = 0;
            PartWidths * pPart = nullptr;
            for (auto& p : m_partwidths)
            {
                if (!p.fixed)
                {
                    p.calculatedWidth += ext;
                    pPart = &p;
                }
                tWidth += p.calculatedWidth;
            }
            if (pPart)
                pPart->calculatedWidth += (maxWidth - tWidth);
            break;
        }
        else
        {
            for (auto it = m_partwidths.rbegin(); it != m_partwidths.rend(); ++it)
            {
                if (!it->fixed)
                {
                    if (!it->shortened)
                    {
                        it->shortened = true;
                        bAdjusted = true;
                        break;
                    }
                }
            }
            if (!bAdjusted)
            {
                for (auto it = m_partwidths.rbegin(); it != m_partwidths.rend(); ++it)
                {
                    if (!it->fixed)
                    {
                        if (!it->collapsed && it->canCollapse)
                        {
                            it->collapsed = true;
                            bAdjusted = true;
                            break;
                        }
                    }
                }
            }
        }
    } while (bAdjusted);

    // set the tooltips
    TOOLINFO ti = { sizeof(TOOLINFO) };
    ti.hinst = hResource;
    ti.uFlags = TTF_SUBCLASS;
    ti.hwnd = *this;
    // first remove all tools
    for (decltype(m_parts.size()) i = 0; i < m_parts.size(); ++i)
    {
        ti.uId = i + 1;
        SendMessage(m_tooltip, TTM_DELTOOL, 0, (LPARAM)&ti);
    }
    // now add all tools
    int startx = 0;
    ti.rect.top = rect.top;
    ti.rect.bottom = rect.bottom;
    for (decltype(m_parts.size()) i = 0; i < m_parts.size(); ++i)
    {
        ti.uId = i + 1;
        ti.rect.left = startx;
        ti.rect.right = startx + m_partwidths[i].calculatedWidth;
        startx = ti.rect.right;
        InflateRect(&ti.rect, -2, 0);
        ti.lpszText = const_cast<wchar_t*>(m_parts[i].tooltip.c_str());
        if (ti.lpszText[0])
        {
            SendMessage(m_tooltip, TTM_ADDTOOL, 0, (LPARAM)&ti);
            SendMessage(m_tooltip, TTM_ACTIVATE, TRUE, (LPARAM)0);
        }
    }
    SetWindowPos(m_tooltip, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
    InvalidateRect(*this, nullptr, FALSE);
}
