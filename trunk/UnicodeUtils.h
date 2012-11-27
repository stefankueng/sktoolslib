// sktoolslib - common files for SK tools

// Copyright (C) 2012 - Stefan Kueng

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

typedef std::basic_string<TCHAR> tstring;

class CUnicodeUtils
{
public:
    CUnicodeUtils(void);
    ~CUnicodeUtils(void);
#ifdef UNICODE
    static std::string StdGetUTF8(const std::wstring& wide);
    static std::wstring StdGetUnicode(const std::string& multibyte);
    static std::string StdGetANSI(const std::wstring& wide);
#else
    static std::string StdGetUTF8(std::string str) {return str;}
    static std::string StdGetUnicode(std::string multibyte) {return multibyte;}
#endif
};

std::string WideToMultibyte(const std::wstring& wide);
std::string WideToUTF8(const std::wstring& wide);
std::wstring MultibyteToWide(const std::string& multibyte);
std::wstring UTF8ToWide(const std::string& multibyte);

#ifdef UNICODE
    tstring UTF8ToString(const std::string& string);
    std::string StringToUTF8(const tstring& string);
#else
    tstring UTF8ToString(const std::string& string);
    std::string StringToUTF8(const tstring& string);
#endif

int LoadStringEx(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax, WORD wLanguage);
