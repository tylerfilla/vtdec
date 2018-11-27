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

#ifndef VTDEC_TABLE_H
#define VTDEC_TABLE_H

#include <array>
#include <utility>

#include <vtdec/action.h>
#include <vtdec/state.h>

namespace vtdec
{

/**
 * An inclusive unsigned character range.
 */
struct table_range
{
    /** The lower bound. */
    unsigned char a;

    /** The upper bound. */
    unsigned char b;

    /**
     * Test whether the range contains a third unsigned character.
     *
     * @param c The test character
     * @return True if such is the case, otherwise false
     */
    constexpr bool contains(unsigned char c) const
    { return a <= c && c <= b; }
};

/**
 * A table predicate.
 */
struct table_predicate
{
    /** An action to perform. */
    int action {action::none};

    /** A state to which to go. */
    int target {state::none};
};

/**
 * A plan for generating the table row of a state.
 *
 * @tparam StateIndex The state index
 */
template<int StateIndex>
struct table_row_plan
{
};

/**
 * A planned predicate to execute on entering a state.
 *
 * @tparam StateIndex The state index
 */
template<int StateIndex>
static constexpr auto table_row_plan_on_enter = table_row_plan<StateIndex>::on_enter;

/**
 * A planned predicate to execute on leaving a state.
 *
 * @tparam StateIndex The state index
 */
template<int StateIndex>
static constexpr auto table_row_plan_on_leave = table_row_plan<StateIndex>::on_leave;

/**
 * An array of planned predicates to execute when putting characters in a state.
 *
 * @tparam StateIndex The state index
 */
template<int StateIndex>
static constexpr auto table_row_plan_on_chars = table_row_plan<StateIndex>::on_chars;

/**
 * State-invariant row plan. These predicates are included in all other rows.
 */
template<>
struct table_row_plan<state::none>
{
    /*
     * Plan:
     *
     * 0x18       => [:execute, transition_to(:GROUND)],
     * 0x1a       => [:execute, transition_to(:GROUND)],
     * 0x80..0x8f => [:execute, transition_to(:GROUND)],
     * 0x91..0x97 => [:execute, transition_to(:GROUND)],
     * 0x99       => [:execute, transition_to(:GROUND)],
     * 0x9a       => [:execute, transition_to(:GROUND)],
     * 0x9c       => transition_to(:GROUND),
     * 0x1b       => transition_to(:ESCAPE),
     * 0x98       => transition_to(:SOS_PM_APC_STRING),
     * 0x9e       => transition_to(:SOS_PM_APC_STRING),
     * 0x9f       => transition_to(:SOS_PM_APC_STRING),
     * 0x90       => transition_to(:DCS_ENTRY),
     * 0x9d       => transition_to(:OSC_STRING),
     * 0x9b       => transition_to(:CSI_ENTRY),
     */

    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x18, 0x18}, table_predicate {action::execute, state::ground}},
            std::pair {table_range {0x1a, 0x1a}, table_predicate {action::execute, state::ground}},
            std::pair {table_range {0x80, 0x8f}, table_predicate {action::execute, state::ground}},
            std::pair {table_range {0x91, 0x97}, table_predicate {action::execute, state::ground}},
            std::pair {table_range {0x99, 0x99}, table_predicate {action::execute, state::ground}},
            std::pair {table_range {0x9a, 0x9a}, table_predicate {action::execute, state::ground}},
            std::pair {table_range {0x9c, 0x9c}, table_predicate {action::none, state::ground}},
            std::pair {table_range {0x1b, 0x1b}, table_predicate {action::none, state::escape}},
            std::pair {table_range {0x98, 0x98}, table_predicate {action::none, state::sos_pm_apc_string}},
            std::pair {table_range {0x9e, 0x9e}, table_predicate {action::none, state::sos_pm_apc_string}},
            std::pair {table_range {0x9f, 0x9f}, table_predicate {action::none, state::sos_pm_apc_string}},
            std::pair {table_range {0x90, 0x90}, table_predicate {action::none, state::dcs_entry}},
            std::pair {table_range {0x9d, 0x9d}, table_predicate {action::none, state::osc_string}},
            std::pair {table_range {0x9b, 0x9b}, table_predicate {action::none, state::csi_entry}},
    };
};

