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

#include <vtdec/decoder.h>

namespace vtdec
{

struct decoder_private
{
    /** The current state. */
    int m_state {-1};
};

decoder::decoder() : m_private {}
{
    m_private = std::make_unique<decoder_private>();
}

decoder::decoder(const decoder& rhs) : m_private {}
{
    m_private = std::make_unique<decoder_private>(*rhs.m_private);
}

decoder::decoder(decoder&& rhs) noexcept : m_private {}
{
    m_private = std::move(rhs.m_private);
}

decoder::~decoder() = default;

decoder& decoder::operator=(const decoder& rhs)
{
    m_private = std::make_unique<decoder_private>(*rhs.m_private);
    return *this;
}

decoder& decoder::operator=(decoder&& rhs) noexcept
{
    m_private = std::move(rhs.m_private);
    return *this;
}

int decoder::get_state() const
{
    return m_private->m_state;
}

std::size_t decoder::put(const char* data, std::size_t length)
{
    // TODO
    return 0;
}

} // namespace vtdec
