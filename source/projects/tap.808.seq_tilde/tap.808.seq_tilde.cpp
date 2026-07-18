/// @file
/// @brief      tap.808.seq~ — one step-sequencer drum row (the tap.808.* trigger contract).
/// @details    A thin Min wrapper around the shared step-sequencer kernel in step_seq.h
///             (taptools::seq::trigger_row — the engine, the swing warp, the slot store, and
///             the pinned trigger levels all live there; see its header for the model and
///             plans/tap.seq.md for the design of record).
///
///             The clocking idiom: the inlet takes a phase ramp, 0..1 per pattern cycle —
///             connect phasor~ (transport-locked if you like). The row derives its step from
///             the phase, so every row fed by one phasor stays phase-coherent forever, and
///             different @length values off the same ramp are polymeter. At each (swung) step
///             start the row emits an impulse whose amplitude is the step's velocity 0..1 —
///             the family's amplitude-as-accent trigger, wired straight into any tap.808.*
///             voice (or tap.adsr~, or anything edge-triggered; @pulse widens the impulse
///             into a held gate for envelope consumers).
///
///             Twelve rows = twelve of these off one phasor, one per voice — the hardware's
///             row decomposition as patch cords. The accent row falls out: set the same
///             accents list on every row for the shared-bus feel.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <taptools/step_seq.h>

#include <atomic>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = taptools::seq;

