cmake_minimum_required(VERSION 3.20.5)

aux_source_directory(plugin plugin_files)
aux_source_directory(src src_files)
aux_source_directory(src/effects src_effects_files)
aux_source_directory(src/io src_io_files)
aux_source_directory(src/notes src_notes_files)
aux_source_directory(src/onset src_onset_files)
aux_source_directory(src/pitch src_pitch_files)
aux_source_directory(src/spectral src_spectral_files)
aux_source_directory(src/synth src_synth_files)
aux_source_directory(src/tempo src_tempo_files)
aux_source_directory(src/temporal src_temporal_files)
aux_source_directory(src/utils src_utils_files)

set(CMAKE_C_FLAGS "-D_VAMP_NO_HOST_NAMESPACE -D_VAMP_NO_PLUGIN_NAMESPACE -D_VAMP_IN_PLUGINSDK \
 -Wno-unused-parameter -Wno-missing-field-initializers -Wno-unused-value -Wno-sign-compare \
 -Wno-type-limits -Wall -Wextra -O2 -Os -fPIC -DHAVE_STDLIB_H=1 -DHAVE_STDIO_H=1 -DHAVE_MATH_H=1 \
 -DHAVE_STRING_H=1 -DHAVE_ERRNO_H=1 -DHAVE_LIMITS_H=1 -DHAVE_STDARG_H=1 -DHAVE_GETOPT_H=1 \
 -DHAVE_UNISTD_H=1")

set(CMAKE_CXX_FLAGS "-D_VAMP_NO_HOST_NAMESPACE -D_VAMP_NO_PLUGIN_NAMESPACE -Wno-unused-value")

include_directories(.
        ..
        ../..
        ../../vamp
        ../../vamp/vamp-sdk
        ../../../jni
        src
        src/effects
        src/io
        src/notes
        src/onset
        src/pitch
        src/spectral
        src/synth
        src/tempo
        src/temporal
        src/utils
        )

add_library( NIOAubio
        STATIC
        ${plugin_files}
        ${src_files}
        ${src_effects_files}
        ${src_io_files}
        ${src_notes_files}
        ${src_onset_files}
        ${src_pitch_files}
        ${src_spectral_files}
        ${src_synth_files}
        ${src_tempo_files}
        ${src_temporal_files}
        ${src_utils_files}
        AubioPlugin.cpp
        AubioVampPluginWrapper.cpp
        )

find_library( log-lib
        log )
target_link_libraries( NIOAubio
        ${log-lib})