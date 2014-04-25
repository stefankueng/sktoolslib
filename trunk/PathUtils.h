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

#pragma once

#include <string>
#include <algorithm>
#include <functional>
#include <memory>


class CPathUtils
{
public:

    static std::wstring GetLongPathname(const std::wstring& path);
    static std::wstring AdjustForMaxPath(const std::wstring& path);

    static std::wstring GetParentDirectory(const std::wstring& path);
    static std::wstring GetFileExtension(const std::wstring& path);
    static std::wstring GetFileName(const std::wstring& path);
    // module/app paths
    static std::wstring GetModulePath(HMODULE hMod = NULL);
    static std::wstring GetModuleDir(HMODULE hMod = NULL);
    static std::wstring Append(const std::wstring& path, const std::wstring& append);
    static std::wstring GetTempFilePath();
    static std::wstring GetVersionFromFile(const std::wstring& path);
    static std::wstring GetAppDataPath(HMODULE hMod = NULL);
};