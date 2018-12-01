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

#ifndef VTDEC_PROCESSOR_H
#define VTDEC_PROCESSOR_H

namespace vtdec
{

/**
 * A decoded event processor. Implement this to receive decode feedback.
 */
struct processor
{
    /**
     * Printable codepoint passthrough.
     *
     * @param c The codepoint value
     */
    virtual void print(char32_t c)
    {
    }

    /**
     * A single-codepoint control has been issued.
     *
     * @param c The codepoint value
     */
    virtual void ctl(char c)
    {
    }

    /**
     * A control sequence has begun.
     */
    virtual void ctl_begin()
    {
    }

    /**
     * A codepoint has arrived as part of a control sequence.
     *
     * @param c The codepoint value
     */
    virtual void ctl_put(char32_t c)
    {
    }

    /**
     * A control sequence has ended.
     *
     * @param cancel True on cancellation, otherwise false
     */
    virtual void ctl_end(bool cancel)
    {
    }

    /**
     * A device control string (DCS) has begun.
     */
    virtual void dcs_begin()
    {
    }

    /**
     * A codepoint has arrived as part of a device control string (DCS).
     *
     * @param c The codepoint value
     */
    virtual void dcs_put(char32_t c)
    {
    }

    /**
     * A device control string (DCS) has ended.
     *
     * @param cancel True on cancellation, otherwise false
     */
    virtual void dcs_end(bool cancel)
    {
    }

    /**
     * An operating system command (OSC) string has begun.
     */
    virtual void osc_begin()
    {
    }

    /**
     * A codepoint has arrived as part of an operating system command (OSC).
     *
     * @param c The codepoint value
     */
    virtual void osc_put(char32_t c)
    {
    }

    /**
     * An operating system command (OSC) string has ended
     *
     * @param cancel True on cancellation, otherwise false
     */
    virtual void osc_end(bool cancel)
    {
    }

    /**
     * A decode operation has begun.
     */
    virtual void decode_begin()
    {
    }

    /**
     * A codepoint has arrived as part of a decode operation.
     *
     * @param c The codepoint value
     */
    virtual void decode_put(char32_t c)
    {
    }

    /**
     * An action is about to be performed as part of a decode operation.
     *
     * @param act The impending action
     */
    virtual void decode_action(int act)
    {
    }

    /**
     * A transition is about to be made as part of a decode operation.
     *
     * @param src The source state
     * @param dst The destination state
     */
    virtual void decode_transition(int src, int dst)
    {
    }

    /**
     * A decode operation has ended.
     *
     * @param cancel True on cancellation, otherwise false
     */
    virtual void decode_end(bool cancel)
    {
    }

    virtual ~processor() = default;
};

} // namespace vtdec

#endif // #ifndef VTDEC_PROCESSOR_H
