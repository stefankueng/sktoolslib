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
#include <vector>


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
