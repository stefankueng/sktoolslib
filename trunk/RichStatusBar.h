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
#pragma once
#include "BaseWindow.h"
#include <string>
#include <vector>

class CRichStatusBarItem
{
public:
    /// text to show for the part.
    std::wstring        text;
    /// text to show if the width of the part is smaller than requested.
    /// if not set, the default text is used and cropped
    std::wstring        shortText;
    /// 0 : left align
    /// 1 : center
    /// 2 : right align
    int                 align;
    /// text for the tooltip of the part
    std::wstring        tooltip;
    /// icon to show. When the text is left-aligned or centered, the icon is shown to the left,
    /// when the text is right-aligned, the icon is shown right of the text.
    /// note: the icon will be shown with the same width and height.
    HICON               icon;
    /// the requested width of the part, in pixels. If set to -1, the width is calculated
    /// at runtime from the text and icon
    int                 width;
    /// determines whether the part can be resized with the main window
    bool                fixedWidth;
    /// icon to show if the width is too small for text.
    /// if not set, the text is shown cropped
    HICON               collapsedIcon;
    /// indicates the priority of this part. The higher the number the eariler
    /// the part is shortened or even collapsed.
    /// parts with the same priority are shortened and then collapsed from right to left.
    int                 shortenPriority;
};

/**
 * internal struct. do not use!
 */
struct PartWidths
{
    int shortWidth = 0;
    int defaultWidth = 0;
    bool fixed = false;
    bool shortened = false;
    bool collapsed = false;
    int calculatedWidth = 0;
};


/**
 * a custom status bar control
 */
class CRichStatusBar : public CWindow
{
public:
    CRichStatusBar(HINSTANCE hInst);
    ~CRichStatusBar();

    bool Init(HWND hParent);

    /// Sets/Updates or inserts a part.
    /// \param index the index to update. If the index does not exist, the function returns false.
    ///              if the index is set to -1, the part is inserted at the end.
    /// \param item  the item to set/insert
    /// \param redraw redraws the status bar after setting/inserting the part. Set to false while initializing.
    /// \param replace if true, the index must exist or be set to -1. if set to false,
    ///               the item is inserted before the index
    bool                SetPart(int index, CRichStatusBarItem item, bool redraw, bool replace = true);
    bool                SetPart(int index, const std::wstring& text, const std::wstring& shortText, const std::wstring& tooltip, int width, int align = 0, bool fixedWidth = false, HICON icon = nullptr, HICON collapsedIcon = nullptr);
    /// returns the recommended height of the status bar
    int                 GetHeight() const { return 22; }
    /// calculates the widths of all parts and updates the status bar.
    /// call this after changing parts or inserting new ones
    void                CalcWidths();

protected:
    LRESULT CALLBACK    WinMsgHandler(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) override;
    void                CalcRequestedWidths(int index);

private:
    std::vector<CRichStatusBarItem>     m_parts;
    std::vector<PartWidths>             m_partwidths;
    HFONT                               m_hFont;
    HWND                                m_tooltip;
};

