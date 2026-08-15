/// @file
/// @brief      tap.period — how long until a set of free-running loops realigns.
/// @details    The one thing a patch of independent tap.reel~ genuinely loses next to
///             tap.airport~: the composite period, the least common multiple of the loop lengths,
///             which needs every length at once and so has nowhere to live inside a single reel.
///             This object is that arithmetic on its own, wrapping the free function
///             tap::tools::airport::composite_period_seconds (taptools/airport.h) — the same code
///             the bank calls, so a patch and the monolith cannot disagree.
///
///             Give it a list of loop lengths in seconds and it reports the period in seconds.
///             Each length is first quantized to a whole number of samples exactly as a reel
///             quantizes it (the 0.5 s floor, then ceil to samples), because the lcm is over
///             sample counts and not over real numbers — 6.3 and 7.7 seconds are commensurate as
///             decimals and wildly not as sample counts. That quantization is a shared kernel
///             function rather than a copy, which is what keeps the agreement honest.
///
///             `inf` is a correct and desirable answer, not an error: it means the lcm left the
///             64-bit range, so the piece will not repeat in any timescale you care about. Seven
///             airport-scale lengths in awkward ratios reach it easily, which is the whole idea.
///
///             The @sr attribute must match the sample rate the reels are actually running at, or
///             the quantization — and therefore the answer — will be for a different machine. It
///             defaults to the current global rate.
/// @author     Timothy Place
// SPDX-License-Identifier: BSD-3-Clause
// Copyright 2026 Timothy Place.

#include <algorithm>
#include <vector>

#include <taptools/airport.h>

#include "c74_min.h"

using namespace c74::min;
namespace kernel = tap::tools::airport;

class lcmperiod : public object<lcmperiod> {
  public:
    MIN_DESCRIPTION{"How long until a set of free-running loops realigns: the least common "
                    "multiple of their lengths, in seconds. Send a list of loop lengths in "
                    "seconds — each is quantized to samples exactly as tap.reel~ quantizes it, "
                    "because the lcm is over sample counts, not decimals. inf is a correct "
                    "answer, not an error: it means the period left the 64-bit range and the "
                    "piece will not repeat. This is the arithmetic tap.airport~ reports for its "
                    "own lanes, for a patch of tap.reel~ that has no bank to ask."};
    MIN_TAGS{"utilities"};
    MIN_AUTHOR{"Timothy Place"};
    MIN_RELATED{"tap.reel~, tap.airport~, tap.prime"};

    inlet<>  m_in{this, "(list) loop lengths in seconds, or samples <n> <n> ..."};
    outlet<> m_out{this, "(float) the composite period in seconds; inf means it will not repeat"};

    attribute<number> sr{this, "sr", c74::max::sys_getsr(), setter{MIN_FUNCTION{
                             const double v = (static_cast<double>(args[0]) > 0.0) ? static_cast<double>(args[0])
                                                                                   : c74::max::sys_getsr();
                             return {v};
                         }},
                         description{"The sample rate the loops are running at. The lcm is over sample counts, so "
                                     "this must match the reels or the answer is for a different machine. Defaults "
                                     "to the current global rate."}};

    message<> list{this, "list",
                   "A list of loop lengths in SECONDS. Each is floored at 0.5 s and rounded up to whole samples, "
                   "exactly as tap.reel~ does, then the lcm of those sample counts is reported in seconds.",
                   MIN_FUNCTION{
                       if (args.empty()) {
                           return {};
                       }
                       std::vector<long> lengths;
                       lengths.reserve(args.size());
                       for (const auto& a : args) {
                           lengths.push_back(kernel::loop_samples_for(static_cast<double>(a), sr));
                       }
                       emit(lengths);
                       return {};
                   }};

    message<> samples{this, "samples",
                      "samples <n> <n> ...: the same question asked in whole samples — what tap.reel~'s "
                      "loopsamples message reports. Exact, since it skips the quantization entirely.",
                      MIN_FUNCTION{
                          if (args.empty()) {
                              return {};
                          }
                          std::vector<long> lengths;
                          lengths.reserve(args.size());
                          for (const auto& a : args) {
                              lengths.push_back(static_cast<long>(static_cast<double>(a)));
                          }
                          emit(lengths);
                          return {};
                      }};

  private:
    void emit(const std::vector<long>& lengths) {
        m_out.send(kernel::composite_period_seconds(lengths.data(), static_cast<int>(lengths.size()), sr));
    }
};

MIN_EXTERNAL(lcmperiod);
