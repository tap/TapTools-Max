/// @file
/// @brief      tap.elixir~ — gain-structure management / crossfading mixer.
/// @details    A mixer with 2–10 signal inlets (set by the first argument). Each inlet is gated by a
///             toggle; the active inlets share the gain equally (1/N), and changes are slewed over a
///             per-inlet (or global) interpolation time. The summed output is scaled by 0.95.
///             Faithful port of the original (self-contained — no Jamoma); the per-inlet-count perform
///             routines of the original are folded into one variable-width loop.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2001-2026 Timothy Place.

#include <algorithm>
#include <array>
#include <memory>
#include <vector>

#include "c74_min.h"

using namespace c74::min;

class elixir : public object<elixir>, public vector_operator<> {
  public:
    MIN_DESCRIPTION{"A crossfading mixer for managing gain structure. 2-10 inlets, each gated by a "
                    "toggle; active inlets share the gain equally and changes are slewed."};
    MIN_TAGS{"mixing"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.limi~, matrix~, *~, line~"};

    outlet<> m_out{this, "(signal) mixed output", "signal"};

    elixir(const atoms& args = {}) {
        int num = 2;
        if (!args.empty()) {
            num = std::clamp(static_cast<int>(args[0]), 2, k_max);
        }
        m_num_inlets = num;
        if (args.size() > 1) {
            m_gtime = static_cast<double>(args[1]);
        }

        for (int i = 0; i < num; ++i) {
            m_inlets.push_back(std::make_unique<inlet<>>(this, "(signal) input / toggle gate"));
            m_time[i] = m_gtime;
        }
    }

    message<> m_int{this, "int", "Toggle a channel on/off (uses the global slew time).",
                    MIN_FUNCTION{
                        doit(static_cast<long>(args[0]), inlet, 0);
                        return {};
                    }};

    message<> m_list{this, "list", "[toggle slewtime] — toggle a channel with a specific slew time.",
                     MIN_FUNCTION{
                         if (args.size() >= 2) {
                             m_time[std::clamp(inlet, 0, k_max - 1)] = args[1];
                         }
                         doit(static_cast<long>(args[0]), inlet, 1);
                         return {};
                     }};

    message<> gtime{this, "gtime", "Set a global slew time (ms) for all channels.",
                    MIN_FUNCTION{
                        m_gtime = args[0];
                        for (int i = 0; i < k_max; ++i) {
                            m_time[i] = (m_gtime > 0.0) ? m_gtime : 0.0;
                        }
                        return {};
                    }};

    void operator()(audio_bundle input, audio_bundle output) override {
        const long n     = input.frame_count();
        const long count = std::min(static_cast<long>(m_num_inlets), input.channel_count());
        double*    out   = output.samples(0);

        for (long k = 0; k < n; ++k) {
            double val = 0.0;
            for (long i = 0; i < count; ++i) {
                const double gain = m_line_toggle[i] ? interpolate(i) : m_mult[i];
                val += input.samples(i)[k] * gain;
            }
            out[k] = val * 0.95;
        }
    }

  private:
    static constexpr int k_max{10};

    std::vector<std::unique_ptr<inlet<>>> m_inlets;
    int                                   m_num_inlets{2};
    double                                m_gtime{0.0};

    std::array<long, k_max>   m_toggle{};
    std::array<double, k_max> m_mult{};
    std::array<double, k_max> m_time{};

    std::array<int, k_max>    m_line_toggle{};
    std::array<double, k_max> m_line_result{};
    std::array<double, k_max> m_line_step{};
    std::array<double, k_max> m_line_dest{};
    std::array<long, k_max>   m_line_num{};
    std::array<long, k_max>   m_line_done{};

    void doit(long val, int in, int typeofmess) {
        in           = std::clamp(in, 0, k_max - 1);
        m_toggle[in] = val;
        if (typeofmess == 0) {
            m_time[in] = m_gtime;
        }

        long active = 0;
        for (int i = 0; i < k_max; ++i) {
            if (m_toggle[i] == 1) {
                ++active;
            }
        }

        const double nn = static_cast<double>(active);
        for (int i = 0; i < k_max; ++i) {
            if (nn != 0.0) {
                m_mult[i] = (m_toggle[i] != 0) ? (1.0 / nn) : 0.0;
            }
            else {
                m_mult[i] = 0.0;
            }
        }
        for (int i = 0; i < k_max; ++i) {
            interp_calc(i);
        }
    }

    void interp_calc(int i) {
        if (m_time[i] != 0.0) {
            const double slew = m_time[i] * 0.001 * samplerate(); // ms -> samples
            m_line_toggle[i]  = 1;
            const double ff   = m_line_result[i];
            m_line_step[i]    = (m_mult[i] - ff) / slew;
            m_line_dest[i]    = m_mult[i];
            m_line_num[i]     = static_cast<long>(slew);
            m_line_done[i]    = 0;
        }
        else {
            m_line_toggle[i] = 0;
        }
    }

    double interpolate(int i) {
        if (m_line_done[i] != m_line_num[i]) {
            m_line_result[i] += m_line_step[i];
            ++m_line_done[i];
        }
        return m_line_result[i];
    }
};

MIN_EXTERNAL(elixir);
