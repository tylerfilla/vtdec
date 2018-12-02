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

#include <vtdec/processor.h>
#include <vtdec/table.h>

namespace vtdec
{

/**
 * Transient state for an ongoing decode operation.
 */
struct decode_state
{
    /** The index of the current state. */
    int state;

    /** The index of the current sequence. */
    int sequence;
};

/**
 * Implementation details.
 */
namespace detail
{

/**
 * A kind of sequence.
 */
enum sequence
{
    idk,
    ctl,
    dcs,
    osc,
};

/**
 * Internal. Carry out an action.
 */
template<class Processor>
static decode_state do_action(Processor&& p, decode_state s, int act, char c)
{
    p.decode_action(act);

    // Perform the action
    switch (act)
    {
    default:
    case action::ignore:
        // This line intentionally left blank
        break;
    case action::print:
        // Pass through printable codepoint
        p.print(c);
        break;
    case action::execute:
        // A single-codepoint control
        p.ctl(c);
        break;
    case action::clear:
        // Cancel the current sequence
        switch (s.sequence)
        {
        case sequence::idk:
            // No sequence to cancel
            break;
        case sequence::ctl:
            p.ctl_end(true);
            break;
        case sequence::dcs:
            p.dcs_end(true);
            break;
        case sequence::osc:
            p.osc_end(true);
            break;
        default:
            throw std::runtime_error {"illegal sequence"};
        }

        // This is our only chance to call the beginnings of certain sequences
        // Look at the current state to figure out which sequence, if any, just began
        switch (s.state)
        {
        case state::csi_entry:
            // Begin control sequence
            p.ctl_begin();
            s.sequence = sequence::ctl;
            break;
        case state::dcs_entry:
            // Begin device control sequence
            p.dcs_begin();
            s.sequence = sequence::dcs;
            break;
        default:
            // Clear current sequence
            s.sequence = sequence::idk;
            break;
        }
        break;
    case action::collect:
        // The details of this action vary according to the sequence
        // Luckily, we already made our transition, so we can look at the state
        switch (s.state)
        {
        case state::csi_intermediate:
        case state::csi_param:
            // Append to control sequence
            p.ctl_put(c);
            break;
        case state::dcs_intermediate:
        case state::dcs_param:
            // Append to device control sequence
            p.dcs_put(c);
            break;
        case state::escape_intermediate:
            // Begin control sequence
            p.ctl_begin();
            s.sequence = sequence::ctl;
            break;
        default:
            throw std::runtime_error {"illegal state"};
        }
        break;
    case action::param:
        // Append to appropriate sequence
        switch (s.sequence)
        {
        case sequence::ctl:
            // Append to control sequence
            p.ctl_put(c);
            break;
        case sequence::dcs:
            // Append to device control sequence
            p.dcs_put(c);
            break;
        case sequence::idk:
        case sequence::osc:
        default:
            throw std::runtime_error {"illegal sequence"};
        }
        break;
    case action::esc_dispatch:
        // End control sequence
        p.ctl_end(false);
        s.sequence = sequence::idk;
        break;
    case action::csi_dispatch:
        // End control sequence
        p.ctl_end(false);
        s.sequence = sequence::idk;
        break;
    case action::hook:
    case action::put:
        // Append to device control sequence
        p.dcs_put(c);
        break;
    case action::unhook:
        // End device control string
        p.dcs_end(false);
        s.sequence = sequence::idk;
        break;
    case action::osc_start:
        // Begin operating system command
        p.osc_begin();
        s.sequence = sequence::osc;
        break;
    case action::osc_put:
        // Continue operating system command
        p.osc_put(c);
        break;
    case action::osc_end:
        // End operating system command
        p.osc_end(false);
        s.sequence = sequence::idk;
        break;
    }

    return s;
}

/**
 * Internal. Carry out a transition.
 */
template<class Processor>
static decode_state do_transition(Processor&& p, decode_state s, int tgt, char c)
{
    p.decode_transition(s.state, tgt);

    // Look up predicate for leaving current state
    auto& pred_leave = table[s.state][129];

    // Do leave action if one is to be taken
    if (pred_leave.action > action::none)
    {
        s = do_action(p, s, pred_leave.action, c);
    }

    // Make the transition
    s.state = tgt;

    // Look up predicate for entering target state
    auto& pred_enter = table[s.state][128];

    // Do enter action if one is to be taken
    if (pred_enter.action > action::none)
    {
        s = do_action(p, s, pred_enter.action, c);
    }

    return s;
}

/**
 * Internal. Unchecked put of a single-octet codepoint.
 */
template<class Processor>
decode_state put(char c, Processor&& p, decode_state s)
{
    // Look up predicate for this codepoint in this state
    auto& pred = table[s.state][c];

    // Do transition if one is to be made
    if (pred.target > state::none)
    {
        s = do_transition(p, s, pred.target, c);
    }

    // Do action if one is to be taken
    if (pred.action > action::none)
    {
        s = do_action(p, s, pred.action, c);
    }

    return s;
}

/**
 * Internal. Unchecked put of a single-octet codepoint. Overload alias for put.
 */
template<class Processor>
decode_state put_one(char c, Processor&& p, decode_state s)
{
    p.decode_put(c);
    return put(c, p, s);
}

/**
 * Internal. Unchecked put of a 32-bit codepoint.
 */
template<class Processor>
decode_state put_one(char32_t c, Processor&& p, decode_state s)
{
    p.decode_put(c);

    // Get least significant byte of codepoint
    // For all VT controls, we only care about this byte
    auto lsb = static_cast<char>(c & 0xff);

    // If the least significant byte equals the whole thing
    if (lsb == c)
    {
        // The codepoint only occupies one octet
        // So, we process it as a single-octet codepoint
        s = put(lsb, p, s);
    }
    else
    {
        // The codepoint occupies more than one octet
        // This is not an active codepoint (i.e. it cannot trigger an action)

        /**
         * A special proxy processor for substituting printable characters in
         * decode events. This is used for large (i.e. non-ASCII) codepoint
         * support.
         */
        class substitutor : public processor
        {
            /** The original processor. */
            Processor&& m_proc;

            /** The original printable codepoint. */
            char32_t m_orig;

        public:
            substitutor(Processor&& p_proc, char32_t p_orig)
                    : m_proc {p_proc}
                    , m_orig {p_orig}
            {
            }

            void print(char32_t) final
            { m_proc.passthrough(m_orig); }

            void ctl(char c) final
            { m_proc.ctl_put(c); }

            void ctl_begin() final
            { m_proc.ctl_begin(); }

            void ctl_put(char32_t) final
            { m_proc.ctl_put(m_orig); }

            void ctl_end(bool cancel) final
            { m_proc.ctl_end(cancel); }

            void dcs_begin() final
            { m_proc.dcs_begin(); }

            void dcs_put(char32_t) final
            { m_proc.dcs_put(m_orig); }

            void dcs_end(bool cancel) final
            { m_proc.dcs_end(cancel); }

            void osc_begin() final
            { m_proc.osc_begin(); }

            void osc_put(char32_t) final
            { m_proc.osc_put(m_orig); }

            void osc_end(bool cancel) final
            { m_proc.osc_end(cancel); }

            void decode_begin() final
            { m_proc.decode_begin(); }

            void decode_put(char32_t) final
            { m_proc.decode_put(m_orig); }

            void decode_action(int act) final
            { m_proc.decode_action(act); }

            void decode_transition(int src, int dst) final
            { m_proc.decode_transition(src, dst); }

            void decode_end(bool cancel) final
            { m_proc.decode_end(cancel); }
        };

        // Pass the large codepoint off as a space character and proxy the results
        // This takes care of all printable codepoints more complicated than ASCII
        s = put(' ', substitutor {p, c}, s);
    }

    return s;
}

/**
 * Internal. Unchecked put over a range of character things.
 */
template<class Processor, class InputIter>
decode_state put_range(InputIter begin, InputIter end, Processor&& p, decode_state s)
{
    for (auto i = begin; i != end; ++i)
    {
        s = put_one(*i, p, s);
    }

    return s;
}

} // namespace detail

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
    static_assert(std::is_base_of_v<processor, std::decay_t<Processor>>, "parameter 'proc' not a vtdec::processor");

    proc.decode_begin();
    state = detail::put_one(c, proc, state);
    proc.decode_end(false);

    return state;
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
    static_assert(std::is_base_of_v<processor, std::decay_t<Processor>>, "parameter 'proc' not a vtdec::processor");

    proc.decode_begin();
    state = detail::put_one(c, proc, state);
    proc.decode_end(false);

    return state;
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
    static_assert(std::is_base_of_v<processor, std::decay_t<Processor>>, "parameter 'proc' not a vtdec::processor");

    proc.decode_begin();
    state = detail::put_range(begin, end, proc, state);
    proc.decode_end(false);

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
    static_assert(std::is_base_of_v<processor, std::decay_t<Processor>>, "parameter 'proc' not a vtdec::processor");

    proc.decode_begin();
    state = detail::put_range(str.begin(), str.end(), proc, state);
    proc.decode_end(false);

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
    static_assert(std::is_base_of_v<processor, std::decay_t<Processor>>, "parameter 'proc' not a vtdec::processor");

    proc.decode_begin();
    state = detail::put_range(str.begin(), str.end(), proc, state);
    proc.decode_end(false);

    return state;
}

} // namespace vtdec

#endif // #ifndef VTDEC_DECODE_H
