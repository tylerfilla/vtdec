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

#ifndef VTDEC_ACTION_H
#define VTDEC_ACTION_H

namespace vtdec
{

/**
 * Action indices.
 */
namespace action
{

enum
{
    none = -1,
    ignore,
    print,
    execute,
    clear,
    collect,
    param,
    esc_dispatch,
    csi_dispatch,
    hook,
    put,
    unhook,
    osc_start,
    osc_put,
    osc_end,
};

} // namespace action

/**
 * Info about an action.
 *
 * @tparam ActionIndex The action index
 */
template<int ActionIndex>
struct action_traits
{
};

/**
 * The name of an action.
 *
 * @tparam ActionIndex The action index
 */
template<int ActionIndex>
static constexpr auto action_name = action_traits<ActionIndex>::name;

/**
 * Action: IGNORE
 */
template<>
struct action_traits<action::ignore>
{
    static constexpr auto name = "ignore";
};

/**
 * Action: PRINT
 */
template<>
struct action_traits<action::print>
{
    static constexpr auto name = "print";
};

/**
 * Action: EXECUTE
 */
template<>
struct action_traits<action::execute>
{
    static constexpr auto name = "execute";
};

/**
 * Action: CLEAR
 */
template<>
struct action_traits<action::clear>
{
    static constexpr auto name = "clear";
};

/**
 * Action: COLLECT
 */
template<>
struct action_traits<action::collect>
{
    static constexpr auto name = "collect";
};

/**
 * Action: PARAM
 */
template<>
struct action_traits<action::param>
{
    static constexpr auto name = "param";
};

/**
 * Action: ESC_DISPATCH
 */
template<>
struct action_traits<action::esc_dispatch>
{
    static constexpr auto name = "esc_dispatch";
};

/**
 * Action: CSI_DISPATCH
 */
template<>
struct action_traits<action::csi_dispatch>
{
    static constexpr auto name = "csi_dispatch";
};

/**
 * Action: HOOK
 */
template<>
struct action_traits<action::hook>
{
    static constexpr auto name = "hook";
};

/**
 * Action: PUT
 */
template<>
struct action_traits<action::put>
{
    static constexpr auto name = "put";
};

/**
 * Action: UNHOOK
 */
template<>
struct action_traits<action::unhook>
{
    static constexpr auto name = "unhook";
};

/**
 * Action: OSC_START
 */
template<>
struct action_traits<action::osc_start>
{
    static constexpr auto name = "osc_start";
};

/**
 * Action: OSC_PUT
 */
template<>
struct action_traits<action::osc_put>
{
    static constexpr auto name = "osc_put";
};

/**
 * Action: OSC_END
 */
template<>
struct action_traits<action::osc_end>
{
    static constexpr auto name = "osc_end";
};

/**
 * Look up the name of an action programmatically.
 *
 * @param index The action index
 * @return The action name
 */
constexpr auto get_action_name(int index)
{
    using namespace action;

    switch (index)
    {
    default:
    case ignore:
        return action_name<ignore>;
    case print:
        return action_name<print>;
    case execute:
        return action_name<execute>;
    case clear:
        return action_name<clear>;
    case collect:
        return action_name<collect>;
    case param:
        return action_name<param>;
    case esc_dispatch:
        return action_name<esc_dispatch>;
    case csi_dispatch:
        return action_name<csi_dispatch>;
    case hook:
        return action_name<hook>;
    case put:
        return action_name<put>;
    case unhook:
        return action_name<unhook>;
    case osc_start:
        return action_name<osc_start>;
    case osc_put:
        return action_name<osc_put>;
    case osc_end:
        return action_name<osc_end>;
    }
}

} // namespace vtdec

#endif // #ifndef VTDEC_ACTION_H
