// sktoolslib - common files for SK tools

// Copyright (C) 2025 - Stefan Kueng

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

// helper class to iterate over an utf-8 encoded string and return wide characters
// used for regex_search with std::wregex
//
// if std::regex ever implements the regex_traits<char32_t>, then this class can easily
// be changed to return char32_t instead of wchar_t by removing the lenBytes, lenCharacters and wcharBuf members
// and just use the codepoint instead.
class Utf8ToWideIterator
{
public:
    using iterator_category = std::bidirectional_iterator_tag;
    using value_type        = wchar_t;
    using difference_type   = ptrdiff_t;
    using pointer           = wchar_t *;
    using reference         = wchar_t &;

    explicit Utf8ToWideIterator(const unsigned char *text = nullptr, std::ptrdiff_t position = 0) noexcept
        : text(text)
        , position(position)
        , characterIndex(0)
        , lenBytes(0)
        , lenCharacters(0)
        , wcharBuf{}
    {
        if (text)
            GetCodePoint();
    }

    value_type operator*() const noexcept
    {
        assert(lenCharacters != 0);
        return wcharBuf[characterIndex];
    }

    Utf8ToWideIterator &operator++() noexcept
    {
        if ((characterIndex + 1) < (lenCharacters))
            characterIndex++;
        else
        {
            position += lenBytes;
            GetCodePoint();
            characterIndex = 0;
        }
        return *this;
    }

    Utf8ToWideIterator operator++(int) noexcept
    {
        Utf8ToWideIterator ret(*this);
        if ((characterIndex + 1) < (lenCharacters))
            characterIndex++;
        else
        {
            position += lenBytes;
            GetCodePoint();
            characterIndex = 0;
        }
        return ret;
    }

    Utf8ToWideIterator &operator--() noexcept
    {
        if (characterIndex)
            characterIndex--;
        else
        {
            --position;
            while ((static_cast<unsigned char>(*(text + position)) & 0xC0) == 0x80)
                --position;

            GetCodePoint();
            characterIndex = lenCharacters - 1;
        }
        return *this;
    }

    bool operator==(const Utf8ToWideIterator &other) const noexcept
    {
        // don't test the cached/buffered values
        return text == other.text &&
               position == other.position &&
               characterIndex == other.characterIndex;
    }

    bool operator!=(const Utf8ToWideIterator &other) const noexcept
    {
        return !operator==(other);
    }

    std::ptrdiff_t CurrentPos() const noexcept
    {
        return position;
    }

private:
    void GetCodePoint() noexcept
    {
        char32_t             codePoint = 0;
        const unsigned char *pBuf      = (text + position);
        if (*pBuf < 0x80)
        {
            codePoint = *pBuf;
            lenBytes  = 1;
        }
        else if (*pBuf < 0xC0)
        {
            // Invalid leading byte
            codePoint = UNICODE_REPLACEMENT_CHARACTER;
            lenBytes  = 1;
        }
        else if (*pBuf < 0xE0)
        {
            codePoint = *pBuf & 0x1F;
            lenBytes  = 2;
        }
        else if (*pBuf < 0xF0)
        {
            codePoint = *pBuf & 0x0F;
            lenBytes  = 3;
        }
        else if (*pBuf < 0xF8)
        {
            codePoint = *pBuf & 0x07;
            lenBytes  = 4;
        }
        else if (*pBuf < 0xFC)
        {
            codePoint = *pBuf & 0x03;
            lenBytes  = 5;
        }
        else if (*pBuf < 0xFE)
        {
            codePoint = *pBuf & 0x01;
            lenBytes  = 6;
        }
        else
        {
            // Invalid leading byte
            codePoint = UNICODE_REPLACEMENT_CHARACTER;
            lenBytes  = 1;
        }

        for (unsigned int i = 1; i < lenBytes; ++i)
        {
            if ((*(pBuf + i) & 0xC0) == 0x80)
                codePoint = (codePoint << 6) | (*(pBuf + i) & 0x3F);
            else
            {
                // Invalid continuation byte
                codePoint = UNICODE_REPLACEMENT_CHARACTER;
                break;
            }
        }

        if (codePoint == UNICODE_REPLACEMENT_CHARACTER)
        {
            lenCharacters = 1;
            wcharBuf[0]   = static_cast<wchar_t>(codePoint);
        }
        else
        {
            if (codePoint < SUPPLEMENTAL_PLANE_FIRST)
            {
                wcharBuf[0]   = static_cast<wchar_t>(codePoint);
                lenCharacters = 1;
            }
            else
            {
                wcharBuf[0]   = static_cast<wchar_t>(((codePoint - SUPPLEMENTAL_PLANE_FIRST) >> 10) + SURROGATE_LEAD_FIRST);
                wcharBuf[1]   = static_cast<wchar_t>((codePoint & 0x3ff) + SURROGATE_TRAIL_FIRST);
                lenCharacters = 2;
            }
        }
    }

    const unsigned char      *text;
    std::ptrdiff_t            position;
    size_t                    characterIndex;

    // members to cache the current codepoint in utf-16
    unsigned int              lenBytes;
    size_t                    lenCharacters;
    wchar_t                   wcharBuf[2];

    static constexpr char32_t SUPPLEMENTAL_PLANE_FIRST      = 0x10000;
    static constexpr char32_t SURROGATE_LEAD_FIRST          = 0xD800;
    static constexpr char32_t SURROGATE_TRAIL_FIRST         = 0xDC00;
    static constexpr char32_t UNICODE_REPLACEMENT_CHARACTER = 0xFFFD;
};
