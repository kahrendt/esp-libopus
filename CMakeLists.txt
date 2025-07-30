# ESP-IDF Component CMakeLists.txt for esp-libopus
# This builds the Opus audio codec as an ESP-IDF component

# Copy our custom stack_alloc.h over the upstream version to ensure it's used
# This is necessary because Opus headers include stack_alloc.h from other headers,
# making it impossible to override with include paths or compiler flags
if(NOT EXISTS ${COMPONENT_DIR}/opus/celt/stack_alloc.h.original)
    # Backup the original file first
    configure_file(${COMPONENT_DIR}/opus/celt/stack_alloc.h 
                   ${COMPONENT_DIR}/opus/celt/stack_alloc.h.original 
                   COPYONLY)
endif()
configure_file(${COMPONENT_DIR}/custom/stack_alloc.h 
               ${COMPONENT_DIR}/opus/celt/stack_alloc.h 
               COPYONLY)

# Base sources that are always needed
set(opus_base_srcs
    # Core Opus source
    "opus/src/opus.c"
    "opus/src/repacketizer.c"
    "opus/src/mapping_matrix.c"
)

# Decoder-specific sources
set(opus_decoder_srcs
    "opus/src/opus_decoder.c"
    "opus/src/opus_multistream_decoder.c"
    "opus/src/opus_projection_decoder.c"
)

# Encoder-specific sources  
set(opus_encoder_srcs
    "opus/src/opus_encoder.c"
    "opus/src/opus_multistream_encoder.c"
    "opus/src/opus_projection_encoder.c"
    "opus/src/opus_multistream.c"
    "opus/src/analysis.c"
    "opus/src/mlp.c"
    "opus/src/mlp_data.c"
)

# Always include all sources (encoder and decoder)
set(opus_srcs ${opus_base_srcs} ${opus_decoder_srcs} ${opus_encoder_srcs})

# Add thread-local storage implementation (always enabled for pseudostack mode)
list(APPEND opus_srcs "custom/thread_local_stack.c")

