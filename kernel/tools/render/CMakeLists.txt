# Offline render tools (kernel-only, no Max dependency). Each renders test/demo signals
# through its kernel header to WAV files for listening checks — the kernels' reusability
# outside Max, demonstrated in ~150 lines apiece.

foreach (tool ladder_render vco_render grm_comb_render grm_pitchaccum_render)
    add_executable(${tool} ${tool}.cpp)
    target_link_libraries(${tool} PRIVATE TapTools::taptools)
    set_target_properties(${tool} PROPERTIES
        CXX_STANDARD 20
        CXX_STANDARD_REQUIRED ON
    )
endforeach ()
