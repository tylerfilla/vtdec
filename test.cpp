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

#include <iostream>
#include <vtdec/decode.h>

static constexpr auto TEST = 1 + R"(
]2;nano]1;nano[?1049h[22;0;0t[1;24r(B[m[4l[?7h[39;49m[?1h=[?1h=[?1h=[?25l[39;49m(B[m[H[2J(B[0;1m[37m[44m  GNU nano 3.2[23X[1;38HNew Buffer[K[79G[39;49m(B[m[22;16H(B[0;1m[37m[42m[ Welcome to nano.  For basic help, type Ctrl+G. ][39;49m(B[m
[23d[36m^G[39;49m(B[m [32mGet Help[39;49m(B[m  [36m^O[39;49m(B[m [32mWrite Out[39;49m(B[m [36m^W[39;49m(B[m [32mWhere Is[39;49m(B[m  [36m^K[39;49m(B[m [32mCut Text[39;49m(B[m  [36m^J[39;49m(B[m [32mJustify[39;49m(B[m   [36m^C[39;49m(B[m [32mCur Pos
[24d[36m^X[39;49m(B[m [32mExit[14G[36m^R[39;49m(B[m [32mRead File[39;49m(B[m [36m^\[39;49m(B[m [32mReplace[39;49m(B[m   [36m^U[39;49m(B[m [32mUncut Text[36m^T[39;49m(B[m [32mTo Spell[39;49m(B[m  [36m^_[39;49m(B[m [32mGo To Line
[22d[39;49m(B[m[2d[39;49m[36m 1[C[39;49m(B[m[?12l[?25h[?25l[1;71H(B[0;1m[37m[44mModified[39;49m(B[m[22;15H(B[0;1m[37m[42m[ line 1/1 (100%), col 2/2 (100%), char 1/1 (100%) ][39;49m(B[m[2;4Ha[?12l[?25h[?25l[22;38H(B[0;1m[37m[42m3/3[22;55H2/2[22;67H[39;49m(B[m[2;5Hs[?12l[?25h[?25l[22;38H(B[0;1m[37m[42m4/4[22;55H3/3[22;67H[39;49m(B[m[2;6Hd[?12l[?25h[?25l[22;38H(B[0;1m[37m[42m5/5[22;55H4/4[22;67H[39;49m(B[m[2;7Hf[?12l[?25h[?25l[22;22H(B[0;1m[37m[42m2/2[22;38H1/1[22;55H5/5[22;67H[39;49m(B[m
[3d[36m 2[C[39;49m(B[m[?12l[?25h[?25l[22;38H(B[0;1m[37m[42m2/2[22;55H6/6[22;67H[39;49m(B[m[3;4Ha[?12l[?25h[?25l[22;38H(B[0;1m[37m[42m3/3[22;55H7/7[22;67H[39;49m(B[m[3;5Hs[?12l[?25h[?25l[22;38H(B[0;1m[37m[42m4/4[22;55H8/8[22;67H[39;49m(B[m[3;6Hd[?12l[?25h[?25l[22;38H(B[0;1m[37m[42m5/5[22;55H9/9[22;67H[39;49m(B[m[3;7Hf[?12l[?25h[?25l[22;14H(B[0;1m[37m[42m[ line 3/3 (100%), col 1/1 (100%), char 10/10 (100%) ][39;49m(B[m
[4d[36m 3[C[39;49m(B[m[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m2/2[22;55H1/11[22;68H[39;49m(B[m[4;4Ha[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m3/3[22;55H2/12[22;68H[39;49m(B[m[4;5Hs[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m4/4[22;55H3/13[22;68H[39;49m(B[m[4;6Hd[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m5/5[22;55H4/14[22;68H[39;49m(B[m[4;7Hf[?12l[?25h[?25l[22;21H(B[0;1m[37m[42m4/4[22;37H1/1[22;55H5/15[22;68H[39;49m(B[m
[5d[36m 4[C[39;49m(B[m[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m2/2[22;55H6/16[22;68H[39;49m(B[m[5;4Hq[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m3/3[22;55H7/17[22;68H[39;49m(B[m[5;5Hw[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m4/4[22;55H8/18[22;68H[39;49m(B[m[5;6He[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m5/5[22;55H9/19[22;68H[39;49m(B[m[5;7Hr[?12l[?25h[?25l[22;21H(B[0;1m[37m[42m5/5[22;37H1/1[22;54H20/20[22;68H[39;49m(B[m
[6d[36m 5[C[39;49m(B[m[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m2/2[22;55H1/21[22;68H[39;49m(B[m[6;4Hq[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m3/3[22;55H2/22[22;68H[39;49m(B[m[6;5Hw[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m4/4[22;55H3/23[22;68H[39;49m(B[m[6;6He[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m5/5[22;55H4/24[22;68H[39;49m(B[m[6;7Hr[?12l[?25h[?25l[22;21H(B[0;1m[37m[42m6/6[22;37H1/1[22;55H5/25[22;68H[39;49m(B[m
[7d[36m 6[C[39;49m(B[m[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m2/2[22;55H6/26[22;68H[39;49m(B[m[7;4Hq[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m3/3[22;55H7/27[22;68H[39;49m(B[m[7;5Hw[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m4/4[22;55H8/28[22;68H[39;49m(B[m[7;6He[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m5/5[22;55H9/29[22;68H[39;49m(B[m[7;7Hr[?12l[?25h[?25l[22;21H(B[0;1m[37m[42m7/7[22;37H1/1[22;54H30/30[22;68H[39;49m(B[m
[8d[36m 7[C[39;49m(B[m[?12l[?25h[?25l[22;21H(B[0;1m[37m[42m6/6[22;37H5/5[22;54H29/29[22;68H[39;49m(B[m
[8d[K[7;8H[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m4/4[22;55H8/28[22;68H[39;49m(B[m[7;7H [?12l[?25h[?25l[22;37H(B[0;1m[37m[42m3/3[22;55H7/27[22;68H[39;49m(B[m[7;6H [?12l[?25h[?25l[22;37H(B[0;1m[37m[42m2/2[22;55H6/26[22;68H[39;49m(B[m[7;5H [?12l[?25h[?25l[22;37H(B[0;1m[37m[42m1/1[22;55H5/25[22;68H[39;49m(B[m[7;4H [?12l[?25h[?25l[22;21H(B[0;1m[37m[42m5/5[22;37H5/5[22;55H4/24[22;68H[39;49m(B[m
[7d[K[6;8H[?12l[?25h[?25l[22;14H (B[0;1m[37m[42m[ line 4/5 (8[22;54H19/24 (79%) ][39;49m(B[m[K[5;8H[?12l[?25h[?25l[22;22H(B[0;1m[37m[42m3/5 (6[22;55H4/24 (58%) ][39;49m(B[m[4;8H[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m4/5 (80%), char 13/24 (54%) ][39;49m(B[m[K[4;7H[?12l[?25h[?25l[22;37H(B[0;1m[37m[42m3/5 (6[22;54H2[61G0%) ][39;49m(B[m[4;6H[?12l[?25h[?25l[22;39H(B[0;1m[37m[42m4 (75[22;57H3 (52%) ][39;49m(B[m[4;6Hf[K[?12l[?25h[?25l[22;22H(B[0;1m[37m[42m4/5 (8[22;39H5 (60[22;54H6/23 (69%) ][39;49m(B[m[5;6H[?12l[?25h[?25l[22;22H(B[0;1m[37m[42m5/6 (83[22;37H1/3 (33[22;54H7/24 (70%) ][39;49m(B[m[5;6H[K[6der[K
[7d[36m 6[39;49m(B[m qwer[A[?12l[?25h[?25l[22;22H(B[0;1m[37m[42m6/7 (85[22;54H8/25 (72%) ][39;49m(B[m[6;4H[K[7der[K
[8d[36m 7[39;49m(B[m qwer[A[?12l[?25h[?25l[22;22H(B[0;1m[37m[42m5/7 (71[22;39H1 (100%), char 17/25 (68%) ][39;49m(B[m[6;4H[?12l[?25h[?25l
[22d(B[0;1m[37m[44mSave modified buffer?  (Answering "No" will DISCARD changes.)[K
[23d(B[0m[36m Y[39;49m(B[m [32mYes[39;49m(B[m[K
[24d[36m N[39;49m(B[m [32mNo[39;49m(B[m  [14G   [36m^C[39;49m(B[m [32mCancel[39;49m(B[m[K[22;63H[?12l[?25h[?25l
[J[24;80H[?12l[?25h[24;1H[?1049l[23;0;0t
[?1l>[1m[7m%[27m[1m[0m
)";

struct my_processor : vtdec::processor
{
    void print(char32_t c) final
    {
        std::cout << "PRINT " << static_cast<char>(c) << "\n";
    }

    void ctl(char c) final
    {
        std::cout << "CTL SINGLE " << c << "\n";
    }

    void ctl_begin() final
    {
        std::cout << "CTL BEGIN\n";
    }

    void ctl_put(char32_t c) final
    {
        std::cout << "CTL PUT " << static_cast<char>(c) << "\n";
    }

    void ctl_end(bool cancel) final
    {
        std::cout << "CTL END\n";
    }

    void dcs_begin() final
    {
        std::cout << "DCS BEGIN\n";
    }

    void dcs_put(char32_t c) final
    {
        std::cout << "DCS PUT " << static_cast<char>(c) << "\n";
    }

    void dcs_end(bool cancel) final
    {
        std::cout << "DCS END\n";
    }

    void osc_begin() final
    {
        std::cout << "OSC BEGIN\n";
    }

    void osc_put(char32_t c) final
    {
        std::cout << "OSC PUT " << static_cast<char>(c) << "\n";
    }

    void osc_end(bool cancel) final
    {
        std::cout << "OSC END\n";
    }

    void decode_action(int act) final
    {
        std::cout << "do " << vtdec::get_action_name(act) << "\n";
    }

    void decode_transition(int src, int dst) final
    {
        std::cout << vtdec::get_state_name(src) << " -> " << vtdec::get_state_name(dst) << "\n";
    }
};

int main(int argc, char* argv[])
{
    try
    {
        if (vtdec::decode<my_processor>(TEST).state)
            throw "incomplete parse";
    }
    catch (const char* s)
    {
        std::cerr << s << "\n";
        return 1;
    }

    return 0;
}
