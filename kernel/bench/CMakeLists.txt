# CPU benchmarks (kernel-only, no Max dependency). svf_bench times the svf.h kernel over a
# config matrix; the ratchet workflow (compare.py + baselines/) is described in README.md.

add_executable(svf_bench svf_bench.cpp)
target_link_libraries(svf_bench PRIVATE TapTools::taptools)
set_target_properties(svf_bench PROPERTIES
    CXX_STANDARD 20
    CXX_STANDARD_REQUIRED ON
)
