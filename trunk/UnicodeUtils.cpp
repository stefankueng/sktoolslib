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

#include "stdafx.h"
#include "UnicodeUtils.h"
#include <memory>

CUnicodeUtils::CUnicodeUtils(void)
{
}

CUnicodeUtils::~CUnicodeUtils(void)
{
}

#ifdef UNICODE
std::string CUnicodeUtils::StdGetUTF8(const std::wstring& wide)
{
    int len = (int)wide.size();
    if (len==0)
        return std::string();
    int size = len*4;
    std::unique_ptr<char[]> narrow(new char[size]);
    int ret = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), len, narrow.get(), size-1, NULL, NULL);
    narrow[ret] = 0;
    std::string sRet = std::string(narrow.get());
    return sRet;
}

std::string CUnicodeUtils::StdGetANSI(const std::wstring& wide)
{
    int len = (int)wide.size();
    if (len==0)
        return std::string();
    int size = len*4;
    std::unique_ptr<char[]> narrow(new char[size]);
    int ret = WideCharToMultiByte(CP_ACP, 0, wide.c_str(), len, narrow.get(), size-1, NULL, NULL);
    narrow[ret] = 0;
    std::string sRet = std::string(narrow.get());
    return sRet;
}

std::wstring CUnicodeUtils::StdGetUnicode(const std::string& multibyte)
{
    int len = (int)multibyte.size();
    if (len==0)
        return std::wstring();
    int size = len*4;
    std::unique_ptr<wchar_t[]> wide(new wchar_t[size]);
    int ret = MultiByteToWideChar(CP_UTF8, 0, multibyte.c_str(), len, wide.get(), size - 1);
    wide[ret] = 0;
    std::wstring sRet = std::wstring(wide.get());
    return sRet;
}
#endif

std::string WideToMultibyte(const std::wstring& wide)
{
    std::unique_ptr<char[]> narrow(new char[wide.length()*3+2]);
    BOOL defaultCharUsed;
    int ret = (int)WideCharToMultiByte(CP_ACP, 0, wide.c_str(), (int)wide.size(), narrow.get(), (int)wide.length()*3 - 1, ".", &defaultCharUsed);
    narrow[ret] = 0;
    std::string str = narrow.get();
    return str;
}

std::string WideToUTF8(const std::wstring& wide)
{
    std::unique_ptr<char[]> narrow(new char[wide.length()*3+2]);
    int ret = (int)WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), narrow.get(), (int)wide.length()*3 - 1, NULL, NULL);
    narrow[ret] = 0;
    std::string str = narrow.get();
    return str;
}

std::wstring MultibyteToWide(const std::string& multibyte)
{
    size_t length = multibyte.length();
    if (length == 0)
        return std::wstring();

    std::unique_ptr<wchar_t[]> wide(new wchar_t[multibyte.length()*2+2]);
    if (wide == NULL)
        return std::wstring();
    int ret = (int)MultiByteToWideChar(CP_ACP, 0, multibyte.c_str(), (int)multibyte.size(), wide.get(), (int)length*2 - 1);
    wide[ret] = 0;
    std::wstring str = wide.get();
    return str;
}

std::wstring UTF8ToWide(const std::string& multibyte)
{
    size_t length = multibyte.length();
    if (length == 0)
        return std::wstring();

    std::unique_ptr<wchar_t[]> wide(new wchar_t[length*2+2]);
    if (wide == NULL)
        return std::wstring();
    int ret = (int)MultiByteToWideChar(CP_UTF8, 0, multibyte.c_str(), (int)multibyte.size(), wide.get(), (int)length*2 - 1);
    wide[ret] = 0;
    std::wstring str = wide.get();
    return str;
}
#ifdef UNICODE
tstring UTF8ToString(const std::string& string) {return UTF8ToWide(string);}
std::string StringToUTF8(const tstring& string) {return WideToUTF8(string);}
#else
tstring UTF8ToString(const std::string& string) {return WideToMultibyte(UTF8ToWide(string));}
std::string StringToUTF8(const tstring& string) {return WideToUTF8(MultibyteToWide(string));}
#endif

#pragma warning(push)
#pragma warning(disable: 4200)
struct STRINGRESOURCEIMAGE
{
    WORD nLength;
    WCHAR achString[];
};
#pragma warning(pop)    // C4200

int LoadStringEx(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax, WORD wLanguage)
{
    const STRINGRESOURCEIMAGE* pImage;
    const STRINGRESOURCEIMAGE* pImageEnd;
    ULONG nResourceSize;
    HGLOBAL hGlobal;
    UINT iIndex;
#ifndef UNICODE
    BOOL defaultCharUsed;
#endif
    int ret;

    if (lpBuffer == NULL)
        return 0;
    lpBuffer[0] = 0;
    HRSRC hResource =  FindResourceEx(hInstance, RT_STRING, MAKEINTRESOURCE(((uID>>4)+1)), wLanguage);
    if (!hResource)
    {
        //try the default language before giving up!
        hResource = FindResource(hInstance, MAKEINTRESOURCE(((uID>>4)+1)), RT_STRING);
        if (!hResource)
            return 0;
    }
    hGlobal = LoadResource(hInstance, hResource);
    if (!hGlobal)
        return 0;
    pImage = (const STRINGRESOURCEIMAGE*)::LockResource(hGlobal);
    if(!pImage)
        return 0;

    nResourceSize = ::SizeofResource(hInstance, hResource);
    pImageEnd = (const STRINGRESOURCEIMAGE*)(LPBYTE(pImage)+nResourceSize);
    iIndex = uID&0x000f;

    while ((iIndex > 0) && (pImage < pImageEnd))
    {
        pImage = (const STRINGRESOURCEIMAGE*)(LPBYTE(pImage)+(sizeof(STRINGRESOURCEIMAGE)+(pImage->nLength*sizeof(WCHAR))));
        iIndex--;
    }
    if (pImage >= pImageEnd)
        return 0;
    if (pImage->nLength == 0)
        return 0;
#ifdef UNICODE
    ret = pImage->nLength;
    if (ret > nBufferMax)
        ret = nBufferMax;
    wcsncpy_s((wchar_t *)lpBuffer, nBufferMax, pImage->achString, ret);
    lpBuffer[ret] = 0;
#else
    ret = WideCharToMultiByte(CP_ACP, 0, pImage->achString, pImage->nLength, (LPSTR)lpBuffer, nBufferMax-1, ".", &defaultCharUsed);
    lpBuffer[ret] = 0;
#endif
    return ret;
}
