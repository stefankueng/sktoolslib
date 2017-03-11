﻿// sktoolslib - common files for SK tools

// Copyright (C) 2012, 2015, 2017 - Stefan Kueng

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


/**
 * \ingroup Utils
 * Helper classes for handles.
 */
template <typename HandleType,
    template <class> class CloseFunction,
    HandleType NULL_VALUE = nullptr>
class CSmartHandle : public CloseFunction<HandleType>
{
public:
    CSmartHandle()
    {
        m_Handle = NULL_VALUE;
    }

    // disable any copies of handles.
    // Handles must be copied only using DuplicateHandle(). But we leave
    // that to an explicit call.
    // See compiler tests at the bottom
    CSmartHandle(const HandleType& h) = delete;
    CSmartHandle(const CSmartHandle& h) = delete;
    HandleType& operator=(const HandleType& h) = delete;
    CSmartHandle& operator=(const CSmartHandle& h) = delete;

    CSmartHandle(HandleType && h)
    {
        m_Handle = h;
    }

    CSmartHandle(CSmartHandle && h)
    {
        m_Handle = h.Detach();
    }

    CSmartHandle& operator=(CSmartHandle && h)
    {
        if (m_Handle != (HandleType)h)
        {
            CleanUp();
            m_Handle = h.Detach();
        }
        else
            h.Detach();

        return *this;
    }

    HandleType& operator=(HandleType && h)
    {
        if (m_Handle != h)
        {
            CleanUp();
            m_Handle = h;
        }

        return m_Handle;
    }

    bool CloseHandle()
    {
        return CleanUp();
    }

    HandleType Detach()
    {
        HandleType p;

        p = m_Handle;
        m_Handle = NULL_VALUE;

        return p;
    }

    operator HandleType()
    {
        return m_Handle;
    }

    HandleType * GetPointer()
    {
        return &m_Handle;
    }

    operator bool()
    {
        return IsValid();
    }

    bool IsValid()
    {
        return m_Handle != NULL_VALUE;
    }

    HandleType Duplicate()
    {
        HandleType hDup = NULL_VALUE;
        if (DuplicateHandle(GetCurrentProcess(),
                            (HANDLE)m_Handle,
                            GetCurrentProcess(),
                            &hDup,
                            0,
                            FALSE,
                            DUPLICATE_SAME_ACCESS))
        {
            return hDup;
        }
        return NULL_VALUE;
    }

    ~CSmartHandle()
    {
        CleanUp();
    }


protected:
    bool CleanUp()
    {
        if (m_Handle != NULL_VALUE)
        {
            bool b = Close(m_Handle);
            m_Handle = NULL_VALUE;
            return b;
        }
        return false;
    }


    HandleType m_Handle;
};

class CEmptyClass
{
};

template <typename T>
struct CCloseHandle
{
    bool Close(T handle)
    {
        return !!::CloseHandle(handle);
    }

protected:
    ~CCloseHandle()
    {
    }
};



template <typename T>
struct CCloseRegKey
{
    bool Close(T handle)
    {
        return RegCloseKey(handle) == ERROR_SUCCESS;
    }

protected:
    ~CCloseRegKey()
    {
    }
};


template <typename T>
struct CCloseLibrary
{
    bool Close(T handle)
    {
        return !!::FreeLibrary(handle);
    }

protected:
    ~CCloseLibrary()
    {
    }
};


template <typename T>
struct CCloseViewOfFile
{
    bool Close(T handle)
    {
        return !!::UnmapViewOfFile(handle);
    }

protected:
    ~CCloseViewOfFile()
    {
    }
};

template <typename T>
struct CCloseFindFile
{
    bool Close(T handle)
    {
        return !!::FindClose(handle);
    }

protected:
    ~CCloseFindFile()
    {
    }
};


// Client code (definitions of standard Windows handles).
typedef CSmartHandle<HANDLE,  CCloseHandle>                                         CAutoGeneralHandle;
typedef CSmartHandle<HKEY,    CCloseRegKey>                                         CAutoRegKey;
typedef CSmartHandle<PVOID,   CCloseViewOfFile>                                     CAutoViewOfFile;
typedef CSmartHandle<HMODULE, CCloseLibrary>                                        CAutoLibrary;
typedef CSmartHandle<HANDLE,  CCloseHandle, INVALID_HANDLE_VALUE>                   CAutoFile;
typedef CSmartHandle<HANDLE,  CCloseFindFile, INVALID_HANDLE_VALUE>                 CAutoFindFile;

/*
void CompilerTests()
{
    // compiler tests
    {
        HANDLE h = (HANDLE)1;
        CAutoFile hFile = h;                    // C2280
        CAutoFile hFile2 = std::move(h);        // OK
        // OK, uses move semantics
        CAutoFile hFile3 = CreateFile(L"c:\\test.txt", GENERIC_WRITE, FILE_SHARE_READ, nullptr, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
        CAutoFile hFile4 = hFile3;              // C2280
        CAutoFile hFile5 = std::move(hFile3);   // OK
    }
}
*/