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
 * Huge thanks to Joshua Haberman for vtparse, the public domain codebase on
 * which vtdec is built! All third-party contributions made to vtparse are also
 * assumed to be dedicated to the public domain.
 */

#ifndef VTDEC_DECODER_H
#define VTDEC_DECODER_H

#include "vtparse_table.h"

#define MAX_INTERMEDIATE_CHARS 2
#define ACTION(state_change) (state_change & 0x0F)
#define STATE(state_change) (state_change >> 4)

namespace vtdec
{

/**
 * A vt decoder.
 */
class Decoder
{
public:
    Decoder();

    Decoder(const Decoder& rhs);

    Decoder(Decoder&& rhs) noexcept;

    ~Decoder();

    Decoder& operator=(const Decoder& rhs);

    Decoder& operator=(Decoder&& rhs) noexcept;
};

struct vtparse;

typedef void (*vtparse_callback_t)(struct vtparse*, vtparse_action_t, unsigned int);

typedef struct vtparse
{
    vtparse_state_tstate;
    vtparse_callback_t cb;
    unsigned char intermediate_chars[MAX_INTERMEDIATE_CHARS + 1];
    int num_intermediate_chars;
    char ignore_flagged;
    int params[16];
    int num_params;
    void* user_data;
    int characterBytes;
    unsigned intutf8Character;
}
vtparse_t;

void vtparse_init(vtparse_t* parser, vtparse_callback_t cb);

void vtparse(vtparse_t* parser, unsigned char* data, unsigned int len);

} // namespace vtdec

#endif // #ifndef VTDEC_DECODER_H
