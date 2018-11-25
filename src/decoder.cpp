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

#include <vtdec/decoder.h>

/*
void vtdec::vtparse_init(vtparse_t* parser, vtparse_callback_t cb)
{
    parser->state                  = VTDEC_STATE_GROUND;
    parser->num_intermediate_chars = 0;
    parser->num_params             = 0;
    parser->ignore_flagged         = 0;
    parser->cb                     = cb;
    parser->characterBytes         = 1;
    parser->utf8Character          = 0;
}
*/

namespace
{

void do_action(vtparse_t* parser, vtparse_action_t action, unsigned int ch)
{
    switch(action)
    {
    case VTDEC_ACTION_PRINT:
    case VTDEC_ACTION_EXECUTE:
    case VTDEC_ACTION_HOOK:
    case VTDEC_ACTION_PUT:
    case VTDEC_ACTION_OSC_START:
    case VTDEC_ACTION_OSC_PUT:
    case VTDEC_ACTION_OSC_END:
    case VTDEC_ACTION_UNHOOK:
    case VTDEC_ACTION_CSI_DISPATCH:
    case VTDEC_ACTION_ESC_DISPATCH:
        parser->cb(parser, action, ch);
        break;
    case VTDEC_ACTION_IGNORE:
        break;
    case VTDEC_ACTION_COLLECT:
        if (parser->num_intermediate_chars + 1 > MAX_INTERMEDIATE_CHARS)
        {
            parser->ignore_flagged = 1;
        }
        else
        {
            parser->intermediate_chars[parser->num_intermediate_chars++] = (unsigned char) ch;
        }
        break;
    case VTDEC_ACTION_PARAM:
        if (ch == ';')
        {
            parser->num_params += 1;
            parser->params[parser->num_params-1] = 0;
        }
        else
        {
            int current_param;

            if (parser->num_params == 0)
            {
                parser->num_params = 1;
                parser->params[0]  = 0;
            }

            current_param = parser->num_params - 1;

            parser->params[current_param] *= 10;
            parser->params[current_param] += ch - '0';
        }
        break;
    case VTDEC_ACTION_CLEAR:
        parser->num_intermediate_chars = 0;
        parser->num_params            = 0;
        parser->ignore_flagged        = 0;
        break;
    default:
        parser->cb(parser, VTDEC_ACTION_ERROR, 0);
        break;
    }
}

void do_state_change(vtparse_t* parser, state_change_t change, unsigned int ch)
{
    /* A state change is an action and/or a new state to transition to. */

    vtparse_state_t new_state = STATE(change);
    vtparse_action_t action = ACTION(change);


    if (new_state)
    {
        /* Perform up to three actions:
         *   1. the exit action of the old state
         *   2. the action associated with the transition
         *   3. the entry action of the new state
         */

        vtparse_action_t exit_action = EXIT_ACTIONS[parser->state-1];
        vtparse_action_t entry_action = ENTRY_ACTIONS[new_state-1];

        if (exit_action)
            do_action(parser, exit_action, 0);

        if (action)
            do_action(parser, action, ch);

        if (entry_action)
            do_action(parser, entry_action, 0);

        parser->state = new_state;
    }
    else
    {
        do_action(parser, action, ch);
    }
}

} // namespace

void vtdec::vtparse(vtparse_t* parser, unsigned char* data, unsigned int len)
{
    int i;
    for(i = 0; i < len; i++)
    {
        unsigned char ch = data[i];
        if (parser->characterBytes != 1)
        {
            parser->utf8Character = (parser->utf8Character << 6) | (ch & 0x3F);
            parser->characterBytes--;

            if (parser->characterBytes == 1)
            {
                state_change_t change = VTDEC_ACTION_PRINT;
                do_state_change(parser, change, parser->utf8Character);
            }
        }
        else if ((ch&(1<<7)) != 0)
        {
            int bit = 6;
            do
            {
                if ((ch&(1<<bit)) == 0)
                {
                    break;
                }
                bit--;
            }while(bit > 1);

            parser->utf8Character = 0;
            parser->characterBytes = 7-bit;
            switch(parser->characterBytes)
            {
            case 2:
                    parser->utf8Character = ch & (1 | (1<<1) | (1<<2) | (1<<3) | (1<<4));
                    break;
            case 3:
                    parser->utf8Character = ch & (1 | (1<<1) | (1<<2) | (1<<3));
                    break;
            case 4:
                    parser->utf8Character = ch & (1 | (1<<1) | (1<<2));
                    break;
            case 5:
                    parser->utf8Character = ch & (1 | (1<<1));
                    break;
            case 6:
                    parser->utf8Character = ch & 1;
                    break;
            }
        }
        else
        {
            state_change_t change = STATE_TABLE[parser->state-1][ch];
            do_state_change(parser, change, (unsigned int)ch);
        }
    }
}
