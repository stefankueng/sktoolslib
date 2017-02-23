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
#include <deque>

constexpr int icon_width = 24;
constexpr int border_width = 3;


CRichStatusBar::CRichStatusBar(HINSTANCE hInst)
    : CWindow(hInst)
    , m_fonts{ nullptr }
    , m_tooltip(nullptr)
    , m_ThemeColorFunc(nullptr)
    , m_hoverPart(-1)
    , m_height(22)
{
}


CRichStatusBar::~CRichStatusBar()
{
    for (auto & font : m_fonts)
        DeleteObject(font);
}

bool CRichStatusBar::Init(HWND hParent)
{
    WNDCLASSEX wcx = { sizeof(WNDCLASSEX) };

    wcx.lpfnWndProc = CWindow::stWinMsgHandler;
    wcx.hInstance = hResource;
    wcx.lpszClassName = L"RichStatusBar_{226E35DD-FFAC-4D97-A040-B94AF5BE39EC}";
    wcx.hbrBackground = (HBRUSH)(COLOR_3DFACE + 1);
    wcx.hCursor = ::LoadCursor(NULL, IDC_ARROW);
    if (RegisterWindow(&wcx))
    {
        if (CreateEx(0, WS_CHILD | WS_VISIBLE, hParent))
        {
            NONCLIENTMETRICS ncm;
            ncm.cbSize = sizeof(NONCLIENTMETRICS);
            SystemParametersInfo(SPI_GETNONCLIENTMETRICS, sizeof(NONCLIENTMETRICS), &ncm, 0U);
            m_fonts[0] = CreateFontIndirect(&ncm.lfStatusFont);
            ncm.lfStatusFont.lfItalic = TRUE;
            m_fonts[1] = CreateFontIndirect(&ncm.lfStatusFont);
            ncm.lfStatusFont.lfItalic = FALSE;
            ncm.lfStatusFont.lfWeight = FW_BOLD;
            m_fonts[2] = CreateFontIndirect(&ncm.lfStatusFont);
            ncm.lfStatusFont.lfItalic = TRUE;
            ncm.lfStatusFont.lfWeight = FW_BOLD;
            m_fonts[3] = CreateFontIndirect(&ncm.lfStatusFont);

            // calculate the height of the status bar from the font size
            RECT fr;
            auto hdc = GetDC(*this);
            DrawText(hdc, L"W", 1, &fr, DT_SINGLELINE | DT_CALCRECT);
            ReleaseDC(*this, hdc);
            m_height = fr.bottom - fr.top;
            m_height += 4;

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

bool CRichStatusBar::SetPart(int index, const CRichStatusBarItem& item, bool redraw, bool replace)
{
    if (index >= (int)m_parts.size())
    {
        for (auto i = m_parts.size(); i <= index; ++i)
        {
            m_parts.push_back(std::move(CRichStatusBarItem()));
            m_partwidths.push_back({});
        }
    }
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

bool CRichStatusBar::SetPart(int index, const std::wstring & text, const std::wstring & shortText, const std::wstring & tooltip, int width, int shortWidth, int align, bool fixedWidth, bool hover, HICON icon, HICON collapsedIcon)
{
    CRichStatusBarItem part;
    part.text = text;
    part.shortText = shortText;
    part.tooltip = tooltip;
    part.width = width;
    part.shortWidth = shortWidth;
    part.align = align;
    part.fixedWidth = fixedWidth;
    part.hoverActive = hover;
    part.icon = icon;
    part.collapsedIcon = collapsedIcon;
    return SetPart(index, part, false);
}

int CRichStatusBar::GetPartIndexAt(const POINT & pt) const
{
    RECT rect;
    GetClientRect(*this, &rect);
    int width = 0;
    for (size_t i = 0; i < m_partwidths.size(); ++i)
    {
        RECT rc = rect;
        rc.left = width;
        rc.right = rc.left + m_partwidths[i].calculatedWidth;
        if (PtInRect(&rc, pt))
        {
            return (int)i;
            break;
        }
        width += m_partwidths[i].calculatedWidth;
    }
    return -1;
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
            GDIHelpers::FillSolidRect(hdc, &rect, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_3DFACE)) : GetSysColor(COLOR_3DFACE));

            SetTextColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_WINDOWTEXT)) : GetSysColor(COLOR_WINDOWTEXT));
            SetBkColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_3DFACE)) : GetSysColor(COLOR_3DFACE));


            RECT partRect = rect;
            int right = 0;
            for (decltype(m_parts.size()) i = 0; i < m_parts.size(); ++i)
            {
                partRect = rect;
                const auto& part = m_parts[i];
                partRect.left = right;
                partRect.right = partRect.left + m_partwidths[i].calculatedWidth;
                right = partRect.right;
                DrawEdge(hdc, &partRect, i == m_hoverPart ? EDGE_ETCHED : BDR_SUNKENOUTER, BF_RECT | BF_ADJUST);
                int x = 0;
                if (part.icon && !m_partwidths[i].collapsed)
                {
                    auto cy = partRect.bottom - partRect.top;
                    DrawIconEx(hdc, partRect.left + 2, partRect.top, part.icon, 0, 0, 0, 0, DI_NORMAL);
                    x = 2 + cy;
                }
                partRect.left += x;
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
                    DrawRichText(hdc, text, temprect, format);
                }
                else
                {
                    auto cy = temprect.bottom - temprect.top;
                    DrawIconEx(hdc, temprect.left + 2, temprect.top, part.icon, 0, 0, 0, 0, DI_NORMAL);
                    x = 2 + cy;
                }
            }

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
        case WM_LBUTTONDBLCLK:
        case WM_LBUTTONUP:
        case WM_LBUTTONDOWN:
        case WM_CONTEXTMENU:
        {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            if (uMsg == WM_CONTEXTMENU)
                ScreenToClient(*this, &pt);
            auto index = GetPartIndexAt(pt);
            if (index >= 0)
                SendMessage(::GetParent(*this), WM_STATUSBAR_MSG, uMsg, index);
        }
        break;
        case WM_MOUSEMOVE:
        {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE;
            tme.hwndTrack = *this;
            TrackMouseEvent(&tme);

            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            auto oldHover = m_hoverPart;
            m_hoverPart = GetPartIndexAt(pt);
            if ((m_hoverPart != oldHover) && (m_parts[m_hoverPart].hoverActive))
                InvalidateRect(hwnd, nullptr, FALSE);
            else
                m_hoverPart = -1;
        }
        break;
        case WM_MOUSELEAVE:
        {
            TRACKMOUSEEVENT tme = { 0 };
            tme.cbSize = sizeof(TRACKMOUSEEVENT);
            tme.dwFlags = TME_LEAVE | TME_CANCEL;
            tme.hwndTrack = *this;
            TrackMouseEvent(&tme);
            if (m_hoverPart >= 0)
                InvalidateRect(hwnd, nullptr, FALSE);
            m_hoverPart = -1;
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
    RECT rect = { 0 };
    GetClientRect(*this, &rect);
    auto& part = m_parts[index];

    PartWidths w;
    w.calculatedWidth = 0;
    w.collapsed = false;
    w.canCollapse = part.collapsedIcon != nullptr;
    w.shortened = false;
    w.fixed = part.fixedWidth;

    if (part.shortWidth > 0)
        w.shortWidth = part.shortWidth;
    else
    {
        RECT rc = rect;
        DrawRichText(hdc, part.shortText.empty() ? part.text : part.shortText, rc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
        w.shortWidth = rc.right - rc.left;
    }
    if (part.width > 0)
        w.defaultWidth = part.width;
    else
    {
        RECT rc = rect;
        DrawRichText(hdc, part.text, rc, DT_CALCRECT | DT_NOPREFIX | DT_SINGLELINE | DT_VCENTER);
        w.defaultWidth = rc.right - rc.left;
    }
    if (part.icon)
    {
        w.defaultWidth += icon_width;
        w.shortWidth += icon_width;
    }
    w.shortWidth += (2 * border_width);
    w.defaultWidth += (2 * border_width);
    // add padding
    if (part.width < 0)
        w.defaultWidth -= part.width;
    if (part.shortWidth < 0)
        w.shortWidth -= part.shortWidth;
    m_partwidths[index] = w;
    ReleaseDC(*this, hdc);
}

void CRichStatusBar::DrawRichText(HDC hdc, const std::wstring & text, RECT & rect, UINT flags)
{
    struct TextControls
    {
        int             xPos = 0;
        std::wstring    text;
        COLORREF        color = (COLORREF)-1;
        HFONT           font = nullptr;
        wchar_t         command = '\0';
    };

    std::list<HGDIOBJ> objStack;
    int font = 0;
    auto oldFont = SelectObject(hdc, m_fonts[font]);

    SetTextColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_WINDOWTEXT)) : GetSysColor(COLOR_WINDOWTEXT));
    SetBkColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_3DFACE)) : GetSysColor(COLOR_3DFACE));

    size_t pos = 0;
    auto percPos = text.find('%', pos);
    int textWidth = 0;
    std::deque<TextControls> tokens;
    TextControls textControls;
    while (percPos != std::wstring::npos)
    {
        if (percPos < text.size() - 1)
        {
            RECT temprc = rect;
            textControls.text = text.substr(pos, percPos - pos);
            DrawText(hdc, textControls.text.c_str(), -1, &temprc, flags | DT_CALCRECT);
            textControls.xPos = textWidth;
            textWidth += (temprc.right - temprc.left);
            tokens.push_back(textControls);

            pos = percPos + 1;
            switch (text[pos])
            {
                case '%':
                {
                    textControls.command = '\0';
                    textControls.text = L"%";
                    textControls.xPos = textWidth;
                    tokens.push_back(textControls);
                    temprc = rect;
                    DrawText(hdc, L"%", 1, &temprc, flags | DT_CALCRECT);
                    textWidth += (temprc.right - temprc.left);
                    ++pos;
                }
                break;
                case 'i':   // italic
                {
                    font |= 1;
                    textControls.font = m_fonts[font];
                    textControls.command = text[pos];
                    objStack.push_front(SelectObject(hdc, m_fonts[font]));
                    ++pos;
                }
                break;
                case 'b':   // bold
                {
                    font |= 2;
                    textControls.font = m_fonts[font];
                    textControls.command = text[pos];
                    objStack.push_front(SelectObject(hdc, m_fonts[font]));
                    ++pos;
                }
                break;
                case 'c':   // color
                {
                    if (percPos < text.size() - 7)
                    {
                        auto sColor = text.substr(percPos + 2, 6);
                        auto color = wcstoul(sColor.c_str(), nullptr, 16);
                        color = RGB(GetBValue(color), GetGValue(color), GetRValue(color));
                        if (m_ThemeColorFunc)
                            color = m_ThemeColorFunc(color);
                        textControls.color = color;
                        textControls.command = text[pos];
                        SetTextColor(hdc, color);
                        pos += 7;
                    }
                }
                break;
                case 'r':   // reset
                {
                    font = 0;
                    for (auto& obj : objStack)
                        SelectObject(hdc, obj);
                    objStack.clear();
                    SetTextColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_WINDOWTEXT)) : GetSysColor(COLOR_WINDOWTEXT));
                    SetBkColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_3DFACE)) : GetSysColor(COLOR_3DFACE));
                    textControls.font = nullptr;
                    textControls.color = (COLORREF)-1;
                    textControls.command = text[pos];
                    ++pos;
                }
                break;
            }
        }
        else
            break;
        percPos = text.find('%', pos);
    }
    RECT temprc = rect;
    textControls.text = text.substr(pos);
    DrawText(hdc, textControls.text.c_str(), -1, &temprc, flags | DT_CALCRECT);
    textControls.xPos = textWidth;
    textWidth += (temprc.right - temprc.left);
    tokens.push_back(textControls);

    for (auto& obj : objStack)
        SelectObject(hdc, obj);

    if (flags & DT_CALCRECT)
    {
        rect.right = textWidth;
    }
    else
    {
        SetTextColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_WINDOWTEXT)) : GetSysColor(COLOR_WINDOWTEXT));
        SetBkColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_3DFACE)) : GetSysColor(COLOR_3DFACE));
        flags &= ~DT_CALCRECT;
        if (flags & DT_CENTER)
        {
            flags &= ~DT_CENTER;
            rect.left = rect.left + ((rect.right - rect.left) - textWidth) / 2;
        }
        if (flags & DT_RIGHT)
        {
            flags &= ~DT_RIGHT;
            rect.left = rect.right - textWidth;
        }

        for (auto& token : tokens)
        {
            switch (token.command)
            {
                case 'i':
                case 'b':
                objStack.push_front(SelectObject(hdc, token.font));
                break;
                case 'c':
                SetTextColor(hdc, token.color);
                break;
                case 'r':
                for (auto& obj : objStack)
                    SelectObject(hdc, obj);
                objStack.clear();
                SetTextColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_WINDOWTEXT)) : GetSysColor(COLOR_WINDOWTEXT));
                SetBkColor(hdc, m_ThemeColorFunc ? m_ThemeColorFunc(GetSysColor(COLOR_3DFACE)) : GetSysColor(COLOR_3DFACE));
                break;
            }
            RECT temprect = rect;
            temprect.left += token.xPos;
            DrawText(hdc, token.text.c_str(), -1, &temprect, flags);
        }
    }

    SelectObject(hdc, oldFont);
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
                if (!it->shortened)
                {
                    it->shortened = true;
                    bAdjusted = true;
                    break;
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
