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

#include "stdafx.h"
#include "PathUtils.h"
#include "StringUtils.h"
#include <vector>
#include <Shlwapi.h>

#pragma comment(lib, "Shlwapi.lib")
#pragma comment(lib, "version.lib")

std::wstring CPathUtils::GetLongPathname(const std::wstring& path)
{
    if (path.empty())
        return path;
    TCHAR pathbufcanonicalized[MAX_PATH]; // MAX_PATH ok.
    DWORD ret = 0;
    std::wstring sRet = path;
    if (!PathIsURL(path.c_str()) && PathIsRelative(path.c_str()))
    {
        ret = GetFullPathName(path.c_str(), 0, NULL, NULL);
        if (ret)
        {
            std::unique_ptr<TCHAR[]> pathbuf(new TCHAR[ret+1]);
            if ((ret = GetFullPathName(path.c_str(), ret, pathbuf.get(), NULL))!=0)
            {
                sRet = std::wstring(pathbuf.get(), ret);
            }
        }
    }
    else if (PathCanonicalize(pathbufcanonicalized, path.c_str()))
    {
        ret = ::GetLongPathName(pathbufcanonicalized, NULL, 0);
        std::unique_ptr<TCHAR[]> pathbuf(new TCHAR[ret+2]);
        ret = ::GetLongPathName(pathbufcanonicalized, pathbuf.get(), ret+1);
        // GetFullPathName() sometimes returns the full path with the wrong
        // case. This is not a problem on Windows since its filesystem is
        // case-insensitive. But for SVN that's a problem if the wrong case
        // is inside a working copy: the svn wc database is case sensitive.
        // To fix the casing of the path, we use a trick:
        // convert the path to its short form, then back to its long form.
        // That will fix the wrong casing of the path.
        int shortret = ::GetShortPathName(pathbuf.get(), NULL, 0);
        if (shortret)
        {
            std::unique_ptr<TCHAR[]> shortpath(new TCHAR[shortret+2]);
            if (::GetShortPathName(pathbuf.get(), shortpath.get(), shortret+1))
            {
                int ret2 = ::GetLongPathName(shortpath.get(), pathbuf.get(), ret+1);
                if (ret2)
                    sRet = std::wstring(pathbuf.get(), ret2);
            }
        }
    }
    else
    {
        ret = ::GetLongPathName(path.c_str(), NULL, 0);
        std::unique_ptr<TCHAR[]> pathbuf(new TCHAR[ret+2]);
        ret = ::GetLongPathName(path.c_str(), pathbuf.get(), ret+1);
        sRet = std::wstring(pathbuf.get(), ret);
        // fix the wrong casing of the path. See above for details.
        int shortret = ::GetShortPathName(pathbuf.get(), NULL, 0);
        if (shortret)
        {
            std::unique_ptr<TCHAR[]> shortpath(new TCHAR[shortret+2]);
            if (::GetShortPathName(pathbuf.get(), shortpath.get(), shortret+1))
            {
                int ret2 = ::GetLongPathName(shortpath.get(), pathbuf.get(), ret+1);
                if (ret2)
                    sRet = std::wstring(pathbuf.get(), ret2);
            }
        }
    }
    if (ret == 0)
        return path;
    return sRet;
}


std::wstring CPathUtils::GetParentDirectory( const std::wstring& path )
{
    auto pos = path.find_last_of('\\');
    if (pos != std::wstring::npos)
    {
        std::wstring sPath = path.substr(0, pos);
        return sPath;
    }
    return path;
}

std::wstring CPathUtils::GetFileExtension( const std::wstring& path )
{
    auto pos = path.find_last_of('.');
    if (pos != std::wstring::npos)
    {
        std::wstring sExt = path.substr(pos+1);
        return sExt;
    }
    return L"";
}

std::wstring CPathUtils::GetModulePath( HMODULE hMod /*= NULL*/ )
{
    DWORD len = 0;
    DWORD bufferlen = MAX_PATH;     // MAX_PATH is not the limit here!
    std::unique_ptr<wchar_t[]> path(new wchar_t[bufferlen]);
    do
    {
        bufferlen += MAX_PATH;      // MAX_PATH is not the limit here!
        path = std::unique_ptr<wchar_t[]>(new wchar_t[bufferlen]);
        len = GetModuleFileName(hMod, path.get(), bufferlen);
    } while(len == bufferlen);
    std::wstring sPath = path.get();
    return sPath;
}

std::wstring CPathUtils::GetModuleDir( HMODULE hMod /*= NULL*/ )
{
    return GetParentDirectory(GetModulePath(hMod));
}

std::wstring CPathUtils::Append( const std::wstring& path, const std::wstring& append )
{
    if (append.empty())
        return path;
    size_t pos = append.find_first_not_of('\\');
    if (*path.rbegin() == '\\')
        return path + &append[pos];
    return path + L"\\" + &append[pos];
}

std::wstring CPathUtils::GetTempFilePath()
{
    DWORD len = ::GetTempPath(0, NULL);
    std::unique_ptr<TCHAR[]> temppath(new TCHAR[len+1]);
    std::unique_ptr<TCHAR[]> tempF(new TCHAR[len+50]);
    ::GetTempPath (len+1, temppath.get());
    std::wstring tempfile;
    ::GetTempFileName (temppath.get(), TEXT("cm_"), 0, tempF.get());
    tempfile = std::wstring(tempF.get());
    //now create the tempfile, so that subsequent calls to GetTempFile() return
    //different filenames.
    HANDLE hFile = CreateFile(tempfile.c_str(), GENERIC_READ, FILE_SHARE_READ, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_TEMPORARY, NULL);
    CloseHandle(hFile);
    return tempfile;
}

std::wstring CPathUtils::GetVersionFromFile(const std::wstring& path)
{
    struct TRANSARRAY
    {
        WORD wLanguageID;
        WORD wCharacterSet;
    };

    std::wstring strReturn;
    DWORD dwReserved = 0;
    DWORD dwBufferSize = GetFileVersionInfoSize((LPTSTR)(LPCTSTR)path.c_str(),&dwReserved);

    if (dwBufferSize > 0)
    {
        LPVOID pBuffer = (void*) malloc(dwBufferSize);

        if (pBuffer != (void*) NULL)
        {
            UINT            nInfoSize = 0,
                            nFixedLength = 0;
            LPSTR           lpVersion = NULL;
            VOID*           lpFixedPointer;
            TRANSARRAY*     lpTransArray;
            std::wstring    strLangProduktVersion;

            GetFileVersionInfo((LPTSTR)(LPCTSTR)path.c_str(),
                dwReserved,
                dwBufferSize,
                pBuffer);

            // Check the current language
            VerQueryValue(  pBuffer,
                L"\\VarFileInfo\\Translation",
                &lpFixedPointer,
                &nFixedLength);
            lpTransArray = (TRANSARRAY*) lpFixedPointer;

            strLangProduktVersion = CStringUtils::Format(L"\\StringFileInfo\\%04x%04x\\ProductVersion",
                                                         lpTransArray[0].wLanguageID, lpTransArray[0].wCharacterSet);

            VerQueryValue(pBuffer,
                (LPTSTR)(LPCTSTR)strLangProduktVersion.c_str(),
                (LPVOID *)&lpVersion,
                &nInfoSize);
            if (nInfoSize && lpVersion)
                strReturn = (LPCTSTR)lpVersion;
            free(pBuffer);
        }
    }

    return strReturn;
}