/**
 * State: GROUND
 */
template<>
struct table_row_plan<state::ground>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :execute,
     * 0x19       => :execute,
     * 0x1c..0x1f => :execute,
     * 0x20..0x7f => :print,
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x20, 0x7f}, table_predicate {action::print, state::none}},
    };
};

/**
 * State: ESCAPE
 */
template<>
struct table_row_plan<state::escape>
{
    /*
     * Plan:
     *
     * :on_entry  => :clear,
     * 0x00..0x17 => :execute,
     * 0x19       => :execute,
     * 0x1c..0x1f => :execute,
     * 0x7f       => :ignore,
     * 0x20..0x2f => [:collect, transition_to(:ESCAPE_INTERMEDIATE)],
     * 0x30..0x4f => [:esc_dispatch, transition_to(:GROUND)],
     * 0x51..0x57 => [:esc_dispatch, transition_to(:GROUND)],
     * 0x59       => [:esc_dispatch, transition_to(:GROUND)],
     * 0x5a       => [:esc_dispatch, transition_to(:GROUND)],
     * 0x5c       => [:esc_dispatch, transition_to(:GROUND)],
     * 0x60..0x7e => [:esc_dispatch, transition_to(:GROUND)],
     * 0x5b       => transition_to(:CSI_ENTRY),
     * 0x5d       => transition_to(:OSC_STRING),
     * 0x50       => transition_to(:DCS_ENTRY),
     * 0x58       => transition_to(:SOS_PM_APC_STRING),
     * 0x5e       => transition_to(:SOS_PM_APC_STRING),
     * 0x5f       => transition_to(:SOS_PM_APC_STRING),
     */

    static constexpr auto on_enter = table_predicate {action::clear, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x20, 0x2f}, table_predicate {action::collect, state::escape_intermediate}},
            std::pair {table_range {0x30, 0x4f}, table_predicate {action::esc_dispatch, state::ground}},
            std::pair {table_range {0x51, 0x57}, table_predicate {action::esc_dispatch, state::ground}},
            std::pair {table_range {0x59, 0x59}, table_predicate {action::esc_dispatch, state::ground}},
            std::pair {table_range {0x5a, 0x5a}, table_predicate {action::esc_dispatch, state::ground}},
            std::pair {table_range {0x5c, 0x5c}, table_predicate {action::esc_dispatch, state::ground}},
            std::pair {table_range {0x60, 0x7e}, table_predicate {action::esc_dispatch, state::ground}},
            std::pair {table_range {0x5b, 0x5b}, table_predicate {action::none, state::csi_entry}},
            std::pair {table_range {0x5d, 0x5d}, table_predicate {action::none, state::osc_string}},
            std::pair {table_range {0x50, 0x50}, table_predicate {action::none, state::dcs_entry}},
            std::pair {table_range {0x58, 0x58}, table_predicate {action::none, state::sos_pm_apc_string}},
            std::pair {table_range {0x5e, 0x5e}, table_predicate {action::none, state::sos_pm_apc_string}},
            std::pair {table_range {0x5f, 0x5f}, table_predicate {action::none, state::sos_pm_apc_string}},
    };
};

/**
 * State: ESCAPE_INTERMEDIATE
 */
template<>
struct table_row_plan<state::escape_intermediate>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :execute,
     * 0x19       => :execute,
     * 0x1c..0x1f => :execute,
     * 0x20..0x2f => :collect,
     * 0x7f       => :ignore,
     * 0x30..0x7e => [:esc_dispatch, transition_to(:GROUND)]
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x20, 0x2f}, table_predicate {action::collect, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x30, 0x7e}, table_predicate {action::esc_dispatch, state::ground}},
    };
};

/**
 * State: CSI_ENTRY
 */
