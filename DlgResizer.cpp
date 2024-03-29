﻿// sktoolslib - common files for SK tools

// Copyright (C) 2012-2013, 2016, 2020-2021 - Stefan Kueng

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
#include "DlgResizer.h"
#include <cassert>
#include <commctrl.h>

#ifndef ComboBox_GetEditSel
#    include <windowsx.h>
#endif

CDlgResizer::CDlgResizer()
    : m_hDlg(nullptr)
    , m_wndGrip(nullptr)
    , m_useSizeGrip(true)
{
    m_controls.clear();
    m_dlgRect       = {};
    m_dlgRectScreen = {};
    m_sizeGrip      = {};
}

CDlgResizer::~CDlgResizer()
{
    m_controls.clear();
}

void CDlgResizer::Init(HWND hWndDlg)
{
    m_hDlg = hWndDlg;
    GetClientRect(hWndDlg, &m_dlgRect);
    GetWindowRect(hWndDlg, &m_dlgRectScreen);
    OffsetRect(&m_dlgRectScreen, -m_dlgRectScreen.left, -m_dlgRectScreen.top);

    m_sizeGrip.cx = GetSystemMetrics(SM_CXVSCROLL);
    m_sizeGrip.cy = GetSystemMetrics(SM_CYHSCROLL);

    RECT rect = {0, 0, m_sizeGrip.cx, m_sizeGrip.cy};

    m_wndGrip = ::CreateWindowEx(0, WC_SCROLLBAR,
                                 static_cast<LPCWSTR>(nullptr),
                                 WS_CHILD | WS_CLIPSIBLINGS | SBS_SIZEGRIP,
                                 rect.left, rect.top,
                                 rect.right - rect.left,
                                 rect.bottom - rect.top,
                                 m_hDlg,
                                 static_cast<HMENU>(nullptr),
                                 nullptr,
                                 nullptr);

    if (m_wndGrip)
    {
        HRGN rgn     = ::CreateRectRgn(0, 0, 1, 1);
        HRGN rgnGrip = ::CreateRectRgnIndirect(&rect);

        for (int y = 0; y < m_sizeGrip.cy; y++)
        {
            ::SetRectRgn(rgn, 0, y, m_sizeGrip.cx - y, y + 1);
            ::CombineRgn(rgnGrip, rgnGrip, rgn, RGN_DIFF);
        }
        ::SetWindowRgn(m_wndGrip, rgnGrip, FALSE);

        if (m_useSizeGrip)
        {
            // update pos
            UpdateGripPos();
            ShowSizeGrip();
        }
    }
}

void CDlgResizer::AdjustMinMaxSize()
{
    GetWindowRect(m_hDlg, &m_dlgRectScreen);
    OffsetRect(&m_dlgRectScreen, -m_dlgRectScreen.left, -m_dlgRectScreen.top);
}

void CDlgResizer::AddControl(HWND hWndDlg, UINT ctrlId, UINT resizeType)
{
    ResizeCtrls ctrlInfo;

    ctrlInfo.hWnd = GetDlgItem(hWndDlg, ctrlId);
    if (!ctrlInfo.hWnd)
    {
        assert(false);
        return;
    }
    ctrlInfo.resizeType = resizeType;

    GetWindowRect(ctrlInfo.hWnd, &ctrlInfo.origSize);
    OffsetRect(&ctrlInfo.origSize, -ctrlInfo.origSize.left, -ctrlInfo.origSize.top);
    MapWindowPoints(ctrlInfo.hWnd, hWndDlg, reinterpret_cast<LPPOINT>(&ctrlInfo.origSize), 2);

    m_controls.push_back(ctrlInfo);
}

