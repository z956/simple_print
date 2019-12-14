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

#include <functional>
#include <sstream>
#include <string>
#include <type_traits>
#include <iostream>

#include "utils.h"

namespace simple_print
{

template<typename T, typename Enable = void>
struct format_type;

template<>
struct format_type<char>
{
    static std::string format(char c)
    {
        return std::string(1, c);
    }
};

template<typename Int>
struct format_type<Int, typename std::enable_if<std::is_integral<Int>::value>::type>
{
    static std::string format(Int i)
    {
        return std::to_string(i);
    }
};

template<typename Float>
struct format_type<Float, typename std::enable_if<std::is_floating_point<Float>::value>::type>
{
    static std::string format(Float f)
    {
        return std::to_string(f);
    }
};

template<>
struct format_type<const char*>
{
    static std::string format(const char* s)
    {
        return s;
    }
};

template<typename T>
struct format_type<T*>
{
    static std::string format(const T* p)
    {
        uintptr_t uptr = reinterpret_cast<uintptr_t>(p);
        std::stringstream ss;
        ss << std::hex << uptr;
        return "0x" + ss.str();
    }
};

struct format_args
{
    std::function<std::string()> f;

    template<typename Int, typename std::enable_if<std::is_integral<Int>::value, int>::type = 0>
    format_args(Int i)
        : f(std::bind(format_type<Int>::format, i))
    {
    }

    template<typename Float, typename std::enable_if<std::is_floating_point<Float>::value, int>::type = 0>
    format_args(Float f)
        : f(std::bind(format_type<Float>::format, f))
    {
    }

    template<typename CString, typename std::enable_if<is_cstring<CString>::value, int>::type = 0>
    format_args(CString s)
        : f(std::bind(format_type<const char*>::format, s))
    {
    }

    template<typename Pointer, typename std::enable_if<
        std::is_pointer<Pointer>::value &&
        !is_cstring<Pointer>::value, int>::type = 0
    >
    format_args(Pointer p)
        : f(std::bind(format_type<Pointer>::format, p))
    {
    }

    template<
        typename Custom, typename std::enable_if<
            !std::is_arithmetic<Custom>::value &&
            !std::is_pointer<Custom>::value &&
            !is_cstring<Custom>::value, int>::type = 0
    >
    format_args(const Custom& c)
        : f(std::bind(format_type<Custom>::format, std::cref(c)))
    {
    }
};

template<typename... Args>
class formatter
{
    static const std::size_t args_len = sizeof...(Args);

    struct parse_context
    {
        int current_idx;
        bool left_bracket;
        bool right_bracket;
        parse_context()
            : current_idx(0)
            , left_bracket(false)
            , right_bracket(false)
        {
        }
    };

public:
    formatter(const char* fmt, const Args&... args)
        : fmt(fmt)
        , args{args...}
    {
    }

    std::string format() const
    {
        parse_context ctx;
        const char* ptr = fmt;

        std::string result;
        while (*ptr)
        {
            parse(ctx, *ptr, result);
            ptr++;
        }

        return result;
    }

private:
    void parse(parse_context& ctx, char c, std::string& result) const
    {
        if (ctx.left_bracket)
        {
            proc_on_left_bracket(ctx, c, result);
        }
        else if (ctx.right_bracket)
        {
            proc_on_right_bracket(ctx, c, result);
        }
        else if (c == '{')
        {
            ctx.left_bracket = true;
        }
        else if (c == '}')
        {
            ctx.right_bracket = true;
        }
        else
        {
            result.push_back(c);
        }
    }
    void proc_on_left_bracket(parse_context& ctx, char c, std::string& result) const
    {
        if (c == '{')
        {
            result.push_back(c);
            ctx.left_bracket = false;
        }
        else if (c == '}')
        {
            if (ctx.current_idx >= args_len)
            {
                result.append("<INVALID>");
            }
            else
            {
                result.append(args[ctx.current_idx++].f());
            }
            ctx.left_bracket = false;
        }
        else
        {
            // TODO, placeholder, skip now
        }
    }
    void proc_on_right_bracket(parse_context& ctx, char c, std::string& result) const
    {
        if (c == '}')
        {
            result.push_back(c);
            ctx.right_bracket = false;
        }
        else
        {
            // invalid, just skip
        }
    }

private:
    const char* fmt;
    format_args args[args_len];
};

}
