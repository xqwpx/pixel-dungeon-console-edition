function(pd_apply_compiler_settings target)
    # Gameplay already uses C++17 if-with-initializer syntax. Keep one common
    # language level for Windows and Vita so shared sources compile unchanged.
    target_compile_features(${target} PUBLIC cxx_std_17)
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
        target_compile_options(${target} PRIVATE -Wall -Wextra -Wno-unused-parameter)
    endif()
endfunction()
