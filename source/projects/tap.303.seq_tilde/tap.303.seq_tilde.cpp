/// @file
/// @brief      tap.303.seq~ — one step-sequenced bass line (the tap.303~ note contract).
/// @details    A thin Min wrapper around the shared step-sequencer kernel in step_seq.h
///             (taptools::seq::note_row — the engine, the swing warp, the slot store, the
///             pinned gate duty and accent levels all live there; see its header for the
///             model and plans/tap.seq.md for the design of record).
///
///             The clocking idiom: the inlet takes a phase ramp, 0..1 per pattern cycle —
///             connect phasor~ (transport-locked if you like); tap.808.seq~ rows on the same
///             phasor stay sample-locked with it. The row emits exactly the tap.303~ inlet
///             pair: a pitch signal (MIDI note number, held between notes) and a gate signal
///             whose edge amplitude is 1.0 for a plain step, 2.0 for an accented one (the
///             voice reads accent depth = amplitude - 1; the accent *amount* stays on the
///             voice's knob, like the hardware). The gate opens at the step start and closes
///             halfway through the step (Open303's measured duty). A step with the slide
///             flag is approached legato: the gate holds through the boundary while the
///             pitch steps, and the voice's ~60 ms glide does the rest — no retrigger,
///             chained slides chain, and a slide across the pattern wrap works.
///
///             Per-step data mirrors the hardware pattern memory: pitch, gate/rest, accent,
///             slide (pitch as a MIDI float subsumes the octave up/down flags; transpose
///             live with @transpose). The 1981 entry workflow is not recreated — patterns
///             are lists, step messages, slots, and dictionaries.
/// @author     Timothy Place
/// @copyright  Copyright 2026 Timothy Place. Distributed under the New BSD License.

#include <atomic>

#include <taptools/step_seq.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = taptools::seq;

class seq303 : public object<seq303>, public vector_operator<> {
  private:
    // Constructed before the attributes below so attribute setters can forward into it.
    kernel::note_row m_row;
    std::atomic<int> m_step_now{-1};
    int              m_step_sent{-1};

