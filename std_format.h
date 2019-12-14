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

#include <string>
#include <map>

namespace simple_print
{

template<>
struct format_type<std::string>
{
    static std::string format(const std::string& s)
    {
        return s;
    }
};

template<typename K, typename V>
struct format_type<std::map<K, V>>
{
    static std::string format(const std::map<K, V>& m)
    {
        std::string result;
        result += "map(";
        for (auto& p : m)
        {
            result += " {";
            result += format_type<K>::format(p.first);
            result += ", ";
            result += format_type<V>::format(p.second);
            result += "},";
        }

        if (m.size() > 0)
        {
            result.erase(result.end() - 1);
        }

        result += " )";
        return result;
    }
};

template<typename V>
struct format_type<std::vector<V>>
{
    static std::string format(const std::vector<V>& v)
    {
        std::string result;
        result += "vector( ";
        for (auto& value : v)
        {
            result += format_type<V>::format(value);
            result += ", ";
        }
        if (v.size() > 0)
        {
            result.erase(result.end() - 2);
        }
        result += ")";

        return result;
    }
};

}