idf_component_register(
    SRCS 
        ${opus_srcs}
        
        # CELT base sources (always needed)
        "opus/celt/bands.c"
        "opus/celt/celt.c"
        "opus/celt/cwrs.c"
        "opus/celt/entcode.c"
        "opus/celt/entdec.c"
        "opus/celt/kiss_fft.c"
        "opus/celt/laplace.c"
        "opus/celt/mathops.c"
        "opus/celt/mdct.c"
        "opus/celt/modes.c"
        "opus/celt/pitch.c"
        "opus/celt/celt_lpc.c"
        "opus/celt/quant_bands.c"
        "opus/celt/rate.c"
        "opus/celt/vq.c"
        
        # CELT decoder sources
        "opus/celt/celt_decoder.c"
        
        # CELT encoder sources  
        "opus/celt/celt_encoder.c"
        "opus/celt/entenc.c"
        
        # SILK base sources (always needed)
        "opus/silk/CNG.c"
        "opus/silk/code_signs.c"
        "opus/silk/gain_quant.c"
        "opus/silk/interpolate.c"
        "opus/silk/LP_variable_cutoff.c"
        "opus/silk/NLSF_stabilize.c"
        "opus/silk/NLSF_VQ_weights_laroia.c"
        "opus/silk/pitch_est_tables.c"
        "opus/silk/resampler.c"
        "opus/silk/resampler_down2_3.c"
        "opus/silk/resampler_down2.c"
        "opus/silk/resampler_private_AR2.c"
        "opus/silk/resampler_private_down_FIR.c"
        "opus/silk/resampler_private_IIR_FIR.c"
        "opus/silk/resampler_private_up2_HQ.c"
        "opus/silk/resampler_rom.c"
        "opus/silk/shell_coder.c"
        "opus/silk/sigm_Q15.c"
        "opus/silk/sort.c"
        "opus/silk/sum_sqr_shift.c"
        "opus/silk/tables_gain.c"
        "opus/silk/tables_LTP.c"
        "opus/silk/tables_NLSF_CB_NB_MB.c"
        "opus/silk/tables_NLSF_CB_WB.c"
        "opus/silk/tables_other.c"
        "opus/silk/tables_pitch_lag.c"
        "opus/silk/tables_pulses_per_block.c"
        "opus/silk/VAD.c"
        "opus/silk/control_audio_bandwidth.c"
        "opus/silk/quant_LTP_gains.c"
        "opus/silk/VQ_WMat_EC.c"
        "opus/silk/HP_variable_cutoff.c"
        "opus/silk/NLSF_unpack.c"
        "opus/silk/NLSF_del_dec_quant.c"
        "opus/silk/process_NLSFs.c"
        "opus/silk/stereo_LR_to_MS.c"
        "opus/silk/stereo_MS_to_LR.c"
        "opus/silk/check_control_input.c"
        "opus/silk/control_SNR.c"
        "opus/silk/control_codec.c"
        "opus/silk/A2NLSF.c"
        "opus/silk/ana_filt_bank_1.c"
        "opus/silk/biquad_alt.c"
        "opus/silk/bwexpander_32.c"
        "opus/silk/bwexpander.c"
        "opus/silk/debug.c"
        "opus/silk/decode_pitch.c"
        "opus/silk/inner_prod_aligned.c"
        "opus/silk/lin2log.c"
        "opus/silk/log2lin.c"
        "opus/silk/LPC_analysis_filter.c"
        "opus/silk/LPC_inv_pred_gain.c"
        "opus/silk/table_LSF_cos.c"
        "opus/silk/NLSF2A.c"
        "opus/silk/stereo_decode_pred.c"
        "opus/silk/stereo_encode_pred.c"
        "opus/silk/stereo_find_predictor.c"
        "opus/silk/stereo_quant_pred.c"
        "opus/silk/LPC_fit.c"
        
        # SILK decoder sources
        "opus/silk/init_decoder.c"
        "opus/silk/decode_core.c"
        "opus/silk/decode_frame.c"
        "opus/silk/decode_parameters.c"
        "opus/silk/decode_indices.c"
        "opus/silk/decode_pulses.c"
        "opus/silk/decoder_set_fs.c"
        "opus/silk/dec_API.c"
        "opus/silk/NLSF_decode.c"
        "opus/silk/PLC.c"
        
        # SILK encoder sources
        "opus/silk/enc_API.c"
        "opus/silk/encode_indices.c"
        "opus/silk/encode_pulses.c"
        "opus/silk/NSQ.c"
        "opus/silk/NSQ_del_dec.c"
        "opus/silk/NLSF_encode.c"
        "opus/silk/NLSF_VQ.c"
        "opus/silk/init_encoder.c"
        
    INCLUDE_DIRS 
        "opus/include"
        "opus"
        "opus/celt"
        "opus/silk"
        "opus/silk/float"
        "opus/silk/fixed"
        "."  # For config.h
)

# Add custom directory to include path BEFORE opus headers
# This allows our custom_support.h to be found by os_support.h
# And our custom stack_alloc.h to override the original
target_include_directories(${COMPONENT_LIB} BEFORE PRIVATE 
    "${COMPONENT_DIR}/custom"
)

# Get IDF target
idf_build_get_property(target IDF_TARGET)

# Base configuration for all ESP32 variants
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    HAVE_CONFIG_H
    OPUS_BUILD
    OPUS_EXPORT=
)

# Common definitions for all targets
target_compile_definitions(${COMPONENT_LIB} PRIVATE
    # OPUS_HAVE_RTCD must be explicitly disabled
    OPUS_HAVE_RTCD=0
    # ESP-IDF supports C99 lrint/lrintf functions
    HAVE_LRINT
    HAVE_LRINTF
    # Use non-threadsafe pseudostack with custom allocation
    NONTHREADSAFE_PSEUDOSTACK
    # Enable custom support header
    CUSTOM_SUPPORT
)

# Check for simplified manual configuration defines
if(DEFINED CONFIG_OPUS_USE_FLOATING_POINT)
    set(CONFIG_OPUS_FLOATING_POINT ON)
