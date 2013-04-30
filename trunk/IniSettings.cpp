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
#include "IniSettings.h"
#include "StringUtils.h"

CIniSettings::CIniSettings(void)
{
}


CIniSettings::~CIniSettings(void)
{
    Save();
}

CIniSettings& CIniSettings::Instance()
{
    static CIniSettings instance;
    return instance;
}

void CIniSettings::SetIniPath( const std::wstring& p )
{
    if (p.empty())
    {
        wchar_t buf[MAX_PATH] = {0};
        GetModuleFileName(NULL, buf, _countof(buf));
        m_iniPath = buf;
        m_iniPath = m_iniPath.substr(0, m_iniPath.find_last_of('\\'));
        m_iniPath += L"\\settings";
    }
    else
        m_iniPath = p;
    m_IniFile.LoadFile(m_iniPath.c_str());
}

void CIniSettings::Save()
{
    FILE * pFile = NULL;
    _tfopen_s(&pFile, m_iniPath.c_str(), _T("wb"));
    m_IniFile.SaveFile(pFile);
    fclose(pFile);
}

__int64 CIniSettings::GetInt64( LPCWSTR section, LPCWSTR key, __int64 default )
{
    _ASSERT(m_iniPath.size());
    const wchar_t * v = m_IniFile.GetValue(section, key, NULL);
    if (v == NULL)
        return default;

    return _wcstoi64(v, NULL, 10);
}

void CIniSettings::SetInt64( LPCWSTR section, LPCWSTR key, __int64 value )
{
    wchar_t val[100] = {0};
    _i64tow_s(value, val, _countof(val), 10);
    m_IniFile.SetValue(section, key, val);
}

LPCWSTR CIniSettings::GetString( LPCWSTR section, LPCWSTR key, LPCWSTR default /*= nullptr*/ )
{
    _ASSERT(m_iniPath.size());
    return m_IniFile.GetValue(section, key, default);
}

void CIniSettings::SetString( LPCWSTR section, LPCWSTR key, LPCWSTR value )
{
    m_IniFile.SetValue(section, key, value);
}

void CIniSettings::RestoreWindowPos( LPCWSTR windowname, HWND hWnd, UINT showCmd )
{
    WINDOWPLACEMENT wpl = {0};
    wpl.length = sizeof(WINDOWPLACEMENT);

    wpl.flags                   = (UINT)GetInt64(L"windowpos", CStringUtils::Format(L"%s_flags", windowname).c_str(), 0);
    wpl.showCmd                 = (UINT)GetInt64(L"windowpos", CStringUtils::Format(L"%s_showCmd", windowname).c_str(), 0);
    wpl.ptMinPosition.x         = (LONG)GetInt64(L"windowpos", CStringUtils::Format(L"%s_ptMinPositionX", windowname).c_str(), 0);
    wpl.ptMinPosition.y         = (LONG)GetInt64(L"windowpos", CStringUtils::Format(L"%s_ptMinPositionY", windowname).c_str(), 0);
    wpl.ptMaxPosition.x         = (LONG)GetInt64(L"windowpos", CStringUtils::Format(L"%s_ptMaxPositionX", windowname).c_str(), 0);
    wpl.ptMaxPosition.y         = (LONG)GetInt64(L"windowpos", CStringUtils::Format(L"%s_ptMaxPositionY", windowname).c_str(), 0);
    wpl.rcNormalPosition.left   = (LONG)GetInt64(L"windowpos", CStringUtils::Format(L"%s_rcNormalPositionLeft", windowname).c_str(), 0);
    wpl.rcNormalPosition.top    = (LONG)GetInt64(L"windowpos", CStringUtils::Format(L"%s_rcNormalPositionTop", windowname).c_str(), 0);
    wpl.rcNormalPosition.right  = (LONG)GetInt64(L"windowpos", CStringUtils::Format(L"%s_rcNormalPositionRight", windowname).c_str(), 0);
    wpl.rcNormalPosition.bottom = (LONG)GetInt64(L"windowpos", CStringUtils::Format(L"%s_rcNormalPositionBottom", windowname).c_str(), 0);

    if (wpl.showCmd)
    {
        wpl.showCmd = showCmd;
        SetWindowPlacement(hWnd, &wpl);
    }
}

void CIniSettings::SaveWindowPos( LPCWSTR windowname, HWND hWnd )
{
    WINDOWPLACEMENT wpl = {0};
    wpl.length = sizeof(WINDOWPLACEMENT);
    GetWindowPlacement(hWnd, &wpl);

    SetInt64(L"windowpos", CStringUtils::Format(L"%s_flags", windowname).c_str(),                   wpl.flags);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_showCmd", windowname).c_str(),                 wpl.showCmd);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_ptMinPositionX", windowname).c_str(),          wpl.ptMinPosition.x);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_ptMinPositionY", windowname).c_str(),          wpl.ptMinPosition.y);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_ptMaxPositionX", windowname).c_str(),          wpl.ptMaxPosition.x);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_ptMaxPositionY", windowname).c_str(),          wpl.ptMaxPosition.y);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_rcNormalPositionLeft", windowname).c_str(),    wpl.rcNormalPosition.left);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_rcNormalPositionTop", windowname).c_str(),     wpl.rcNormalPosition.top);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_rcNormalPositionRight", windowname).c_str(),   wpl.rcNormalPosition.right);
    SetInt64(L"windowpos", CStringUtils::Format(L"%s_rcNormalPositionBottom", windowname).c_str(),  wpl.rcNormalPosition.bottom);
}
