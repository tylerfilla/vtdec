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

namespace vtdec
{

enum class State
{
    NONE = 0,
    GROUND,
    ESCAPE,
    ESCAPE_INTERMEDIATE,
    CSI_ENTRY,
    CSI_IGNORE,
    CSI_INTERMEDIATE,
    CSI_PARAM,
    DCS_ENTRY,
    DCS_IGNORE,
    DCS_INTERMEDIATE,
    DCS_PARAM,
    DCS_PASSTHROUGH,
    SOS_PM_APC_STRING,
    OSC_STRING,
};

enum class Action
{
    NONE = 0,
    EXECUTE,
    CLEAR,
    PRINT,
    IGNORE,
    COLLECT,
    ESC_DISPATCH,
    OSC_START,
    OSC_PUT,
    OSC_END,
    HOOK,
    PUT,
    UNHOOK,
    PARAM,
    CSI_DISPATCH,
};

namespace state
{
namespace detail
{

/**
 * Build action row for a state.
 *
 * @tparam The type of the target action map
 * @tparam The type of the "any" action map
 * @param target The target state action map
 * @return The event row
 */
template<class TargetMapT, class AnyMapT>
constexpr auto build_action_row(TargetMapT&& target, AnyMapT&& any)
{
    std::array<std::pair<Action, State>, 130> row {};

    // Iterate over action events
    // These include ASCII characters (0 to 0x7f), ENTR (0x80), and LEAV (0x81)
    for (unsigned char c = 0; c <= 0x7f + 1 + 1; ++c)
    {
        // Generic algorithm for testing any action range
        auto test = [&row, c](auto&& e) -> bool
        {
            if (c >= e.first.first && c <= e.first.second)
            {
                row[c].first = e.second.first;
                row[c].second = e.second.second;
                return true;
            }

            return false;
        };

        // Test all target actions
        for (auto&& e : target)
        {
            if (test(e))
                break;
        }

        // Test all "any" actions
        for (auto&& e : any)
        {
            if (test(e))
                break;
        }
    }

    return row;
}

} // namespace detail

/**
 * An event code for entering a state.
 */
constexpr auto ENTR = 0x80;

/**
 * An event code for leaving a state.
 */
constexpr auto LEAV = 0x81;

/**
 * Information about any state.
 */
namespace any
{

/**
 * Name for any state.
 */
constexpr auto name = "";

/**
 * Action map for any state.
 */
constexpr auto actions = std::array {
        /*
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

        std::pair {std::pair {0x18, 0x18}, std::pair {Action::EXECUTE, State::GROUND}},
        std::pair {std::pair {0x1a, 0x1a}, std::pair {Action::EXECUTE, State::GROUND}},
        std::pair {std::pair {0x80, 0x8f}, std::pair {Action::EXECUTE, State::GROUND}},
        std::pair {std::pair {0x91, 0x97}, std::pair {Action::EXECUTE, State::GROUND}},
        std::pair {std::pair {0x99, 0x99}, std::pair {Action::EXECUTE, State::GROUND}},
        std::pair {std::pair {0x9a, 0x9a}, std::pair {Action::EXECUTE, State::GROUND}},
        std::pair {std::pair {0x9c, 0x9c}, std::pair {Action::NONE, State::GROUND}},
        std::pair {std::pair {0x1b, 0x1b}, std::pair {Action::NONE, State::ESCAPE}},
        std::pair {std::pair {0x98, 0x98}, std::pair {Action::NONE, State::SOS_PM_APC_STRING}},
        std::pair {std::pair {0x9e, 0x9e}, std::pair {Action::NONE, State::SOS_PM_APC_STRING}},
        std::pair {std::pair {0x9f, 0x9f}, std::pair {Action::NONE, State::SOS_PM_APC_STRING}},
        std::pair {std::pair {0x90, 0x90}, std::pair {Action::NONE, State::DCS_ENTRY}},
        std::pair {std::pair {0x9d, 0x9d}, std::pair {Action::NONE, State::OSC_STRING}},
        std::pair {std::pair {0x9b, 0x9b}, std::pair {Action::NONE, State::CSI_ENTRY}},
};

} // namespace any

/**
 * Information about GROUND state.
 */
namespace ground
{

/**
 * Name for GROUND state.
 */
constexpr auto name = "ground";

/**
 * Action map for GROUND state.
 */
constexpr auto actions = std::array {
        /*
         * 0x00..0x17 => :execute,
         * 0x19       => :execute,
         * 0x1c..0x1f => :execute,
         * 0x20..0x7f => :print,
         */

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x20, 0x7f}, std::pair {Action::PRINT, State::NONE}},
};

namespace detail
{

/**
 * Build the action row for GROUND state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(ground::actions, any::actions);
}

} // namespace detail
} // namespace ground

/**
 * Information about ESCAPE state.
 */
namespace escape
{

/**
 * Name for ESCAPE state.
 */
constexpr auto name = "escape";

/**
 * Action map for ESCAPE state.
 */
constexpr auto actions = std::array {
        /*
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

        std::pair {std::pair {ENTR, ENTR}, std::pair {Action::CLEAR, State::NONE}},
        std::pair {std::pair {0x00, 0x17}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x20, 0x2f}, std::pair {Action::COLLECT, State::ESCAPE_INTERMEDIATE}},
        std::pair {std::pair {0x30, 0x4f}, std::pair {Action::ESC_DISPATCH, State::GROUND}},
        std::pair {std::pair {0x51, 0x57}, std::pair {Action::ESC_DISPATCH, State::GROUND}},
        std::pair {std::pair {0x59, 0x59}, std::pair {Action::ESC_DISPATCH, State::GROUND}},
        std::pair {std::pair {0x5a, 0x5a}, std::pair {Action::ESC_DISPATCH, State::GROUND}},
        std::pair {std::pair {0x5c, 0x5c}, std::pair {Action::ESC_DISPATCH, State::GROUND}},
        std::pair {std::pair {0x60, 0x7e}, std::pair {Action::ESC_DISPATCH, State::GROUND}},
        std::pair {std::pair {0x5b, 0x5b}, std::pair {Action::NONE, State::CSI_ENTRY}},
        std::pair {std::pair {0x5d, 0x5d}, std::pair {Action::NONE, State::OSC_STRING}},
        std::pair {std::pair {0x50, 0x50}, std::pair {Action::NONE, State::DCS_ENTRY}},
        std::pair {std::pair {0x58, 0x58}, std::pair {Action::NONE, State::SOS_PM_APC_STRING}},
        std::pair {std::pair {0x5e, 0x5e}, std::pair {Action::NONE, State::SOS_PM_APC_STRING}},
        std::pair {std::pair {0x5f, 0x5f}, std::pair {Action::NONE, State::SOS_PM_APC_STRING}},
};

namespace detail
{

/**
 * Build the action row for ESCAPE state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(escape::actions, any::actions);
}

} // namespace detail
} // namespace escape

/**
 * Information about ESCAPE_INTERMEDIATE state.
 */
namespace escape_intermediate
{

/**
 * Name for ESCAPE_INTERMEDIATE state.
 */
constexpr auto name = "escape_intermediate";

/**
 * Action map for ESCAPE_INTERMEDIATE state.
 */
constexpr auto actions = std::array {
        /*
         * 0x00..0x17 => :execute,
         * 0x19       => :execute,
         * 0x1c..0x1f => :execute,
         * 0x20..0x2f => :collect,
         * 0x7f       => :ignore,
         * 0x30..0x7e => [:esc_dispatch, transition_to(:GROUND)]
         */

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x20, 0x2f}, std::pair {Action::COLLECT, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x30, 0x7e}, std::pair {Action::ESC_DISPATCH, State::GROUND}},
};

namespace detail
{

/**
 * Build the action row for ESCAPE_INTERMEDIATE state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(escape_intermediate::actions, any::actions);
}

} // namespace detail
} // namespace escape_intermediate

/**
 * Information about CSI_ENTRY state.
 */
namespace csi_entry
{

/**
 * Name for CSI_ENTRY state.
 */
constexpr auto name = "csi_entry";

/**
 * Action map for CSI_ENTRY state.
 */
constexpr auto actions = std::array {
        /*
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

        std::pair {std::pair {ENTR, ENTR}, std::pair {Action::CLEAR, State::NONE}},
        std::pair {std::pair {0x00, 0x17}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x20, 0x2f}, std::pair {Action::COLLECT, State::CSI_INTERMEDIATE}},
        std::pair {std::pair {0x3a, 0x3a}, std::pair {Action::NONE, State::CSI_IGNORE}},
        std::pair {std::pair {0x30, 0x39}, std::pair {Action::PARAM, State::CSI_PARAM}},
        std::pair {std::pair {0x3b, 0x3b}, std::pair {Action::PARAM, State::CSI_PARAM}},
        std::pair {std::pair {0x3c, 0x3f}, std::pair {Action::COLLECT, State::CSI_PARAM}},
        std::pair {std::pair {0x40, 0x7e}, std::pair {Action::CSI_DISPATCH, State::GROUND}},
};

namespace detail
{

/**
 * Build the action row for CSI_ENTRY state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(csi_entry::actions, any::actions);
}

} // namespace detail
} // namespace csi_entry

/**
 * Information about CSI_IGNORE state.
 */
namespace csi_ignore
{

/**
 * Name for CSI_IGNORE state.
 */
constexpr auto name = "csi_ignore";

/**
 * Action map for CSI_IGNORE state.
 */
constexpr auto actions = std::array {
        /*
         * 0x00..0x17 => :execute,
         * 0x19       => :execute,
         * 0x1c..0x1f => :execute,
         * 0x20..0x3f => :ignore,
         * 0x7f       => :ignore,
         * 0x40..0x7e => transition_to(:GROUND),
        */

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x20, 0x3f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x40, 0x7e}, std::pair {Action::NONE, State::GROUND}},
};

namespace detail
{

/**
 * Build the action row for CSI_IGNORE state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(csi_ignore::actions, any::actions);
}

} // namespace detail
} // namespace csi_ignore

/**
 * Information about CSI_PARAM state.
 */
namespace csi_param
{

/**
 * Name for CSI_PARAM state.
 */
constexpr auto name = "csi_param";

/**
 * Action map for CSI_PARAM state.
 */
constexpr auto actions = std::array {
        /*
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

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x30, 0x39}, std::pair {Action::PARAM, State::NONE}},
        std::pair {std::pair {0x3b, 0x3b}, std::pair {Action::PARAM, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x3a, 0x3a}, std::pair {Action::NONE, State::CSI_IGNORE}},
        std::pair {std::pair {0x3c, 0x3f}, std::pair {Action::NONE, State::CSI_IGNORE}},
        std::pair {std::pair {0x20, 0x2f}, std::pair {Action::COLLECT, State::CSI_INTERMEDIATE}},
        std::pair {std::pair {0x40, 0x7e}, std::pair {Action::CSI_DISPATCH, State::GROUND}},
};

namespace detail
{

/**
 * Build the action row for CSI_PARAM state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(csi_param::actions, any::actions);
}

} // namespace detail
} // namespace csi_param

/**
 * Information about CSI_INTERMEDIATE state.
 */
namespace csi_intermediate
{

/**
 * Name for CSI_INTERMEDIATE state.
 */
constexpr auto name = "csi_intermediate";

/**
 * Action map for CSI_INTERMEDIATE state.
 */
constexpr auto actions = std::array {
        /*
         * 0x00..0x17 => :execute,
         * 0x19       => :execute,
         * 0x1c..0x1f => :execute,
         * 0x20..0x2f => :collect,
         * 0x7f       => :ignore,
         * 0x30..0x3f => transition_to(:CSI_IGNORE),
         * 0x40..0x7e => [:csi_dispatch, transition_to(:GROUND)],
         */

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::EXECUTE, State::NONE}},
        std::pair {std::pair {0x20, 0x2f}, std::pair {Action::COLLECT, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x30, 0x3f}, std::pair {Action::NONE, State::CSI_IGNORE}},
        std::pair {std::pair {0x40, 0x7e}, std::pair {Action::CSI_DISPATCH, State::GROUND}},
};

namespace detail
{

/**
 * Build the action row for CSI_INTERMEDIATE state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(csi_intermediate::actions, any::actions);
}

} // namespace detail
} // namespace csi_intermediate

/**
 * Information about DCS_ENTRY state.
 */
namespace dcs_entry
{

/**
 * Name for DCS_ENTRY state.
 */
constexpr auto name = "dcs_entry";

/**
 * Action map for DCS_ENTRY state.
 */
constexpr auto actions = std::array {
        /*
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

        std::pair {std::pair {ENTR, ENTR}, std::pair {Action::CLEAR, State::NONE}},
        std::pair {std::pair {0x00, 0x17}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x3a, 0x3a}, std::pair {Action::NONE, State::DCS_IGNORE}},
        std::pair {std::pair {0x20, 0x2f}, std::pair {Action::COLLECT, State::DCS_INTERMEDIATE}},
        std::pair {std::pair {0x30, 0x39}, std::pair {Action::PARAM, State::DCS_PARAM}},
        std::pair {std::pair {0x3b, 0x3b}, std::pair {Action::PARAM, State::DCS_PARAM}},
        std::pair {std::pair {0x3c, 0x3f}, std::pair {Action::COLLECT, State::DCS_PARAM}},
        std::pair {std::pair {0x40, 0x7e}, std::pair {Action::NONE, State::DCS_PASSTHROUGH}},
};

namespace detail
{

/**
 * Build the action row for DCS_ENTRY state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(dcs_entry::actions, any::actions);
}

} // namespace detail
} // namespace dcs_entry

/**
 * Information about DCS_INTERMEDIATE state.
 */
namespace dcs_intermediate
{

/**
 * Name for DCS_INTERMEDIATE state.
 */
constexpr auto name = "dcs_intermediate";

/**
 * Action map for DCS_INTERMEDIATE state.
 */
constexpr auto actions = std::array {
        /*
         * 0x00..0x17 => :ignore,
         * 0x19       => :ignore,
         * 0x1c..0x1f => :ignore,
         * 0x20..0x2f => :collect,
         * 0x7f       => :ignore,
         * 0x30..0x3f => transition_to(:DCS_IGNORE),
         * 0x40..0x7e => transition_to(:DCS_PASSTHROUGH)
         */

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x20, 0x2f}, std::pair {Action::COLLECT, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x30, 0x3f}, std::pair {Action::NONE, State::DCS_IGNORE}},
        std::pair {std::pair {0x40, 0x7e}, std::pair {Action::NONE, State::DCS_PASSTHROUGH}},
};

namespace detail
{

/**
 * Build the action row for DCS_INTERMEDIATE state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(dcs_intermediate::actions, any::actions);
}

} // namespace detail
} // namespace dcs_intermediate

/**
 * Information about DCS_IGNORE state.
 */
namespace dcs_ignore
{

/**
 * Name for DCS_IGNORE state.
 */
constexpr auto name = "dcs_ignore";

/**
 * Action map for DCS_IGNORE state.
 */
constexpr auto actions = std::array {
        /*
         * 0x00..0x17 => :ignore,
         * 0x19       => :ignore,
         * 0x1c..0x1f => :ignore,
         * 0x20..0x7f => :ignore,
         */

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x20, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
};

namespace detail
{

/**
 * Build the action row for DCS_IGNORE state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(dcs_ignore::actions, any::actions);
}

} // namespace detail
} // namespace dcs_ignore

/**
 * Information about DCS_PARAM state.
 */
namespace dcs_param
{

/**
 * Name for DCS_PARAM state.
 */
constexpr auto name = "dcs_param";

/**
 * Action map for DCS_PARAM state.
 */
constexpr auto actions = std::array {
        /*
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

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x30, 0x39}, std::pair {Action::PARAM, State::NONE}},
        std::pair {std::pair {0x3b, 0x3b}, std::pair {Action::PARAM, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x3a, 0x3a}, std::pair {Action::NONE, State::DCS_IGNORE}},
        std::pair {std::pair {0x3c, 0x3f}, std::pair {Action::NONE, State::DCS_IGNORE}},
        std::pair {std::pair {0x20, 0x2f}, std::pair {Action::COLLECT, State::DCS_INTERMEDIATE}},
        std::pair {std::pair {0x40, 0x7e}, std::pair {Action::NONE, State::DCS_PASSTHROUGH}},
};

namespace detail
{

/**
 * Build the action row for DCS_PARAM state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(dcs_param::actions, any::actions);
}

} // namespace detail
} // namespace dcs_param

/**
 * Information about DCS_PASSTHROUGH state.
 */
namespace dcs_passthrough
{

/**
 * Name for DCS_PASSTHROUGH state.
 */
constexpr auto name = "dcs_passthrough";

/**
 * Action map for DCS_PASSTHROUGH state.
 */
constexpr auto actions = std::array {
        /*
         * :on_entry  => :hook,
         * 0x00..0x17 => :put,
         * 0x19       => :put,
         * 0x1c..0x1f => :put,
         * 0x20..0x7e => :put,
         * 0x7f       => :ignore,
         * :on_exit   => :unhook
         */

        std::pair {std::pair {ENTR, ENTR}, std::pair {Action::HOOK, State::NONE}},
        std::pair {std::pair {0x00, 0x17}, std::pair {Action::PUT, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::PUT, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::PUT, State::NONE}},
        std::pair {std::pair {0x20, 0x7e}, std::pair {Action::PUT, State::NONE}},
        std::pair {std::pair {0x7f, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {LEAV, LEAV}, std::pair {Action::UNHOOK, State::NONE}},
};

namespace detail
{

/**
 * Build the action row for DCS_PASSTHROUGH state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(dcs_passthrough::actions, any::actions);
}

} // namespace detail
} // namespace dcs_passthrough

/**
 * Information about SOS_PM_APC_STRING state.
 */
namespace sos_pm_apc_string
{

/**
 * Name for SOS_PM_APC_STRING state.
 */
constexpr auto name = "sos_pm_apc_string";

/**
 * Action map for SOS_PM_APC_STRING state.
 */
constexpr auto actions = std::array {
        /*
         * 0x00..0x17 => :ignore,
         * 0x19       => :ignore,
         * 0x1c..0x1f => :ignore,
         * 0x20..0x7f => :ignore,
         */

        std::pair {std::pair {0x00, 0x17}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x20, 0x7f}, std::pair {Action::IGNORE, State::NONE}},
};

namespace detail
{

/**
 * Build the action row for SOS_PM_APC_STRING state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(sos_pm_apc_string::actions, any::actions);
}

} // namespace detail
} // namespace sos_pm_apc_string

/**
 * Information about OSC_STRING state.
 */
namespace osc_string
{

/**
 * Name for OSC_STRING state.
 */
constexpr auto name = "osc_string";

/**
 * Action map for OSC_STRING state.
 */
constexpr auto actions = std::array {
        /*
         * :on_entry  => :osc_start,
         * 0x00..0x17 => :ignore,
         * 0x19       => :ignore,
         * 0x1c..0x1f => :ignore,
         * 0x20..0x7f => :osc_put,
         * :on_exit   => :osc_end
         */

        std::pair {std::pair {ENTR, ENTR}, std::pair {Action::OSC_START, State::NONE}},
        std::pair {std::pair {0x00, 0x17}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x19, 0x19}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x1c, 0x1f}, std::pair {Action::IGNORE, State::NONE}},
        std::pair {std::pair {0x20, 0x7f}, std::pair {Action::OSC_PUT, State::NONE}},
        std::pair {std::pair {LEAV, LEAV}, std::pair {Action::OSC_END, State::NONE}},
};

namespace detail
{

/**
 * Build the action row for OSC_STRING state.
 *
 * @return The action row
 */
constexpr auto build_action_row()
{
    return state::detail::build_action_row(osc_string::actions, any::actions);
}

} // namespace detail
} // namespace osc_string

namespace detail
{

/**
 * Build the action table.
 *
 * @return The action table
 */
constexpr auto build_action_table()
{
    return std::array {
            state::ground::detail::build_action_row(),
            state::escape::detail::build_action_row(),
            state::escape_intermediate::detail::build_action_row(),
            state::csi_entry::detail::build_action_row(),
            state::csi_ignore::detail::build_action_row(),
            state::csi_intermediate::detail::build_action_row(),
            state::csi_param::detail::build_action_row(),
            state::dcs_entry::detail::build_action_row(),
            state::dcs_ignore::detail::build_action_row(),
            state::dcs_intermediate::detail::build_action_row(),
            state::dcs_param::detail::build_action_row(),
            state::dcs_passthrough::detail::build_action_row(),
            state::sos_pm_apc_string::detail::build_action_row(),
            state::osc_string::detail::build_action_row(),
    };
}

} // namespace detail
} // namespace state

/**
 * The action table. This table is primarily indexed by the current state and
 * secondarily indexed by the received input code (0 to 0x7f, ENTR, LEAV).
 */
constexpr auto action_table = state::detail::build_action_table();

} // namespace vtdec

#endif // #ifndef VTDEC_TABLE_H