endif()

# Check if floating-point is explicitly enabled (only for ESP32-S3)
if(CONFIG_OPUS_FLOATING_POINT AND target STREQUAL "esp32s3")
    # Floating-point implementation for ESP32-S3
    # Include floating-point SILK sources
    target_sources(${COMPONENT_LIB} PRIVATE
            # Floating-point SILK sources
            "opus/silk/float/apply_sine_window_FLP.c"
            "opus/silk/float/corrMatrix_FLP.c"
            "opus/silk/float/encode_frame_FLP.c"
            "opus/silk/float/find_LPC_FLP.c"
            "opus/silk/float/find_LTP_FLP.c"
            "opus/silk/float/find_pitch_lags_FLP.c"
            "opus/silk/float/find_pred_coefs_FLP.c"
            "opus/silk/float/LPC_analysis_filter_FLP.c"
            "opus/silk/float/LTP_analysis_filter_FLP.c"
            "opus/silk/float/LTP_scale_ctrl_FLP.c"
            "opus/silk/float/noise_shape_analysis_FLP.c"
            "opus/silk/float/process_gains_FLP.c"
            "opus/silk/float/regularize_correlations_FLP.c"
            "opus/silk/float/residual_energy_FLP.c"
            "opus/silk/float/warped_autocorrelation_FLP.c"
            "opus/silk/float/wrappers_FLP.c"
            "opus/silk/float/autocorrelation_FLP.c"
            "opus/silk/float/burg_modified_FLP.c"
            "opus/silk/float/bwexpander_FLP.c"
            "opus/silk/float/energy_FLP.c"
            "opus/silk/float/inner_product_FLP.c"
            "opus/silk/float/k2a_FLP.c"
            "opus/silk/float/LPC_inv_pred_gain_FLP.c"
            "opus/silk/float/pitch_analysis_core_FLP.c"
            "opus/silk/float/scale_copy_vector_FLP.c"
            "opus/silk/float/scale_vector_FLP.c"
            "opus/silk/float/schur_FLP.c"
            "opus/silk/float/sort_FLP.c"
        )
    
    # Floating-point build with optimizations
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        OPUS_ENABLE_FLOAT_API
        FLOATING_POINT
        ESP_PLATFORM
        # Enable fast floating-point approximations for better performance
        FLOAT_APPROX
    )
    
    # Enable fast math for FPU (FLOAT_APPROX makes this safe)
    target_compile_options(${COMPONENT_LIB} PRIVATE
        -ffast-math
        -fno-finite-math-only
    )
else()
    # Default to fixed-point implementation for all ESP32 variants
    # Fixed-point sources for encoder
    target_sources(${COMPONENT_LIB} PRIVATE
            # Fixed-point SILK sources
            "opus/silk/fixed/LTP_analysis_filter_FIX.c"
            "opus/silk/fixed/LTP_scale_ctrl_FIX.c"
            "opus/silk/fixed/corrMatrix_FIX.c"
            "opus/silk/fixed/encode_frame_FIX.c"
            "opus/silk/fixed/find_LPC_FIX.c"
            "opus/silk/fixed/find_LTP_FIX.c"
            "opus/silk/fixed/find_pitch_lags_FIX.c"
            "opus/silk/fixed/find_pred_coefs_FIX.c"
            "opus/silk/fixed/noise_shape_analysis_FIX.c"
            "opus/silk/fixed/process_gains_FIX.c"
            "opus/silk/fixed/regularize_correlations_FIX.c"
            "opus/silk/fixed/residual_energy16_FIX.c"
            "opus/silk/fixed/residual_energy_FIX.c"
            "opus/silk/fixed/warped_autocorrelation_FIX.c"
            "opus/silk/fixed/apply_sine_window_FIX.c"
            "opus/silk/fixed/autocorr_FIX.c"
            "opus/silk/fixed/burg_modified_FIX.c"
            "opus/silk/fixed/k2a_FIX.c"
            "opus/silk/fixed/k2a_Q16_FIX.c"
            "opus/silk/fixed/pitch_analysis_core_FIX.c"
            "opus/silk/fixed/vector_ops_FIX.c"
            "opus/silk/fixed/schur64_FIX.c"
            "opus/silk/fixed/schur_FIX.c"
        )
    
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        FIXED_POINT=1
        DISABLE_FLOAT_API
    )