class seq808 : public object<seq808>, public vector_operator<> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    kernel::trigger_row m_row;
    bool                m_hit[kernel::k_max_steps]{};
    bool                m_flag[kernel::k_max_steps]{};
    double              m_plain{kernel::k_trig_plain};
    double              m_accented{kernel::k_trig_accented};
    std::atomic<int>    m_step_now{-1};
    int                 m_step_sent{-1};

  public:
    MIN_DESCRIPTION{"One step-sequencer drum row for the tap.808 family. Feed a phase ramp "
                    "(phasor~, 0..1 per pattern cycle) and the row emits a trigger impulse at "
                    "each sounding step whose amplitude 0..1 is the accent — exactly the "
                    "tap.808.* trigger contract, modeled on the hardware's common 4-14 V bus. "
                    "Per-step velocities (0 = rest), or the hardware view via hits/accents "
                    "lists mapped onto the plain and accented levels. Sixteen steps by default "
                    "(up to 64 — different lengths off one phasor are polymeter), swing, 16 "
                    "pattern slots with boundary-quantized recall (the A/B-half and fill "
                    "switching), and dictionary save/recall. One row per voice, all off one "
                    "phasor: the 808's sequencer as patch cords."};
    MIN_TAGS{"sequencing, drums"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.303.seq~, tap.808.kick~, tap.808.hat~, tap.adsr~, phasor~"};

    inlet<>  m_in{this, "(signal) phase ramp: 0..1 per pattern cycle (connect phasor~)"};
    outlet<> m_out{this, "(signal) triggers: impulse amplitude 0..1 = accent", "signal"};
    outlet<> m_out_step{this, "(int) current step number, 1-based, at each step change"};

    seq808(const atoms& args = {}) { m_row.prepare(samplerate()); }

    attribute<int> length{this, "length", kernel::k_default_length, setter{ MIN_FUNCTION {
                              const int v = std::clamp(static_cast<int>(args[0]), 1, kernel::k_max_steps);
                              m_row.clock().data().set_length(v);
                              return {v};
                          }},
                          description{"Steps per pattern cycle (1..64, default 16). Different lengths on rows "
                                      "sharing one phasor run in polymeter — length 12 against 16 is the "
                                      "hardware's triplet pre-scale and then some."}};

    attribute<number> swing{this, "swing", 0.0, setter{ MIN_FUNCTION {
                                const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                m_row.clock().set_swing(v);
                                return {v};
                            }},
                            description{"Swing 0..1: odd-numbered steps play late by up to half a step "
                                        "(0.67 is the classic triplet shuffle). The hardware had none — 0 is "
                                        "stock."}};

    attribute<number> plain{this, "plain", kernel::k_trig_plain, setter{ MIN_FUNCTION {
                                m_plain = std::clamp(static_cast<double>(args[0]), 0.002, 1.0);
                                sync_flag_velocities();
                                return {m_plain};
                            }},
                            description{"Trigger level for un-accented hits (0..1 amplitude). The default 0.01 "
                                        "is the hardware's 4 V base trigger — accent ~0, kept just above the "
                                        "voices' edge threshold."}};

    attribute<number> accented{this, "accented", kernel::k_trig_accented, setter{ MIN_FUNCTION {
                                   m_accented = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                   sync_flag_velocities();
                                   return {m_accented};
                               }},
                               description{"Trigger level for accented hits (0..1 amplitude). The default 0.5 "
                                           "is the accent level knob at noon; 1.0 is the full 14 V bus."}};

    attribute<number> pulse{this, "pulse", 0.0, setter{ MIN_FUNCTION {
                                const double v = std::max(0.0, static_cast<double>(args[0]));
                                m_row.set_pulse_ms(v);
                                return {v};
                            }},
                            description{"Impulse width in ms. 0 (default) is a single-sample edge — the family "
                                        "contract; longer holds a gate for driving envelopes. Keep it shorter "
                                        "than a step, or back-to-back triggers merge."}};

    attribute<symbol> quantize{this, "quantize", "cycle", setter{ MIN_FUNCTION {
                                   const symbol s = args[0];
                                   const int    m = (s == symbol("now"))    ? kernel::quantize_now
                                                    : (s == symbol("step")) ? kernel::quantize_step
                                                                            : kernel::quantize_cycle;
                                   m_row.clock().set_quantize(m);
                                   return {m == kernel::quantize_now    ? symbol("now")
                                           : m == kernel::quantize_step ? symbol("step")
                                                                        : symbol("cycle")};
                               }},
                               description{"When a recalled pattern takes over: cycle (default — the next "
                                           "pattern top, the hardware's A/B and fill feel), step, or now."}};

    attribute<bool> mute{this, "mute", false, description{"Silence the trigger output (the clock keeps running)."}};

    // -- messages -------------------------------------------------------------------------------------

    message<> velocities{this, "velocities",
                         "Set per-step trigger levels directly: velocities <v1> ... <vN> (0..1 each, 0 = rest).",
                         MIN_FUNCTION {
                             auto& p = m_row.clock().data();
                             const size_t n = std::min(args.size(), static_cast<size_t>(kernel::k_max_steps));
                             for (size_t k = 0; k < n; ++k) {
                                 const double v        = std::clamp(static_cast<double>(args[k]), 0.0, 1.0);
                                 p.steps[k].velocity   = v;
                                 m_hit[k]              = v > 0.0;
                                 m_flag[k]             = v >= m_accented;
                             }
                             return {};
                         }};

    message<> hits{this, "hits",
                   "The hardware view: hits <0/1> ... — which steps sound, at the plain level (accented "
                   "steps use the accented level). Overwrites the row's velocities.",
                   MIN_FUNCTION {
                       const size_t n = std::min(args.size(), static_cast<size_t>(kernel::k_max_steps));
                       for (size_t k = 0; k < n; ++k)
                           m_hit[k] = static_cast<int>(args[k]) != 0;
                       sync_flag_velocities();
                       return {};
                   }};

    message<> accents{this, "accents",
                      "The hardware view: accents <0/1> ... — which sounding steps are accented. "
                      "Overwrites the row's velocities.",
                      MIN_FUNCTION {
                          const size_t n = std::min(args.size(), static_cast<size_t>(kernel::k_max_steps));
                          for (size_t k = 0; k < n; ++k)
                              m_flag[k] = static_cast<int>(args[k]) != 0;
                          sync_flag_velocities();
                          return {};
                      }};

    message<> step{this, "step", "Set one step: step <number 1..64> <velocity 0..1>.",
                   MIN_FUNCTION {
                       if (args.size() < 2)
                           return {};
                       const int k = static_cast<int>(args[0]) - 1;
                       if (k < 0 || k >= kernel::k_max_steps)
                           return {};
                       const double v                       = std::clamp(static_cast<double>(args[1]), 0.0, 1.0);
                       m_row.clock().data().steps[k].velocity = v;
                       m_hit[k]                             = v > 0.0;
                       m_flag[k]                            = v >= m_accented;
                       return {};
                   }};

    message<> store{this, "store", "Store the pattern in a slot (1..16).",
                    MIN_FUNCTION {
                        if (!args.empty())
                            m_row.clock().store(static_cast<int>(args[0]) - 1);
                        return {};
                    }};

    message<> recall{this, "recall",
                     "Recall a slot (1..16); takes over at the quantize boundary (default: the next "
                     "pattern top — the A/B-half / fill switch).",
                     MIN_FUNCTION {
                         if (!args.empty())
                             m_row.clock().recall(static_cast<int>(args[0]) - 1);
                         return {};
                     }};

    message<> todict{this, "todict", "Write the pattern into a named dictionary: todict <name>.",
                     MIN_FUNCTION {
                         if (args.empty())
                             return {};
                         dict  d{symbol(args[0])};
                         auto& p = m_row.clock().data();
                         atoms v;
                         for (int k = 0; k < p.length; ++k)
                             v.push_back(p.steps[k].velocity);
                         set_key(d, "length", atoms{p.length});
                         set_key(d, "velocities", v);
                         d.touch();
                         return {};
                     }};

    message<> fromdict{this, "fromdict", "Read a pattern written by todict: fromdict <name>.",
                       MIN_FUNCTION {
                           if (args.empty())
                               return {};
                           try {
                               dict        d{symbol(args[0])};
                               const atoms ln = d.at("length");
                               const atoms v  = d.at("velocities");
                               if (ln.empty() || v.empty())
                                   throw std::runtime_error("empty");
                               const int n = std::clamp(static_cast<int>(ln[0]), 1, kernel::k_max_steps);
                               auto&       p = m_row.clock().data();
                               p.set_length(n);
                               length = n;
                               for (int k = 0; k < n && k < static_cast<int>(v.size()); ++k) {
                                   const double x      = std::clamp(static_cast<double>(v[k]), 0.0, 1.0);
                                   p.steps[k].velocity = x;
                                   m_hit[k]            = x > 0.0;
                                   m_flag[k]           = x >= m_accented;
                               }
                           }
                           catch (...) {
                               cerr << "fromdict: no pattern found in that dictionary" << endl;
                           }
                           return {};
                       }};

    message<> clear{this, "clear", "Empty the pattern (all rests) and reset the clock state.",
                    MIN_FUNCTION {
                        m_row.clock().data().clear();
                        for (auto& h : m_hit)
                            h = false;
                        for (auto& f : m_flag)
                            f = false;
                        m_row.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reconfigure for the sample rate when DSP starts.",
                       MIN_FUNCTION {
                           m_row.prepare(samplerate());
                           return {};
                       }};

    // Step-change notification: coalescing single-value handoff from the audio thread to the
    // scheduler (the tap.sift~ ring's little sibling — the newest index wins, which is all a
    // step display wants).
    timer<> m_notify{this,
                     MIN_FUNCTION {
                         const int k = m_step_now.load(std::memory_order_acquire);
                         if (k >= 0 && k != m_step_sent) {
                             m_step_sent = k;
                             m_out_step.send(k + 1);
                         }
                         return {};
                     }};

    // -- perform --------------------------------------------------------------------------------------

    void operator()(audio_bundle input, audio_bundle output) {
        const long    n     = input.frame_count();
        const double* in    = input.samples(0);
        double*       out   = output.samples(0);
        const bool    muted = mute;

        if (!m_in.has_signal_connection()) {
            std::fill_n(out, n, 0.0);
            return;
        }
        for (long i = 0; i < n; ++i) {
            const double y = m_row.process(in[i]);
            out[i]         = muted ? 0.0 : y;
        }
        const int k = m_row.clock().current_step();
        if (k != m_step_now.load(std::memory_order_relaxed)) {
            m_step_now.store(k, std::memory_order_release);
            m_notify.delay(0);
        }
    }

    // Exposed for unit tests.
    kernel::trigger_row& row() { return m_row; }

  private:
    // Set a list value on a dictionary key (atom_reference only assigns scalars).
    static void set_key(dict& d, const char* key, const atoms& v) {
        if (v.empty())
            return;
        c74::max::dictionary_appendatoms(reinterpret_cast<c74::max::t_dictionary*>(static_cast<c74::max::t_object*>(d)),
                                    symbol(key), static_cast<long>(v.size()),
                                    reinterpret_cast<c74::max::t_atom*>(const_cast<atom*>(v.data())));
    }

    void sync_flag_velocities() {
        auto& p = m_row.clock().data();
        for (int k = 0; k < kernel::k_max_steps; ++k)
            p.steps[k].velocity = m_hit[k] ? (m_flag[k] ? m_accented : m_plain) : 0.0;
    }
};

MIN_EXTERNAL(seq808);
