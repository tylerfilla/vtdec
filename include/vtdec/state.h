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

#ifndef VTDEC_STATE_H
#define VTDEC_STATE_H

namespace vtdec
{

/**
 * State indices.
 */
namespace state
{

enum
{
    none = -1,
    ground,
    escape,
    escape_intermediate,
    csi_entry,
    csi_param,
    csi_intermediate,
    csi_ignore,
    dcs_entry,
    dcs_param,
    dcs_intermediate,
    dcs_passthrough,
    dcs_ignore,
    osc_string,
    sos_pm_apc_string,
};

} // namespace state

/**
 * Info about a state.
 *
 * @tparam StateIndex The state index
 */
template<int StateIndex>
struct state_traits
{
};

/**
 * The name of a state.
 *
 * @tparam StateIndex The state index
 */
template<int StateIndex>
inline constexpr auto state_name = state_traits<StateIndex>::name;

/**
 * State: GROUND
 */
template<>
struct state_traits<state::ground>
{
    static constexpr auto name = "ground";
};

/**
 * State: ESCAPE
 */
template<>
struct state_traits<state::escape>
{
    static constexpr auto name = "escape";
};

/**
 * State: ESCAPE_INTERMEDIATE
 */
template<>
struct state_traits<state::escape_intermediate>
{
    static constexpr auto name = "escape_intermediate";
};

/**
 * State: CSI_ENTRY
 */
template<>
struct state_traits<state::csi_entry>
{
    static constexpr auto name = "csi_entry";
};

/**
 * State: CSI_PARAM
 */
template<>
struct state_traits<state::csi_param>
{
    static constexpr auto name = "csi_param";
};

/**
 * State: CSI_INTERMEDIATE
 */
template<>
struct state_traits<state::csi_intermediate>
{
    static constexpr auto name = "csi_intermediate";
};

/**
 * State: CSI_IGNORE
 */
template<>
struct state_traits<state::csi_ignore>
{
    static constexpr auto name = "csi_ignore";
};

/**
 * State: DCS_ENTRY
 */
template<>
struct state_traits<state::dcs_entry>
{
    static constexpr auto name = "dcs_entry";
};

/**
 * State: DCS_PARAM
 */
template<>
struct state_traits<state::dcs_param>
{
    static constexpr auto name = "dcs_param";
};

/**
 * State: DCS_INTERMEDIATE
 */
template<>
struct state_traits<state::dcs_intermediate>
{
    static constexpr auto name = "dcs_intermediate";
};

/**
 * State: DCS_PASSTHROUGH
 */
template<>
struct state_traits<state::dcs_passthrough>
{
    static constexpr auto name = "dcs_passthrough";
};

/**
 * State: DCS_IGNORE
 */
template<>
struct state_traits<state::dcs_ignore>
{
    static constexpr auto name = "dcs_ignore";
};

/**
 * State: OSC_STRING
 */
template<>
struct state_traits<state::osc_string>
{
    static constexpr auto name = "osc_string";
};

/**
 * State: SOS_PM_APC_STRING
 */
template<>
struct state_traits<state::sos_pm_apc_string>
{
    static constexpr auto name = "sos_pm_apc_string";
};

/**
 * Look up the name of a state programmatically.
 *
 * @param index The state index
 * @return The state name
 */
constexpr auto get_state_name(int index)
{
    using namespace state;

    switch (index)
    {
    default:
    case ground:
        return state_name<ground>;
    case escape:
        return state_name<escape>;
    case escape_intermediate:
        return state_name<escape_intermediate>;
    case csi_entry:
        return state_name<csi_entry>;
    case csi_param:
        return state_name<csi_param>;
    case csi_intermediate:
        return state_name<csi_intermediate>;
    case csi_ignore:
        return state_name<csi_ignore>;
    case dcs_entry:
        return state_name<dcs_entry>;
    case dcs_param:
        return state_name<dcs_param>;
    case dcs_intermediate:
        return state_name<dcs_intermediate>;
    case dcs_passthrough:
        return state_name<dcs_passthrough>;
    case dcs_ignore:
        return state_name<dcs_ignore>;
    case osc_string:
        return state_name<osc_string>;
    case sos_pm_apc_string:
        return state_name<sos_pm_apc_string>;
    }
}

} // namespace vtdec

#endif // #ifndef VTDEC_STATE_H
