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

#include <functional>
#include <memory>
#include <string_view>

namespace vtdec
{

struct decoder_private;

/**
 * A stream decoder for DEC-compatible VT escape codes. A state machine is used
 * to break the stream into small pieces, and the pieces are sent out to a set
 * of user-assigned callback functions for more processing.
 *
 * This class concerns itself only with escape code syntax, and it does not
 * allocate any memory or buffer any data. This class does not concern itself
 * with interpretation of sequences (character sets, DCSes, OSCs, etc.). Think
 * of these as exercises for the reader :)
 */
class decoder
{
public:
    /**
     * A callback for printing a character.
     *
     * @param c The character
     */
    using cb_act_print = std::function<void(char c)>;

    /**
     * Events regarding a character sequence.
     */
    enum class sequence_event
    {
        /** The sequence has begun. */
        begin,

        /** The sequence was cancelled prematurely. */
        cancel,

        /** The sequence has ended. */
        end,

        /** A character has been appended to the sequence. */
        put,

        /** The sequence contains just one character. */
        single,
    };

    /**
     * A callback for control sequence events.
     *
     * @param event The sequence event
     * @param c The character value (only used for "put" and "single" events)
     */
    using cb_act_ctl = std::function<void(sequence_event event, char c)>;

    /**
     * A callback for DCS (device control string) events.
     *
     * Sequence event "single" is never used.
     *
     * @param event The sequence event
     * @param c The character value (only used for "put" events)
     */
    using cb_act_dcs = std::function<void(sequence_event event, char c)>;

    /**
     * A callback for OSC (operating system command) events.
     *
     * Sequence event "single" is never used.
     *
     * @param event The sequence event
     * @param c The character value (only used for "put" events)
     */
    using cb_act_osc = std::function<void(sequence_event event, char c)>;

    /**
     * A diagnostic callback for skimming actions.
     *
     * @param state The current state
     * @param action The action to be performed
     * @param c The character value prompting the action
     */
    using cb_diag_action = std::function<void(int state, int action, char c)>;

    /**
     * A diagnostic callback for state transitions.
     *
     * @param current The current state
     * @param target The target state
     * @param c The character value prompting the transition
     */
    using cb_diag_tran = std::function<void(int current, int target, char c)>;

    /**
     * A diagnostic callback for ingesting character data.
     *
     * @param state The current state
     * @param c The ingested character
     */
    using cb_diag_char = std::function<void(int state, char c)>;

private:
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
    /**
     * Do an action.
     *
     * @param action The action index
     * @param c The character value prompting the action
     */
    void do_action(int action, char c);

    /**
     * Make a transition.
     *
     * @param target The target state index
     * @param c The character value prompting the transition
     */
    void do_transition(int target, char c);

    /**
     * Ingest a character.
     *
     * @param c The character value
     */
    void ingest(char c);

public:
    /**
     * @return A callback for printing a character
     */
    cb_act_print get_cb_act_print() const;

    /**
     * @param p_cb_act_print A callback for printing a character
     */
    void set_cb_act_print(cb_act_print p_cb_act_print);

    /**
     * @return A callback for control sequence events
     */
    cb_act_ctl get_cb_act_ctl() const;

    /**
     * @param p_cb_act_ctl A callback for control sequence events
     */
    void set_cb_act_ctl(cb_act_ctl p_cb_act_ctl);

    /**
     * @return A callback for DCS (device control string) events
     */
    cb_act_dcs get_cb_act_dcs() const;

    /**
     * @param p_cb_act_dcs A callback for DCS (device control string) events
     */
    void set_cb_act_dcs(cb_act_dcs p_cb_act_dcs);

    /**
     * @return A callback for OSC (operating system command) events
     */
    cb_act_osc get_cb_act_osc() const;

    /**
     * @param p_cb_act_osc A callback for OSC (operating system command) events
     */
    void set_cb_act_osc(cb_act_osc p_cb_act_osc);

    /**
     * @return A diagnostic callback for skimming actions
     */
    cb_diag_action get_cb_diag_action() const;

    /**
     * @param p_cb_diag_action A diagnostic callback for skimming actions
     */
    void set_cb_diag_action(cb_diag_action p_cb_diag_action);

    /**
     * @return A diagnostic callback for state transitions
     */
    cb_diag_tran get_cb_diag_tran() const;

    /**
     * @param p_cb_diag_tran A diagnostic callback for state transitions
     */
    void set_cb_diag_tran(cb_diag_tran p_cb_diag_tran);

    /**
     * @return A diagnostic callback for ingesting character data
     */
    cb_diag_char get_cb_diag_char() const;

    /**
     * @param p_cb_diag_char A diagnostic callback for ingesting character data
     */
    void set_cb_diag_char(cb_diag_char p_cb_diag_char);

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
    std::size_t put(char c);

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