template<>
struct table_row_plan<state::csi_entry>
{
    /*
     * Plan:
     *
     * :on_entry  => :clear,
     * 0x00..0x17 => :execute,
     * 0x19       => :execute,
     * 0x1c..0x1f => :execute,
     * 0x7f       => :ignore,
     * 0x20..0x2f => [:collect, transition_to(:CSI_INTERMEDIATE)],
     * 0x3a       => transition_to(:CSI_IGNORE),
     * 0x30..0x39 => [:param, transition_to(:CSI_PARAM)],
     * 0x3b       => [:param, transition_to(:CSI_PARAM)],
     * 0x3c..0x3f => [:collect, transition_to(:CSI_PARAM)],
     * 0x40..0x7e => [:csi_dispatch, transition_to(:GROUND)]
     */

    static constexpr auto on_enter = table_predicate {action::clear, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x20, 0x2f}, table_predicate {action::collect, state::csi_intermediate}},
            std::pair {table_range {0x3a, 0x3a}, table_predicate {action::none, state::csi_ignore}},
            std::pair {table_range {0x30, 0x39}, table_predicate {action::param, state::csi_param}},
            std::pair {table_range {0x3b, 0x3b}, table_predicate {action::param, state::csi_param}},
            std::pair {table_range {0x3c, 0x3f}, table_predicate {action::collect, state::csi_param}},
            std::pair {table_range {0x40, 0x7e}, table_predicate {action::csi_dispatch, state::ground}},
    };
};

/**
 * State: CSI_PARAM
 */
template<>
struct table_row_plan<state::csi_param>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :execute,
     * 0x19       => :execute,
     * 0x1c..0x1f => :execute,
     * 0x30..0x39 => :param,
     * 0x3b       => :param,
     * 0x7f       => :ignore,
     * 0x3a       => transition_to(:CSI_IGNORE),
     * 0x3c..0x3f => transition_to(:CSI_IGNORE),
     * 0x20..0x2f => [:collect, transition_to(:CSI_INTERMEDIATE)],
     * 0x40..0x7e => [:csi_dispatch, transition_to(:GROUND)]
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x30, 0x39}, table_predicate {action::param, state::none}},
            std::pair {table_range {0x3b, 0x3b}, table_predicate {action::param, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x3a, 0x3a}, table_predicate {action::none, state::csi_ignore}},
            std::pair {table_range {0x3c, 0x3f}, table_predicate {action::none, state::csi_ignore}},
            std::pair {table_range {0x20, 0x2f}, table_predicate {action::collect, state::csi_intermediate}},
            std::pair {table_range {0x40, 0x7e}, table_predicate {action::csi_dispatch, state::ground}},
    };
};

/**
 * State: CSI_INTERMEDIATE
 */
template<>
struct table_row_plan<state::csi_intermediate>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :execute,
     * 0x19       => :execute,
     * 0x1c..0x1f => :execute,
     * 0x20..0x2f => :collect,
     * 0x7f       => :ignore,
     * 0x30..0x3f => transition_to(:CSI_IGNORE),
     * 0x40..0x7e => [:csi_dispatch, transition_to(:GROUND)],
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x20, 0x2f}, table_predicate {action::collect, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x30, 0x3f}, table_predicate {action::none, state::csi_ignore}},
            std::pair {table_range {0x40, 0x7e}, table_predicate {action::csi_dispatch, state::ground}},
    };
};

/**
 * State: CSI_IGNORE
 */
template<>
struct table_row_plan<state::csi_ignore>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :execute,
     * 0x19       => :execute,
     * 0x1c..0x1f => :execute,
     * 0x20..0x3f => :ignore,
     * 0x7f       => :ignore,
     * 0x40..0x7e => transition_to(:GROUND),
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::execute, state::none}},
            std::pair {table_range {0x20, 0x3f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x40, 0x7e}, table_predicate {action::none, state::ground}},
    };
};

/**
 * State: DCS_ENTRY
 */
