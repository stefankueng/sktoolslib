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
#include <vector>


/**
 * handles text files.
 */
class CTextFile
{
public:
    CTextFile(void);
    ~CTextFile(void);

    enum UnicodeType
    {
        AUTOTYPE,
        BINARY,
        ANSI,
        UNICODE_LE,
        UTF8,
    };

    /**
     * Loads a file from the specified \c path.
     */
    bool            Load(LPCTSTR path, UnicodeType& type, bool bUTF8);

    /**
     * Saves the file contents to disk at \c path.
     */
    bool            Save(LPCTSTR path);

    /**
     * modifies the contents of a file.
     * \param pBuf pointer to a buffer holding the new contents of the file.
     *             note: the buffer must be created with 'new BYTE[len]'
     * \param newLen length of the new file content in bytes
     * \note the old buffer is automatically freed.
     */
    bool            ContentsModified(BYTE * pBuf, DWORD newLen);

    /**
     * Returns the line number from a given character position inside the file.
     */
    long            LineFromPosition(long pos) const;

    /**
     * Returns the line from a given line number
     */
    std::wstring    GetLineString(long lineNumber) const;

    /**
     * Returns the file content as a text string.
     * \note the text string can not be modified and is to be treated read-only.
     */
    const std::wstring&  GetFileString() const {return textcontent;}

    /**
     * Returns a pointer to the file contents. Call GetFileLength() to get
     * the size in number of bytes of this buffer.
     */
    LPVOID          GetFileContent() {return pFileBuf;}

    /**
     * Returns the size of the file in bytes
     */
    long            GetFileLength() const {return filelen;}

    /**
     * Returns the encoding of the file
     */
    UnicodeType     GetEncoding() const {return encoding;}

    /**
     * Returns the filename
     */
    const std::wstring&  GetFileName() const {return filename;}

    /**
     * Returns the filename without the extension (if any)
     */
    std::wstring    GetFileNameWithoutExtension();

    /**
     * Returns the filename extension (if any)
     */
    std::wstring    GetFileNameExtension();

    /**
     * Replaces the file content.
     */
    void            SetFileContent(const std::wstring& content);
protected:
    /**
     * Tries to find out the encoding of the file (utf8, utf16, ansi)
     */
    UnicodeType     CheckUnicodeType(BYTE * pBuffer, int cb);
    /**
     * Fills an array with line information to make it faster later
     * to get the line from a char position.
     */
    bool            CalculateLines();

private:
    BYTE *              pFileBuf;
    DWORD               filelen;
    std::wstring        textcontent;
    std::vector<size_t> linepositions;
    UnicodeType         encoding;
    std::wstring        filename;
    bool                hasBOM;
};