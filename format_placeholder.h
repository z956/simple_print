/*
 * Copyright (c) 2019, Li-Wei Cheng. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *     * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *     * Neither the name of the SimplePrint Project nor the
 *       names of its contributors may be used to endorse or promote products
 *       derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL PETER THORSON BE LIABLE FOR ANY
 * DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND
 * ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 */

#pragma once

#include <exception>
#include <string>

namespace simple_print
{

class format_placeholder
{
public:
    enum Flags
    {
        MINUS = 0x1,
        PLUS = 0x2,
        SPACE = 0x4,
        ZERO = 0x8,
        HASH = 0x10,
    };

    enum Type
    {
        TYPE_DEFAULT,
        TYPE_SIGNED_INT,
        TYPE_UNSIGNED_INT,
        TYPE_OCT,
        TYPE_HEX_UPPER,
        TYPE_HEX_LOWER,
    };

    enum NumFieldStatus
    {
        NUM_FIELD_DEFAULT,
        NUM_FIELD_SPECIFIED,
        NUM_FIELD_CUSTOMIZE,
    };

    struct num_field
    {
        NumFieldStatus status;
        // num only valid when status is NumFieldStatus::NUM_FIELD_SPECIFIED
        int num;

        num_field()
            : status(NUM_FIELD_DEFAULT), num(-1)
        {
        }
    };

public:
    format_placeholder(const char* spec)
        : param(-1)
        , type(TYPE_DEFAULT)
    {
        parse(spec);
    }

    int get_param() const
    {
        return param;
    }

    unsigned get_flags() const
    {
        return flags;
    }

    Type get_type() const
    {
        return type;
    }

    const num_field& get_width() const
    {
        return width;
    }

    const num_field& get_precision() const
    {
        return precision;
    }

private:
    using parse_func = std::size_t (format_placeholder::*)(const std::string&, std::size_t);

    void parse(const std::string& spec)
    {
        static parse_func tbl[] =
        {
            &format_placeholder::parse_param,
            &format_placeholder::parse_flags,
            &format_placeholder::parse_width,
            &format_placeholder::parse_precision,
            &format_placeholder::parse_type,
        };

        static int tbl_size = sizeof(tbl) / sizeof(parse_func);
        std::size_t pos = 0;

        for (int i = 0; i < tbl_size; i++)
        {
            if (pos >= spec.length())
            {
                break;
            }

            parse_func f = tbl[i];
            pos = (this->*f)(spec, pos);
        }
    }

    std::size_t parse_param(const std::string& spec, std::size_t start)
    {
        std::size_t pos = spec.find('$', start);
        if (pos == std::string::npos)
        {
            return start;
        }

        try
        {
            param = std::stoi(spec.substr(start, pos));
        }
        catch (const std::exception& e)
        {
            // skip param
        }
        return pos + 1;
    }

    std::size_t parse_flags(const std::string& spec, std::size_t start)
    {
        std::size_t pos = start;
        for (; pos != spec.length(); pos++)
        {
            switch (spec.at(pos))
            {
            case '-':
                flags |= Flags::MINUS;
                break;
            case '+':
                flags |= Flags::PLUS;
                break;
            case ' ':
                flags |= Flags::SPACE;
                break;
            case '0':
                flags |= Flags::ZERO;
                break;
            case '#':
                flags |= Flags::HASH;
                break;
            default:
                return pos;
            }
        }
        return pos;
    }

    std::size_t parse_width(const std::string& spec, std::size_t start)
    {
        std::size_t pos = spec.find_first_not_of("0123456789*", start);
        if (pos == std::string::npos)
        {
            pos = spec.length();
        }

        parse_num_field(width, spec, start, pos);
        return pos;
    }

    std::size_t parse_precision(const std::string& spec, std::size_t start)
    {
        if (spec.front() != '.')
        {
            return start;
        }

        start += 1;
        std::size_t pos = spec.find_first_not_of("0123456789*", start);
        if (pos == std::string::npos)
        {
            pos = spec.length();
        }
        parse_num_field(precision, spec, start, pos);
        return pos;
    }

    std::size_t parse_type(const std::string& spec, std::size_t start)
    {
        char t = spec.front();
        switch (t)
        {
        case 'd':
        case 'i':
            type = TYPE_SIGNED_INT;
            start++;
            break;
        case 'u':
            type = TYPE_UNSIGNED_INT;
            start++;
            break;
        case 'o':
            type = TYPE_OCT;
            start++;
            break;
        case 'X':
            type = TYPE_HEX_UPPER;
            start++;
            break;
        case 'x':
            type = TYPE_HEX_LOWER;
            start++;
            break;
        default:
            break;
        }
        return start;
    }

    void parse_num_field(num_field& field, const std::string& spec, std::size_t start, std::size_t end)
    {
        // remove leading zero because they belongs to flags
        while (start < end && spec.at(start) == '0')
        {
            start++;
        }
        if (start == end)
        {
            return;
        }

        // the range [start, end) only contains 0123456789*
        // the '*' and numbers are disjoin
        if (spec.at(start) == '*' && end - start == 1)
        {
            field.status = NUM_FIELD_CUSTOMIZE;
            return;
        }
        else
        {
            try
            {
                field.num = std::stoi(spec.substr(start, end - start));
                if (field.num > 0)
                {
                    field.status = NUM_FIELD_SPECIFIED;
                }
            }
            catch (const std::exception& e)
            {
                field.num = -1;
            }
        }
    }

private:
    int param;
    unsigned flags;
    Type type;
    num_field width;
    num_field precision;
};

}