void CDlgResizer::DoResize(int width, int height)
{
    UpdateGripPos();
    if (m_controls.empty())
        return;

    InvalidateRect(m_hDlg, nullptr, true);
    HDWP hDwp = BeginDeferWindowPos(static_cast<int>(m_controls.size()));

    std::vector<std::pair<size_t, DWORD>> savedSelections;
    for (size_t i = 0; i < m_controls.size(); ++i)
    {
        wchar_t className[257];
        const auto& [hWnd, resizeType, origSize] = m_controls[i];
        // Work around a bug in the standard combo box control that causes it to
        // incorrectly change the selection status after resizing. Without this
        // fix sometimes the combo box will show selected text after a WM_SIZE
        // resize type event even if there was no text selected before the size event.
        // The workaround is to save the current selection state before the resize and
        // to restore that state after the resize.
        int  status     = GetClassName(hWnd, className, static_cast<int>(std::size(className)));
        bool isComboBox = status > 0 && _wcsicmp(className, WC_COMBOBOX) == 0;
        if (isComboBox)
        {
            DWORD sel = ComboBox_GetEditSel(hWnd);
            savedSelections.push_back({i, sel});
        }
        RECT newPos = origSize;
        switch (resizeType)
        {
            case RESIZER_TOPLEFT:
                break; // do nothing - the original position is fine
            case RESIZER_TOPRIGHT:
                newPos.left += (width - m_dlgRect.right);
                newPos.right += (width - m_dlgRect.right);
                break;
            case RESIZER_TOPLEFTRIGHT:
                newPos.right += (width - m_dlgRect.right);
                break;
            case RESIZER_TOPLEFTBOTTOMRIGHT:
                newPos.right += (width - m_dlgRect.right);
                newPos.bottom += (height - m_dlgRect.bottom);
                break;
            case RESIZER_BOTTOMLEFT:
                newPos.top += (height - m_dlgRect.bottom);
                newPos.bottom += (height - m_dlgRect.bottom);
                break;
            case RESIZER_BOTTOMRIGHT:
                newPos.top += (height - m_dlgRect.bottom);
                newPos.bottom += (height - m_dlgRect.bottom);
                newPos.left += (width - m_dlgRect.right);
                newPos.right += (width - m_dlgRect.right);
                break;
            case RESIZER_BOTTOMLEFTRIGHT:
                newPos.top += (height - m_dlgRect.bottom);
                newPos.bottom += (height - m_dlgRect.bottom);
                newPos.right += (width - m_dlgRect.right);
                break;
            case RESIZER_TOPLEFTBOTTOMLEFT:
                newPos.bottom += (height - m_dlgRect.bottom);
                break;
            case RESIZER_TOPRIGHTBOTTOMRIGHT:
                newPos.left += (width - m_dlgRect.right);
                newPos.right += (width - m_dlgRect.right);
                newPos.bottom += (height - m_dlgRect.bottom);
                break;
        }
        hDwp = DeferWindowPos(hDwp, hWnd, nullptr, newPos.left, newPos.top,
                              newPos.right - newPos.left, newPos.bottom - newPos.top,
                              SWP_NOZORDER | SWP_NOACTIVATE);
    }
    EndDeferWindowPos(hDwp);
    for (const auto& [index, sel] : savedSelections)
    {
        int startSel = LOWORD(sel);
        int endSel   = HIWORD(sel);
        ComboBox_SetEditSel(m_controls[index].hWnd, startSel, endSel);
    }
    UpdateGripPos();
}

void CDlgResizer::ShowSizeGrip(bool bShow) const
{
    ::ShowWindow(m_wndGrip, (bShow && m_useSizeGrip) ? SW_SHOW : SW_HIDE);
}

void CDlgResizer::UpdateGripPos() const
{
    RECT rect;
    ::GetClientRect(m_hDlg, &rect);

    rect.left = rect.right - m_sizeGrip.cx;
    rect.top  = rect.bottom - m_sizeGrip.cy;

    // must stay below other children
    ::SetWindowPos(m_wndGrip, HWND_BOTTOM, rect.left, rect.top, 0, 0,
                   SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOREPOSITION);

    // maximized windows cannot be resized

    if (::IsZoomed(m_hDlg))
    {
        ::EnableWindow(m_wndGrip, FALSE);
        ShowSizeGrip(false);
    }
    else
    {
        ::EnableWindow(m_wndGrip, TRUE);
        ShowSizeGrip(true);
    }
}