template<>
struct table_row_plan<state::dcs_entry>
{
    /*
     * Plan:
     *
     * :on_entry  => :clear,
     * 0x00..0x17 => :ignore,
     * 0x19       => :ignore,
     * 0x1c..0x1f => :ignore,
     * 0x7f       => :ignore,
     * 0x3a       => transition_to(:DCS_IGNORE),
     * 0x20..0x2f => [:collect, transition_to(:DCS_INTERMEDIATE)],
     * 0x30..0x39 => [:param, transition_to(:DCS_PARAM)],
     * 0x3b       => [:param, transition_to(:DCS_PARAM)],
     * 0x3c..0x3f => [:collect, transition_to(:DCS_PARAM)],
     * 0x40..0x7e => [transition_to(:DCS_PASSTHROUGH)]
     */

    static constexpr auto on_enter = table_predicate {action::clear, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x3a, 0x3a}, table_predicate {action::none, state::dcs_ignore}},
            std::pair {table_range {0x20, 0x2f}, table_predicate {action::collect, state::dcs_intermediate}},
            std::pair {table_range {0x30, 0x39}, table_predicate {action::param, state::dcs_param}},
            std::pair {table_range {0x3b, 0x3b}, table_predicate {action::param, state::dcs_param}},
            std::pair {table_range {0x3c, 0x3f}, table_predicate {action::collect, state::dcs_param}},
            std::pair {table_range {0x40, 0x7e}, table_predicate {action::none, state::dcs_passthrough}},
    };
};

/**
 * State: DCS_PARAM
 */
template<>
struct table_row_plan<state::dcs_param>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :ignore,
     * 0x19       => :ignore,
     * 0x1c..0x1f => :ignore,
     * 0x30..0x39 => :param,
     * 0x3b       => :param,
     * 0x7f       => :ignore,
     * 0x3a       => transition_to(:DCS_IGNORE),
     * 0x3c..0x3f => transition_to(:DCS_IGNORE),
     * 0x20..0x2f => [:collect, transition_to(:DCS_INTERMEDIATE)],
     * 0x40..0x7e => transition_to(:DCS_PASSTHROUGH)
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x30, 0x39}, table_predicate {action::param, state::none}},
            std::pair {table_range {0x3b, 0x3b}, table_predicate {action::param, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x3a, 0x3a}, table_predicate {action::none, state::dcs_ignore}},
            std::pair {table_range {0x3c, 0x3f}, table_predicate {action::none, state::dcs_ignore}},
            std::pair {table_range {0x20, 0x2f}, table_predicate {action::collect, state::dcs_intermediate}},
            std::pair {table_range {0x40, 0x7e}, table_predicate {action::none, state::dcs_passthrough}},
    };
};

/**
 * State: DCS_INTERMEDIATE
 */
template<>
struct table_row_plan<state::dcs_intermediate>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :ignore,
     * 0x19       => :ignore,
     * 0x1c..0x1f => :ignore,
     * 0x20..0x2f => :collect,
     * 0x7f       => :ignore,
     * 0x30..0x3f => transition_to(:DCS_IGNORE),
     * 0x40..0x7e => transition_to(:DCS_PASSTHROUGH)
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x20, 0x2f}, table_predicate {action::collect, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x30, 0x3f}, table_predicate {action::none, state::dcs_ignore}},
            std::pair {table_range {0x40, 0x7e}, table_predicate {action::none, state::dcs_passthrough}},
    };
};

/**
 * State: DCS_PASSTHROUGH
 */
template<>
struct table_row_plan<state::dcs_passthrough>
{
    /*
     * Plan:
     *
     * :on_entry  => :hook,
     * 0x00..0x17 => :put,
     * 0x19       => :put,
     * 0x1c..0x1f => :put,
     * 0x20..0x7e => :put,
     * 0x7f       => :ignore,
     * :on_exit   => :unhook
     */