endif()

# Common optimization flags
target_compile_options(${COMPONENT_LIB} PRIVATE
    -O3
    -ffunction-sections
    -fdata-sections
)

# ESP32-S3 Xtensa LX7 optimizations
if(target STREQUAL "esp32s3")
    # Add custom silk directory to override SILK headers with Xtensa optimizations
    # This must come BEFORE the standard silk directory in the include path
    target_include_directories(${COMPONENT_LIB} BEFORE PRIVATE 
        "${COMPONENT_DIR}/custom/silk"
    )
    
    
    # Define that we have Xtensa LX7 optimizations available
    target_compile_definitions(${COMPONENT_LIB} PRIVATE
        OPUS_XTENSA_LX7
    )
    
    message(STATUS "Opus: Enabling Xtensa LX7 optimizations for ESP32-S3")
endif()

# Create config.h for Opus build
file(WRITE "${CMAKE_CURRENT_BINARY_DIR}/config.h" "
/* Auto-generated config.h for ESP-IDF Opus component */
#ifndef CONFIG_H
#define CONFIG_H

/* Define to 1 if you have the <dlfcn.h> header file. */
/* #undef HAVE_DLFCN_H */

/* Define to 1 if you have the <inttypes.h> header file. */
#define HAVE_INTTYPES_H 1

/* Define to 1 if you have the <memory.h> header file. */
#define HAVE_MEMORY_H 1

/* Define to 1 if you have the <stdint.h> header file. */
#define HAVE_STDINT_H 1

/* Define to 1 if you have the <stdlib.h> header file. */
#define HAVE_STDLIB_H 1

/* Define to 1 if you have the <strings.h> header file. */
#define HAVE_STRINGS_H 1

/* Define to 1 if you have the <string.h> header file. */
#define HAVE_STRING_H 1

/* Define to 1 if you have the <sys/stat.h> header file. */
#define HAVE_SYS_STAT_H 1

/* Define to 1 if you have the <sys/types.h> header file. */
#define HAVE_SYS_TYPES_H 1

/* Define to 1 if you have the <unistd.h> header file. */
#define HAVE_UNISTD_H 1

/* Define to the address where bug reports for this package should be sent. */
#define PACKAGE_BUGREPORT \"opus@xiph.org\"

/* Define to the full name of this package. */
#define PACKAGE_NAME \"opus\"

/* Define to the full name and version of this package. */
#define PACKAGE_STRING \"opus 1.4\"

/* Define to the one symbol short name of this package. */
#define PACKAGE_TARNAME \"opus\"

/* Define to the version of this package. */
#define PACKAGE_VERSION \"1.4\"

/* The size of `int', as computed by sizeof. */
#define SIZEOF_INT 4

/* The size of `long', as computed by sizeof. */
#define SIZEOF_LONG 4

/* The size of `long long', as computed by sizeof. */
#define SIZEOF_LONG_LONG 8

/* The size of `short', as computed by sizeof. */
#define SIZEOF_SHORT 2

/* Define to 1 if you have the ANSI C header files. */
#define STDC_HEADERS 1

/* Version number of package */
#define VERSION \"1.4\"

/* Define to `__inline__' or `__inline' if that's what the C compiler
   calls it, or to nothing if 'inline' is not supported under any name.  */
#ifndef __cplusplus
/* #undef inline */
#endif

/* Define to `int' if <sys/types.h> does not define. */
/* #undef pid_t */

/* Define to the equivalent of the C99 'restrict' keyword */
#define restrict __restrict

/* Define to `unsigned int' if <sys/types.h> does not define. */
/* #undef size_t */

#endif /* CONFIG_H */
")

# Add the generated config.h directory to include paths
target_include_directories(${COMPONENT_LIB} PRIVATE "${CMAKE_CURRENT_BINARY_DIR}")