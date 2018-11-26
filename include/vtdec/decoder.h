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

#ifndef VTDEC_DECODER_H
#define VTDEC_DECODER_H

#include <memory>
#include <string_view>

namespace vtdec
{

struct decoder_private;

/**
 * A stream decoder for DEC-compatible VT escape codes.
 */
class decoder
{
    /** Private. */
    std::unique_ptr<decoder_private> m_private;

public:
    decoder();

    decoder(const decoder& rhs);

    decoder(decoder&& rhs) noexcept;

    ~decoder();

    decoder& operator=(const decoder& rhs);

    decoder& operator=(decoder&& rhs) noexcept;

private:
    void ingest(int c);

public:
    /**
     * @return The current state
     */
    int get_state() const;

    /**
     * Put data into the decoder.
     *
     * @param c A single character value
     * @return The number of chars processed
     */
    std::size_t put(int c);

    /**
     * Put data into the decoder.
     *
     * @param data The data buffer
     * @param length The data buffer length
     * @return The number of chars processed
     */
    std::size_t put(const char* data, std::size_t length);

    /**
     * Put data into the decoder.
     *
     * @param str The data string
     * @return The number of chars processed
     */
    std::size_t put(std::string_view str)
    { return put(str.data(), str.length()); }
};

} // namespace vtdec

#endif // #ifndef VTDEC_DECODER_H
