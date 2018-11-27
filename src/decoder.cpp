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
#include <vtdec/table.h>

namespace vtdec
{

struct decoder_private
{
    /**
     * A kind of sequence.
     */
    struct sequence
    {
        enum
        {
            idk = -1,
            ctl,
            dcs,
            osc,
        };
    };

    /** A callback for printing a character. */
    decoder::cb_act_print m_cb_act_print;

    /** A callback for control sequence events. */
    decoder::cb_act_ctl m_cb_act_ctl;

    /** A callback for DCS (device control string) events. */
    decoder::cb_act_dcs m_cb_act_dcs;

    /** A callback for OSC (operating system command) events. */
    decoder::cb_act_osc m_cb_act_osc;

    /** A diagnostic callback for skimming actions. */
    decoder::cb_diag_action m_cb_diag_action {};

    /** A diagnostic callback for state transitions. */
    decoder::cb_diag_tran m_cb_diag_tran {};

    /** A diagnostic callback for ingesting character data. */
    decoder::cb_diag_char m_cb_diag_char {};

    /** The current state. */
    int m_state {state::ground};

    /** The current sequence. */
    int m_seq {sequence::idk};
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

void decoder::do_action(int action, char c)
{
    // Call back for action
    if (get_cb_diag_action())
    {
        get_cb_diag_action()(get_state(), action, c);
    }

    // Perform action
    switch (action)
    {
    default:
    case action::ignore:
        // This line intentionally left blank
        break;
    case action::print:
        // Print the character
        if (get_cb_act_print())
        {
            get_cb_act_print()(c);
        }
        break;
    case action::execute:
        // Report single-character control
        if (get_cb_act_ctl())
        {
            get_cb_act_ctl()(sequence_event::single, c);
        }
        break;
    case action::clear:
        // Cancel the appropriate sequence
        switch (m_private->m_seq)
        {
        default:
        case decoder_private::sequence::idk:
            // Nothing to cancel
            break;
        case decoder_private::sequence::ctl:
            // Cancel control sequence
            if (get_cb_act_ctl())
            {
                get_cb_act_ctl()(sequence_event::cancel, 0);
            }
            break;
        case decoder_private::sequence::dcs:
            // Cancel device control sequence
            if (get_cb_act_dcs())
            {
                get_cb_act_dcs()(sequence_event::cancel, 0);
            }
            break;
        case decoder_private::sequence::osc:
            // Cancel operating system command
            if (get_cb_act_osc())
            {
                get_cb_act_osc()(sequence_event::cancel, 0);
            }
            break;
        }

        // Make it official
        m_private->m_seq = decoder_private::sequence::idk;
        break;
    case action::collect:
        // TODO
        break;
    case action::param:
        // TODO
        break;
    case action::esc_dispatch:
        // TODO
        break;
    case action::csi_dispatch:
        // TODO
        break;
    case action::hook:
        // TODO
        break;
    case action::put:
        // TODO
        break;
    case action::unhook:
        // TODO
        break;
    case action::osc_start:
        // Begin operating system command
        if (get_cb_act_osc())
        {
            get_cb_act_osc()(sequence_event::begin, c);
        }

        // Remember current sequence
        m_private->m_seq = decoder_private::sequence::osc;
        break;
    case action::osc_put:
        // Continue operating system command
        if (get_cb_act_osc())
        {
            get_cb_act_osc()(sequence_event::put, c);
        }
        break;
    case action::osc_end:
        // End operating system command
        if (get_cb_act_osc())
        {
            get_cb_act_osc()(sequence_event::end, c);
        }

        // Forget sequence
        m_private->m_seq = decoder_private::sequence::idk;
        break;
    }
}

void decoder::do_transition(int target, char c)
{
    // Call back for state transition
    if (get_cb_diag_tran())
    {
        get_cb_diag_tran()(get_state(), target, c);
    }

    // Make the transition
    m_private->m_state = target;
}

void decoder::ingest(char c)
{
    // Call back for character ingest
    if (get_cb_diag_char())
    {
        get_cb_diag_char()(get_state(), c);
    }

    // Look up predicate in table
    auto&& pred = table[get_state()][c];

    // Do action if one is to be taken
    if (pred.action > action::none)
    {
        do_action(pred.action, c);
    }

    // Do transition if one is to be made
    if (pred.target > state::none)
    {
        do_transition(pred.target, c);
    }
}

decoder::cb_act_print decoder::get_cb_act_print() const
{
    return m_private->m_cb_act_print;
}

void decoder::set_cb_act_print(cb_act_print p_cb_act_print)
{
    m_private->m_cb_act_print = p_cb_act_print;
}

decoder::cb_act_ctl decoder::get_cb_act_ctl() const
{
    return m_private->m_cb_act_ctl;
}

void decoder::set_cb_act_ctl(cb_act_ctl p_cb_act_ctl)
{
    m_private->m_cb_act_ctl = p_cb_act_ctl;
}

decoder::cb_act_dcs decoder::get_cb_act_dcs() const
{
    return m_private->m_cb_act_dcs;
}

void decoder::set_cb_act_dcs(cb_act_dcs p_cb_act_dcs)
{
    m_private->m_cb_act_dcs = p_cb_act_dcs;
}

decoder::cb_act_osc decoder::get_cb_act_osc() const
{
    return m_private->m_cb_act_osc;
}

void decoder::set_cb_act_osc(cb_act_osc p_cb_act_osc)
{
    m_private->m_cb_act_osc = p_cb_act_osc;
}

decoder::cb_diag_action decoder::get_cb_diag_action() const
{
    return m_private->m_cb_diag_action;
}

void decoder::set_cb_diag_action(cb_diag_action p_cb_diag_action)
{
    m_private->m_cb_diag_action = p_cb_diag_action;
}

decoder::cb_diag_tran decoder::get_cb_diag_tran() const
{
    return m_private->m_cb_diag_tran;
}

void decoder::set_cb_diag_tran(cb_diag_tran p_cb_diag_tran)
{
    m_private->m_cb_diag_tran = p_cb_diag_tran;
}

decoder::cb_diag_char decoder::get_cb_diag_char() const
{
    return m_private->m_cb_diag_char;
}

void decoder::set_cb_diag_char(cb_diag_char p_cb_diag_char)
{
    m_private->m_cb_diag_char = p_cb_diag_char;
}

int decoder::get_state() const
{
    return m_private->m_state;
}

std::size_t decoder::put(char c)
{
    ingest(c);
    return 1;
}

std::size_t decoder::put(const char* data, std::size_t length)
{
    for (std::size_t i = 0; i < length; ++i)
    {
        ingest(data[i]);
    }

    return length;
}

} // namespace vtdec
