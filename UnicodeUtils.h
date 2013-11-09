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

/// determines the codepage from a text buffer. Returns -1 for binary
int GetCodepageFromBuf(LPVOID pBuffer, int cb, bool& hasBOM);

#ifdef UNICODE
    tstring UTF8ToString(const std::string& string);
    std::string StringToUTF8(const tstring& string);
#else
    tstring UTF8ToString(const std::string& string);
    std::string StringToUTF8(const tstring& string);
#endif

class UTF8Helper
{
public:
    // basic classification of UTF-8 bytes
    inline static bool isSingleByte(UCHAR c)       { return c < 0x80; }
    inline static bool isPartOfMultibyte(UCHAR c)  { return c >= 0x80; }
    inline static bool isFirstOfMultibyte(UCHAR c) { return c >= 0xC2 && c < 0xF5; } // 0xF5 to 0xFD are defined by UTF-8, but are not currently valid Unicode
    inline static bool isContinuation(UCHAR c)     { return (c & 0xC0) == 0x80; }
    inline static bool isValid(UCHAR c)            { return c < 0xC0 || isFirstOfMultibyte(c); }    // validates a byte, out of context

    // number of continuation bytes for a given valid first character (0 for single byte characters)
    inline static int  continuationBytes(UCHAR c)
    {
        static const char len[] = { 1,1,2,3 };
        return (c < 0xC0) ? 0 : len[(c & 0x30) >>  4];
    }

    // validates a full character
    inline static bool isValid(const char* buf, int buflen)
    {
        if (isSingleByte(buf[0])) return true; // single byte is valid
        if (!isFirstOfMultibyte(buf[0])) return false; // not single byte, nor valid multi-byte first byte
        int charContinuationBytes = continuationBytes(buf[0]);
        if(buflen < charContinuationBytes+1) return false; // character does not fit in buffer
        for (int i = charContinuationBytes; i > 0; --i)
            if (!isContinuation(*(++buf))) return false; // not enough continuation bytes
        return true;  // the character is valid (if there are too many continuation bytes, it is the next character that will be invalid)
    }

    // rewinds to the first byte of a multi-byte character for any valid UTF-8 (and will not rewind too much on any other input)
    inline static int characterStart(const char* buf, int startingIndex)
    {
        int charContinuationBytes = 0;
        while(charContinuationBytes < startingIndex // rewind past start of buffer?
            && charContinuationBytes < 5    // UTF-8 support up to 5 continuation bytes (but valid sequences currently do not have more than 3)
            && isContinuation(buf[startingIndex-charContinuationBytes])
            )
            ++charContinuationBytes;
        return startingIndex-charContinuationBytes;
    }
};


int LoadStringEx(HINSTANCE hInstance, UINT uID, LPTSTR lpBuffer, int nBufferMax, WORD wLanguage);