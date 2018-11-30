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
 * Huge thanks to Joshua Haberman for vtparse and Paul Williams for the action
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
     * Unescaped codepoint passthrough.
     *
     * @param c The codepoint value
     */
    virtual void passthrough(char32_t c)
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
     *
     * Part of the diagnostic callback suite.
     */
    virtual void diag_decode_begin()
    {
    }

    /**
     * A codepoint has arrived as part of a decode operation.
     *
     * Part of the diagnostic callback suite.
     *
     * @param c The codepoint value
     */
    virtual void diag_decode_put(char32_t c)
    {
    }

    /**
     * A decode operation has ended.
     *
     * Part of the diagnostic callback suite.
     *
     * @param cancel True on cancellation, otherwise false
     */
    virtual void diag_decode_end(bool cancel)
    {
    }

    /**
     * The decoder is about to perform an action.
     *
     * Part of the diagnostic callback suite.
     *
     * @param act The impending action
     */
    virtual void diag_action(int act)
    {
    }

    /**
     * The decoder is about to perform an action due to a state being entered.
     *
     * Part of the diagnostic callback suite.
     *
     * @param act The impending action
     * @param reason The state prompting the action
     */
    virtual void diag_action_enter(int act, int reason)
    {
        // Delegate to catch-all handler
        diag_action(act);
    }

    /**
     * The decoder is about to perform an action due to a state being left.
     *
     * Part of the diagnostic callback suite.
     *
     * @param act The impending action
     * @param reason The state prompting the action
     */
    virtual void diag_action_leave(int act, int reason)
    {
        // Delegate to catch-all handler
        diag_action(act);
    }

    /**
     * The decoder is about to perform an action due to a codepoint having been
     * received in some state. This action is not due to an entry/leave event.
     *
     * Part of the diagnostic callback suite.
     *
     * @param act The impending action
     * @param reason The codepoint prompting the action
     */
    virtual void diag_action_char(int act, char32_t reason)
    {
        // Delegate to catch-all handler
        diag_action(act);
    }

    /**
     * The decoder is about to make a state transition.
     *
     * Part of the diagnostic callback suite.
     *
     * @param src The source state
     * @param dst The destination state
     * @param reason The codepoint prompting the transition
     */
    virtual void diag_transition(int src, int dst, char32_t reason)
    {
    }

    /**
     * @return True if, and only if, diagnostic callbacks are enabled
     */
    bool is_diag() const
    { return m_diag; }

protected:
    /**
     * @param p_diag True if, and only if, diagnostic callbacks are enabled
     */
    void set_diag(bool p_diag)
    { m_diag = p_diag; }

private:
    /**
     * True if, and only if, diagnostic callbacks are enabled.
     */
    bool m_diag = false;
};

} // namespace vtdec

#endif // #ifndef VTDEC_PROCESSOR_H
