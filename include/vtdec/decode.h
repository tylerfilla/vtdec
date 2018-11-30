/*
 * vtdec
 * Copyright 2018 Tyler Filla
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Huge thanks to Joshua Haberman for vtparse and Paul Williams for the state
 * machine underlying vtdec. All third-party contributions made to vtparse are
 * assumed to have been dedicated to the public domain.
 */

#ifndef VTDEC_DECODE_H
#define VTDEC_DECODE_H

#include <string_view>
#include <type_traits>

namespace vtdec
{

struct processor;

/**
 * Transient state for an ongoing decode operation.
 */
class decode_state
{
    int state;
    int sequence;
};

namespace details
{

/**
 * @private
 */
template<class Processor>
decode_state put(char32_t c, Processor&& proc, decode_state& state)
{
    static_assert(std::is_base_of_v<processor, std::decay_t<Processor>>, "proc not a vtdec::processor");

    // TODO

    return state;
}

} // namespace details

/**
 * Decode the given single-octet codepoint.
 *
 * @tparam Processor The processor type
 * @param c The codepoint
 * @param proc The target processor (optional)
 * @param state An initial state (optional)
 * @return The residual state
 */
template<class Processor>
decode_state decode(char c, Processor&& proc = {}, decode_state state = {})
{
    return details::put(static_cast<char32_t>(c), proc, state);
}

/**
 * Decode the given 32-bit codepoint.
 *
 * @tparam Processor The processor type
 * @param c The codepoint
 * @param proc The target processor (optional)
 * @param state An initial state (optional)
 * @return The residual state
 */
template<class Processor>
decode_state decode(char32_t c, Processor&& proc = {}, decode_state state = {})
{
    return details::put(c, proc, state);
}

/**
 * Decode a collection of input codepoints.
 *
 * Valid codepoint types are char, char32_t, and anything that may implicitly
 * convert to one of these.
 *
 * @tparam Processor The processor type
 * @tparam InputIter The iterator type
 * @param begin An iterator to the codepoint collection
 * @param end An iterator one past the end of the codepoint collection
 * @param proc The target processor (optional)
 * @param state An initial state (optional)
 * @return The residual state
 */
template<class Processor, class InputIter>
decode_state decode(InputIter begin, InputIter end, Processor&& proc = {}, decode_state state = {})
{
    for (auto i = begin; i != end; ++i)
    {
        state = decode(*i, proc, state);
    }
    return state;
}

/**
 * Decode a string of single-octet input codepoints.
 *
 * @tparam Processor The processor type
 * @param str A view of the input string
 * @param proc The target processor (optional)
 * @param state An initial state (optional)
 * @return The residual state
 */
template<class Processor>
decode_state decode(std::string_view str, Processor&& proc = {}, decode_state state = {})
{
    for (char c : str)
    {
        state = details::put(static_cast<char32_t>(c), proc, state);
    }
    return state;
}

/**
 * Decode a string of 32-bit input codepoints.
 *
 * @tparam Processor The processor type
 * @param str A view of the input string
 * @param proc The target processor (optional)
 * @param state An initial state (optional)
 * @return The residual state
 */
template<class Processor>
decode_state decode(std::u32string_view str, Processor&& proc = {}, decode_state state = {})
{
    for (char32_t c : str)
    {
        state = details::put(c, proc, state);
    }
    return state;
}

} // namespace vtdec

#endif // #ifndef VTDEC_DECODE_H
