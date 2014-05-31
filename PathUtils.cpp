// sktoolslib - common files for SK tools

// Copyright (C) 2013-2014 - Stefan Kueng

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

std::wstring CPathUtils::AdjustForMaxPath(const std::wstring& path)
{
    if (path.size() < 248)  // 248 instead of MAX_PATH because 248 is the limit for directories
        return path;
    if (path.substr(0, 4).compare(L"\\\\?\\") == 0)
        return path;
    return L"\\\\?\\" + path;
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

std::wstring CPathUtils::GetFileName(const std::wstring& path)
{
    auto pos = path.find_last_of('\\');
    if (pos != std::wstring::npos)
    {
        std::wstring sName = path.substr(pos + 1);
        return sName;
    }
    else
    {
        pos = path.find_last_of('/');
        std::wstring sName = path.substr(pos + 1);
        return sName;
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
    dwReserved = 0;
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

std::wstring CPathUtils::GetAppDataPath(HMODULE hMod)
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
    sPath = sPath.substr(0, sPath.find_last_of('\\'));
    return sPath;
}

std::wstring CPathUtils::GetCWD()
{
    // Getting the CWD is a little more complicated than it seems.
    // The directory can change between asking for the name size
    // and obtaining the name value. So we need to handle that.
    // We also need to handle any eror the first or second time we ask.

    for (;;)
    {
        // Returned length already includes + 1 fo null.
        auto estimatedLen = GetCurrentDirectory(0, NULL);
        if (estimatedLen <= 0) // Error, can't recover.
            break;
        std::unique_ptr<TCHAR[]> cwd(new TCHAR[estimatedLen]);
        auto actualLen = GetCurrentDirectory(estimatedLen, cwd.get());
        if (actualLen <= 0) // Error Can't recover
            break;
        // Directory changed in mean time and got larger..
        if (actualLen <= estimatedLen)
            return std::wstring(cwd.get(), actualLen);
        // If we reach here, the directory has changed between us
        // asking for it's size and obtaining the value and the
        // the size has increased, so loop around to try again.
    }
    return L"";
}


bool CPathUtils::Unzip2Folder(LPCWSTR lpZipFile, LPCWSTR lpFolder)
{
    IShellDispatch *pISD;

    Folder  *pZippedFile = 0L;
    Folder  *pDestination = 0L;

    long FilesCount = 0;
    IDispatch* pItem = 0L;
    FolderItems *pFilesInside = 0L;

    VARIANT Options, OutFolder, InZipFile, Item;
    HRESULT hr = S_OK;
    CoInitialize(NULL);
    __try
    {
        if (CoCreateInstance(CLSID_Shell, NULL, CLSCTX_INPROC_SERVER, IID_IShellDispatch, (void **)&pISD) != S_OK)
            return false;

        InZipFile.vt = VT_BSTR;
        InZipFile.bstrVal = const_cast<BSTR>(lpZipFile);
        hr = pISD->NameSpace(InZipFile, &pZippedFile);
        if (FAILED(hr) || !pZippedFile)
        {
            pISD->Release();
            return false;
        }

        OutFolder.vt = VT_BSTR;
        OutFolder.bstrVal = const_cast<BSTR>(lpFolder);
        pISD->NameSpace(OutFolder, &pDestination);
        if (!pDestination)
        {
            pZippedFile->Release();
            pISD->Release();
            return false;
        }

        pZippedFile->Items(&pFilesInside);
        if (!pFilesInside)
        {
            pDestination->Release();
            pZippedFile->Release();
            pISD->Release();
            return false;
        }

        pFilesInside->get_Count(&FilesCount);
        if (FilesCount < 1)
        {
            pFilesInside->Release();
            pDestination->Release();
            pZippedFile->Release();
            pISD->Release();
            return true;
        }

        pFilesInside->QueryInterface(IID_IDispatch, (void**)&pItem);

        Item.vt = VT_DISPATCH;
        Item.pdispVal = pItem;

        Options.vt = VT_I4;
        Options.lVal = 1024 | 512 | 16 | 4;//http://msdn.microsoft.com/en-us/library/bb787866(VS.85).aspx

        bool retval = pDestination->CopyHere(Item, Options) == S_OK;

        pItem->Release(); pItem = 0L;
        pFilesInside->Release(); pFilesInside = 0L;
        pDestination->Release(); pDestination = 0L;
        pZippedFile->Release(); pZippedFile = 0L;
        pISD->Release(); pISD = 0L;

        return retval;

    }
    __finally
    {
        CoUninitialize();
    }
}