  public:
    MIN_DESCRIPTION{"A step-sequenced acid bass line for tap.303~. Feed a phase ramp (phasor~, "
                    "0..1 per pattern cycle) and the row emits the voice's inlet pair: pitch "
                    "(MIDI note number) and a gate whose edge amplitude encodes accent (1 "
                    "plain, 2 accented), with the hardware's slide realized as gate-hold across "
                    "the step boundary — the voice glides legato without retriggering. Per-step "
                    "pitch, gate/rest, accent, and slide; sixteen steps by default (up to 64), "
                    "swing, live transpose, 16 pattern slots with boundary-quantized recall, "
                    "and dictionary save/recall. The gate edge also triggers any tap.808.* "
                    "voice — a bass line doubling a kick is one patch cord."};
    MIN_TAGS{"sequencing, synthesis"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.303~, tap.808.seq~, tap.adsr~, phasor~"};

    inlet<>  m_in{this, "(signal) phase ramp: 0..1 per pattern cycle (connect phasor~)"};
    outlet<> m_out_pitch{this, "(signal) pitch as MIDI note number -> tap.303~ left inlet", "signal"};
    outlet<> m_out_gate{this, "(signal) gate; edge amplitude 1..2 = accent -> tap.303~ right inlet", "signal"};
    outlet<> m_out_step{this, "(int) current step number, 1-based, at each step change"};

    seq303(const atoms& args = {}) { m_row.prepare(samplerate()); }

    attribute<int> length{this, "length", kernel::k_default_length, setter{ MIN_FUNCTION {
                              const int v = std::clamp(static_cast<int>(args[0]), 1, kernel::k_max_steps);
                              m_row.clock().data().set_length(v);
                              return {v};
                          }},
                          description{"Steps per pattern cycle (1..64, default 16). A different length than the "
                                      "other rows on the same phasor is polymeter."}};

    attribute<number> swing{this, "swing", 0.0, setter{ MIN_FUNCTION {
                                const double v = std::clamp(static_cast<double>(args[0]), 0.0, 1.0);
                                m_row.clock().set_swing(v);
                                return {v};
                            }},
                            description{"Swing 0..1: odd-numbered steps play late by up to half a step (0.67 is "
                                        "the classic triplet shuffle). The hardware had none — 0 is stock."}};

    attribute<number> transpose{this, "transpose", 0.0, setter{ MIN_FUNCTION {
                                    const double v = std::clamp(static_cast<double>(args[0]), -48.0, 48.0);
                                    m_row.set_transpose(v);
                                    return {v};
                                }},
                                description{"Live transposition in semitones (-48..48), applied as notes fire — "
                                            "what the hardware's transpose mode did, without the mode."}};

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
                                           "pattern top), step, or now."}};

    attribute<bool> mute{this, "mute", false,
                         description{"Force the gate low (the clock keeps running; pitch holds)."}};

    // -- messages -------------------------------------------------------------------------------------

    message<> step{this, "step",
                   "Set one step and gate it on: step <number 1..64> <pitch> [accent 0/1] [slide 0/1]. "
                   "Pitch is a MIDI note number; fractional values are cents.",
                   MIN_FUNCTION {
                       if (args.size() < 2) {
                           return {};
                       }
                       const int k = static_cast<int>(args[0]) - 1;
                       if (k < 0 || k >= kernel::k_max_steps) {
                           return {};
                       }
                       auto& st  = m_row.clock().data().steps[k];
                       st.pitch  = args[1];
                       st.gate   = true;
                       st.accent = args.size() > 2 && static_cast<int>(args[2]) != 0;
                       st.slide  = args.size() > 3 && static_cast<int>(args[3]) != 0;
                       return {};
                   }};

    message<> rest{this, "rest", "Silence one step: rest <number 1..64>.",
                   MIN_FUNCTION {
                       if (args.empty()) {
                           return {};
                       }
                       const int k = static_cast<int>(args[0]) - 1;
                       if (k >= 0 && k < kernel::k_max_steps) {
                           m_row.clock().data().steps[k].gate = false;
                       }
                       return {};
                   }};

    message<> pitches{this, "pitches", "Set per-step pitches (MIDI note numbers): pitches <p1> ... <pN>.",
                      MIN_FUNCTION {
                          auto&        p = m_row.clock().data();
                          const size_t n = std::min(args.size(), static_cast<size_t>(kernel::k_max_steps));
                          for (size_t k = 0; k < n; ++k) {
                              p.steps[k].pitch = args[k];
                          }
                          return {};
                      }};

    message<> gates{this, "gates", "Set which steps sound: gates <0/1> ... (0 = rest).",
                    MIN_FUNCTION {
                        auto&        p = m_row.clock().data();
                        const size_t n = std::min(args.size(), static_cast<size_t>(kernel::k_max_steps));
                        for (size_t k = 0; k < n; ++k) {
                            p.steps[k].gate = static_cast<int>(args[k]) != 0;
                        }
                        return {};
                    }};

    message<> accents{this, "accents", "Set per-step accent flags: accents <0/1> ....",
                      MIN_FUNCTION {
                          auto&        p = m_row.clock().data();
                          const size_t n = std::min(args.size(), static_cast<size_t>(kernel::k_max_steps));
                          for (size_t k = 0; k < n; ++k) {
                              p.steps[k].accent = static_cast<int>(args[k]) != 0;
                          }
                          return {};
                      }};

    message<> slides{this, "slides",
                     "Set per-step slide flags: slides <0/1> .... A flagged step is approached legato — "
                     "the gate holds through the boundary and the voice glides to it.",
                     MIN_FUNCTION {
                         auto&        p = m_row.clock().data();
                         const size_t n = std::min(args.size(), static_cast<size_t>(kernel::k_max_steps));
                         for (size_t k = 0; k < n; ++k) {
                             p.steps[k].slide = static_cast<int>(args[k]) != 0;
                         }
                         return {};
                     }};

    message<> store{this, "store", "Store the pattern in a slot (1..16).",
                    MIN_FUNCTION {
                        if (!args.empty()) {
                            m_row.clock().store(static_cast<int>(args[0]) - 1);
                        }
                        return {};
                    }};

    message<> recall{this, "recall",
                     "Recall a slot (1..16); takes over at the quantize boundary (default: the next "
                     "pattern top).",
                     MIN_FUNCTION {
                         if (!args.empty()) {
                             m_row.clock().recall(static_cast<int>(args[0]) - 1);
                         }
                         return {};
                     }};

    message<> todict{this, "todict", "Write the pattern into a named dictionary: todict <name>.",
                     MIN_FUNCTION {
                         if (args.empty()) {
                             return {};
                         }
                         dict  d{symbol(args[0])};
                         auto& p = m_row.clock().data();
                         atoms pitches_, gates_, accents_, slides_;
                         for (int k = 0; k < p.length; ++k) {
                             pitches_.push_back(p.steps[k].pitch);
                             gates_.push_back(p.steps[k].gate ? 1 : 0);
                             accents_.push_back(p.steps[k].accent ? 1 : 0);
                             slides_.push_back(p.steps[k].slide ? 1 : 0);
                         }
                         set_key(d, "length", atoms{p.length});
                         set_key(d, "pitches", pitches_);
                         set_key(d, "gates", gates_);
                         set_key(d, "accents", accents_);
                         set_key(d, "slides", slides_);
                         d.touch();
                         return {};
                     }};

    message<> fromdict{this, "fromdict", "Read a pattern written by todict: fromdict <name>.",
                       MIN_FUNCTION {
                           if (args.empty()) {
                               return {};
                           }
                           try {
                               dict        d{symbol(args[0])};
                               const atoms ln = d.at("length");
                               const atoms pv = d.at("pitches");
                               const atoms gv = d.at("gates");
                               const atoms av = d.at("accents");
                               const atoms sv = d.at("slides");
                               if (ln.empty()) {
                                   throw std::runtime_error("empty");
                               }
                               const int n = std::clamp(static_cast<int>(ln[0]), 1, kernel::k_max_steps);
                               auto&     p = m_row.clock().data();
                               p.set_length(n);
                               length = n;
                               for (int k = 0; k < n; ++k) {
                                   auto& st = p.steps[k];
                                   if (k < static_cast<int>(pv.size())) {
                                       st.pitch = pv[k];
                                   }
                                   if (k < static_cast<int>(gv.size())) {
                                       st.gate = static_cast<int>(gv[k]) != 0;
                                   }
                                   if (k < static_cast<int>(av.size())) {
                                       st.accent = static_cast<int>(av[k]) != 0;
                                   }
                                   if (k < static_cast<int>(sv.size())) {
                                       st.slide = static_cast<int>(sv[k]) != 0;
                                   }
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
                        m_row.reset();
                        return {};
                    }};

    message<> dspsetup{this, "dspsetup", "Reconfigure for the sample rate when DSP starts.",
                       MIN_FUNCTION {
                           m_row.prepare(samplerate());
                           return {};
                       }};

    // Step-change notification: coalescing single-value handoff from the audio thread to the
    // scheduler (the tap.sift~ ring's little sibling — the newest index wins).
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
        const long    n         = input.frame_count();
        const double* in        = input.samples(0);
        double*       pitch_out = output.samples(0);
        double*       gate_out  = output.samples(1);
        const bool    muted     = mute;

        if (!m_in.has_signal_connection()) {
            std::fill_n(pitch_out, n, 0.0);
            std::fill_n(gate_out, n, 0.0);
            return;
        }
        for (long i = 0; i < n; ++i) {
            const auto o = m_row.process(in[i]);
            pitch_out[i] = o.pitch;
            gate_out[i]  = muted ? 0.0 : o.gate;
        }
        const int k = m_row.clock().current_step();
        if (k != m_step_now.load(std::memory_order_relaxed)) {
            m_step_now.store(k, std::memory_order_release);
            m_notify.delay(0);
        }
    }

    // Set a list value on a dictionary key (atom_reference only assigns scalars).
    static void set_key(dict& d, const char* key, const atoms& v) {
        if (v.empty()) {
            return;
        }
        c74::max::dictionary_appendatoms(reinterpret_cast<c74::max::t_dictionary*>(static_cast<c74::max::t_object*>(d)),
                                         symbol(key), static_cast<long>(v.size()),
                                         reinterpret_cast<c74::max::t_atom*>(const_cast<atom*>(v.data())));
    }

    // Exposed for unit tests.
  public:
    kernel::note_row& row() { return m_row; }
};

MIN_EXTERNAL(seq303);