    static constexpr auto on_enter = table_predicate {action::hook, state::none};
    static constexpr auto on_leave = table_predicate {action::unhook, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::put, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::put, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::put, state::none}},
            std::pair {table_range {0x20, 0x7e}, table_predicate {action::put, state::none}},
            std::pair {table_range {0x7f, 0x7f}, table_predicate {action::ignore, state::none}},
    };
};

/**
 * State: DCS_IGNORE
 */
template<>
struct table_row_plan<state::dcs_ignore>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :ignore,
     * 0x19       => :ignore,
     * 0x1c..0x1f => :ignore,
     * 0x20..0x7f => :ignore,
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x20, 0x7f}, table_predicate {action::ignore, state::none}},
    };
};

/**
 * State: OSC_STRING
 */
template<>
struct table_row_plan<state::osc_string>
{
    /*
     * Plan:
     *
     * :on_entry  => :osc_start,
     * 0x00..0x17 => :ignore,
     * 0x19       => :ignore,
     * 0x1c..0x1f => :ignore,
     * 0x20..0x7f => :osc_put,
     * :on_exit   => :osc_end
     */

    static constexpr auto on_enter = table_predicate {action::osc_start, state::none};
    static constexpr auto on_leave = table_predicate {action::osc_end, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x20, 0x7f}, table_predicate {action::osc_put, state::none}},
    };
};

/**
 * State: SOS_PM_APC_STRING
 */
template<>
struct table_row_plan<state::sos_pm_apc_string>
{
    /*
     * Plan:
     *
     * 0x00..0x17 => :ignore,
     * 0x19       => :ignore,
     * 0x1c..0x1f => :ignore,
     * 0x20..0x7f => :ignore,
     */

    static constexpr auto on_enter = table_predicate {action::none, state::none};
    static constexpr auto on_leave = table_predicate {action::none, state::none};
    static constexpr auto on_chars = std::array {
            std::pair {table_range {0x00, 0x17}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x19, 0x19}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x1c, 0x1f}, table_predicate {action::ignore, state::none}},
            std::pair {table_range {0x20, 0x7f}, table_predicate {action::ignore, state::none}},
    };
};

/**
 * Build a table row for a state according to plan.
 *
 * @tparam StateIndex The state index
 * @return The built table row
 */
template<int StateIndex>
static constexpr auto table_build_row()
{
    // We need to fit 128 ASCII characters and two event codes in the row
    constexpr int width = 128 + 2;

    // The new row
    std::array<table_predicate, width> row {};

    // Map enter and leave events for plan of interest
    row[128] = table_row_plan_on_enter<StateIndex>;
    row[129] = table_row_plan_on_leave<StateIndex>;

    // Now map ASCII characters
    for (int c = 0; c < 128; ++c)
    {
        // Generic algorithm for mapping characters
        auto map = [&row, c](auto&& plan) -> bool
        {
            for (auto&&[range, predicate] : plan)
            {
                if (range.contains(c))
                {
                    row[c] = predicate;
                    return true;
                }
            }

            return false;
        };

        // Try to map character for plan of interest
        if (!map(table_row_plan_on_chars<StateIndex>))
        {
            // Fall back to "any" plan if not mapped
            map(table_row_plan_on_chars<state::none>);
        }
    }

    return row;
}

/**
 * The state transition table.
 */
static constexpr auto table = std::array {
        table_build_row<state::ground>(),
        table_build_row<state::escape>(),
        table_build_row<state::escape_intermediate>(),
        table_build_row<state::csi_entry>(),
        table_build_row<state::csi_param>(),
        table_build_row<state::csi_intermediate>(),
        table_build_row<state::csi_ignore>(),
        table_build_row<state::dcs_entry>(),
        table_build_row<state::dcs_param>(),
        table_build_row<state::dcs_intermediate>(),
        table_build_row<state::dcs_passthrough>(),
        table_build_row<state::dcs_ignore>(),
        table_build_row<state::osc_string>(),
        table_build_row<state::sos_pm_apc_string>(),
};

} // namespace vtdec

#endif // #ifndef VTDEC_TABLE_H
