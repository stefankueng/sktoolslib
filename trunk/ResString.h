// sktoolslib - common files for SK tools

// Copyright (C) 2013 - Stefan Kueng

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
#include <string>
#include <memory>

/**
 * Loads a string from the application resources.
 */
class ResString
{
public:
    // Easier to use. TODO: Remove the reverse one if opportunity allows.
    ResString (int resId, HINSTANCE hInst = NULL)
        : ResString(hInst, resId)
    {
    }

    ResString (HINSTANCE hInst, int resId)
    {
        int bufsize = 1024;
        for ( ;; )
        {
            std::unique_ptr<wchar_t[]> buf(new wchar_t[bufsize]);
            int ret = ::LoadString(hInst, resId, buf.get(), bufsize);
            if (ret == (bufsize-1))
                bufsize *= 2;
            else
            {
                str = buf.get();
                break;
            }
        }
    }
    operator TCHAR const * () const { return str.c_str(); }
    operator std::wstring () const { return str; }
private:
    std::wstring str;
};

