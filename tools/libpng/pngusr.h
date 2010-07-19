/* minwrpngconf.h: headers to make a minimal png-write-only library
 *
 * Copyright (c) 2007, 2009 Glenn Randers-Pehrson
 *
 * This code is released under the libpng license.
 * For conditions of distribution and use, see the disclaimer
 * and license in png.h
 *
 * Derived from pngcrush.h, Copyright 1998-2007, Glenn Randers-Pehrson
 */

#ifndef MINWRPNGCONF_H
#define MINWRPNGCONF_H

#define PNG_NO_READ_SUPPORTED

#define PNG_NO_WARNINGS
#define png_warning(s1,s2)
#define png_chunk_warning(s1,s2)
#define PNG_NO_ERROR_TEXT
#define png_error(s1,s2) png_err(s1)
#define png_chunk_error(s1,s2) png_err(s1)
#ifdef _WIN32_WCE
#define PNG_ABORT()
#endif

#define PNG_NO_WRITE_BACKGROUND
#define PNG_NO_WRITE_BGR
#define PNG_NO_WRITE_GAMMA
#define PNG_NO_WRITE_QUANTIZE
#define PNG_NO_WRITE_INVERT
#define PNG_NO_WRITE_SHIFT
#define PNG_NO_WRITE_PACK
#define PNG_NO_WRITE_PACKSWAP
#define PNG_NO_WRITE_FILLER
#define PNG_NO_WRITE_SWAP
#define PNG_NO_WRITE_SWAP_ALPHA
#define PNG_NO_WRITE_INVERT_ALPHA
#define PNG_NO_WRITE_RGB_TO_GRAY
#define PNG_NO_WRITE_USER_TRANSFORM
#define PNG_NO_WRITE_bKGD
#define PNG_NO_WRITE_cHRM
#define PNG_NO_WRITE_gAMA
#define PNG_NO_WRITE_hIST
#define PNG_NO_WRITE_iCCP
#define PNG_NO_WRITE_oFFs
#define PNG_NO_WRITE_pCAL
#define PNG_NO_WRITE_pHYs
#define PNG_NO_WRITE_sBIT
#define PNG_NO_WRITE_sCAL
#define PNG_NO_WRITE_sPLT
#define PNG_NO_WRITE_sRGB
#define PNG_NO_WRITE_TEXT
#define PNG_NO_WRITE_tIME
#define PNG_NO_WRITE_UNKNOWN_CHUNKS
#define PNG_NO_WRITE_USER_CHUNKS
#define PNG_NO_WRITE_EMPTY_PLTE
#define PNG_NO_WRITE_OPT_PLTE
#define PNG_NO_WRITE_FILTER
#define PNG_NO_WRITE_WEIGHTED_FILTER
#define PNG_NO_WRITE_INTERLACING_SUPPORTED
#define PNG_NO_WRITE_FLUSH

#define PNG_NO_INFO_IMAGE
#define PNG_NO_IO_STATE
#define PNG_NO_USER_MEM
#define PNG_NO_FIXED_POINT_SUPPORTED
#define PNG_NO_MNG_FEATURES
#define PNG_NO_USER_TRANSFORM_PTR
#define PNG_NO_HANDLE_AS_UNKNOWN
#define PNG_NO_CONSOLE_IO
#define PNG_NO_ZALLOC_ZERO
#define PNG_NO_ERROR_NUMBERS
#define PNG_NO_EASY_ACCESS
#define PNG_NO_USER_LIMITS
#define PNG_NO_SET_USER_LIMITS
#define PNG_NO_TIME_RFC1143

#define png_set_error_fn my_png_set_error_fn
#define png_get_error_ptr my_png_get_error_ptr
#define png_access_version_number my_png_access_version_number
#define png_set_sig_bytes my_png_set_sig_bytes
#define png_sig_cmp my_png_sig_cmp
#define png_create_read_struct my_png_create_read_struct
#define png_create_write_struct my_png_create_write_struct
#define png_get_compression_buffer_size my_png_get_compression_buffer_size
#define png_set_compression_buffer_size my_png_set_compression_buffer_size
#define png_set_longjmp_fn my_png_set_longjmp_fn
#define png_reset_zstream my_png_reset_zstream
#define png_create_read_struct_2 my_png_create_read_struct_2
#define png_create_write_struct_2 my_png_create_write_struct_2
#define png_write_sig my_png_write_sig
#define png_write_chunk my_png_write_chunk
#define png_write_chunk_start my_png_write_chunk_start
#define png_write_chunk_data my_png_write_chunk_data
#define png_write_chunk_end my_png_write_chunk_end
#define png_create_info_struct my_png_create_info_struct
#define png_info_init_3 my_png_info_init_3
#define png_write_info_before_PLTE my_png_write_info_before_PLTE
#define png_write_info my_png_write_info
#define png_read_info my_png_read_info
#define png_convert_to_rfc1123 my_png_convert_to_rfc1123
#define png_convert_from_struct_tm my_png_convert_from_struct_tm
#define png_convert_from_time_t my_png_convert_from_time_t
#define png_set_expand my_png_set_expand
#define png_set_expand_gray_1_2_4_to_8 my_png_set_expand_gray_1_2_4_to_8
#define png_set_palette_to_rgb my_png_set_palette_to_rgb
#define png_set_tRNS_to_alpha my_png_set_tRNS_to_alpha
#define png_set_bgr my_png_set_bgr
#define png_set_gray_to_rgb my_png_set_gray_to_rgb
#define png_set_rgb_to_gray my_png_set_rgb_to_gray
#define png_set_rgb_to_gray_fixed my_png_set_rgb_to_gray_fixed
#define png_get_rgb_to_gray_status my_png_get_rgb_to_gray_status
#define png_build_grayscale_palette my_png_build_grayscale_palette
#define png_set_strip_alpha my_png_set_strip_alpha
#define png_set_swap_alpha my_png_set_swap_alpha
#define png_set_invert_alpha my_png_set_invert_alpha
#define png_set_filler my_png_set_filler
#define png_set_add_alpha my_png_set_add_alpha
#define png_set_swap my_png_set_swap
#define png_set_packing my_png_set_packing
#define png_set_packswap my_png_set_packswap
#define png_set_shift my_png_set_shift
#define png_set_interlace_handling my_png_set_interlace_handling
#define png_set_invert_mono my_png_set_invert_mono
#define png_set_background my_png_set_background
#define png_set_strip_16 my_png_set_strip_16
#define png_set_quantize my_png_set_quantize
#define png_set_gamma my_png_set_gamma
#define png_set_flush my_png_set_flush
#define png_write_flush my_png_write_flush
#define png_start_read_image my_png_start_read_image
#define png_read_update_info my_png_read_update_info
#define png_read_rows my_png_read_rows
#define png_read_row my_png_read_row
#define png_read_image my_png_read_image
#define png_write_row my_png_write_row
#define png_write_rows my_png_write_rows
#define png_write_image my_png_write_image
#define png_write_end my_png_write_end
#define png_read_end my_png_read_end
#define png_destroy_info_struct my_png_destroy_info_struct
#define png_destroy_read_struct my_png_destroy_read_struct
#define png_destroy_write_struct my_png_destroy_write_struct
#define png_set_crc_action my_png_set_crc_action
#define png_set_filter my_png_set_filter
#define png_set_filter_heuristics my_png_set_filter_heuristics
#define png_set_compression_level my_png_set_compression_level
#define png_set_compression_mem_level my_png_set_compression_mem_level
#define png_set_compression_strategy my_png_set_compression_strategy
#define png_set_compression_window_bits my_png_set_compression_window_bits
#define png_set_compression_method my_png_set_compression_method
#define png_init_io my_png_init_io
#define png_set_error_fn my_png_set_error_fn
#define png_get_error_ptr my_png_get_error_ptr
#define png_set_write_fn my_png_set_write_fn
#define png_set_read_fn my_png_set_read_fn
#define png_get_io_ptr my_png_get_io_ptr
#define png_set_read_status_fn my_png_set_read_status_fn
#define png_set_write_status_fn my_png_set_write_status_fn
#define png_set_mem_fn my_png_set_mem_fn
#define png_get_mem_ptr my_png_get_mem_ptr
#define png_set_read_user_transform_fn my_png_set_read_user_transform_fn
#define png_set_write_user_transform_fn my_png_set_write_user_transform_fn
#define png_set_user_transform_info my_png_set_user_transform_info
#define png_get_user_transform_ptr my_png_get_user_transform_ptr
#define png_set_read_user_chunk_fn my_png_set_read_user_chunk_fn
#define png_get_user_chunk_ptr my_png_get_user_chunk_ptr
#define png_set_progressive_read_fn my_png_set_progressive_read_fn
#define png_get_progressive_ptr my_png_get_progressive_ptr
#define png_process_data my_png_process_data
#define png_progressive_combine_row my_png_progressive_combine_row
#define png_malloc my_png_malloc
#define png_calloc my_png_calloc
#define png_malloc_warn my_png_malloc_warn
#define png_free my_png_free
#define png_free_data my_png_free_data
#define png_data_freer my_png_data_freer
#define png_malloc_default my_png_malloc_default
#define png_free_default my_png_free_default
//#define png_error my_png_error
//#define png_chunk_error my_png_chunk_error
#define png_err my_png_err
//#define png_warning my_png_warning
//#define png_chunk_warning my_png_chunk_warning
//#define png_benign_error my_png_benign_error
//#define png_chunk_benign_error my_png_chunk_benign_error
#define png_set_benign_errors my_png_set_benign_errors
#define png_get_valid my_png_get_valid
#define png_get_rowbytes my_png_get_rowbytes
#define png_get_rows my_png_get_rows
#define png_set_rows my_png_set_rows
#define png_get_channels my_png_get_channels
#define png_get_image_width my_png_get_image_width
#define png_get_image_height my_png_get_image_height
#define png_get_bit_depth my_png_get_bit_depth
#define png_get_color_type my_png_get_color_type
#define png_get_filter_type my_png_get_filter_type
#define png_get_interlace_type my_png_get_interlace_type
#define png_get_compression_type my_png_get_compression_type
#define png_get_pixels_per_meter my_png_get_pixels_per_meter
#define png_get_x_pixels_per_meter my_png_get_x_pixels_per_meter
#define png_get_y_pixels_per_meter my_png_get_y_pixels_per_meter
#define png_get_pixel_aspect_ratio my_png_get_pixel_aspect_ratio
#define png_get_x_offset_pixels my_png_get_x_offset_pixels
#define png_get_y_offset_pixels my_png_get_y_offset_pixels
#define png_get_x_offset_microns my_png_get_x_offset_microns
#define png_get_y_offset_microns my_png_get_y_offset_microns
#define png_get_signature my_png_get_signature
#define png_get_bKGD my_png_get_bKGD
#define png_set_bKGD my_png_set_bKGD
#define png_get_cHRM my_png_get_cHRM
#define png_get_cHRM_fixed my_png_get_cHRM_fixed
#define png_set_cHRM my_png_set_cHRM
#define png_set_cHRM_fixed my_png_set_cHRM_fixed
#define png_get_gAMA my_png_get_gAMA
#define png_get_gAMA_fixed my_png_get_gAMA_fixed
#define png_set_gAMA my_png_set_gAMA
#define png_set_gAMA_fixed my_png_set_gAMA_fixed
#define png_get_hIST my_png_get_hIST
#define png_set_hIST my_png_set_hIST
#define png_get_IHDR my_png_get_IHDR
#define png_set_IHDR my_png_set_IHDR
#define png_get_oFFs my_png_get_oFFs
#define png_set_oFFs my_png_set_oFFs
#define png_get_pCAL my_png_get_pCAL
#define png_set_pCAL my_png_set_pCAL
#define png_get_pHYs my_png_get_pHYs
#define png_set_pHYs my_png_set_pHYs
#define png_get_PLTE my_png_get_PLTE
#define png_set_PLTE my_png_set_PLTE
#define png_get_sBIT my_png_get_sBIT
#define png_set_sBIT my_png_set_sBIT
#define png_get_sRGB my_png_get_sRGB
#define png_set_sRGB my_png_set_sRGB
#define png_set_sRGB_gAMA_and_cHRM my_png_set_sRGB_gAMA_and_cHRM
#define png_get_iCCP my_png_get_iCCP
#define png_set_iCCP my_png_set_iCCP
#define png_get_sPLT my_png_get_sPLT
#define png_set_sPLT my_png_set_sPLT
#define png_get_text my_png_get_text
#define png_set_text my_png_set_text
#define png_get_tIME my_png_get_tIME
#define png_set_tIME my_png_set_tIME
#define png_get_tRNS my_png_get_tRNS
#define png_set_tRNS my_png_set_tRNS
#define png_get_sCAL my_png_get_sCAL
#define png_get_sCAL_s my_png_get_sCAL_s
#define png_set_sCAL my_png_set_sCAL
#define png_set_sCAL_s my_png_set_sCAL_s
#define png_set_keep_unknown_chunks my_png_set_keep_unknown_chunks
#define png_handle_as_unknown my_png_handle_as_unknown
#define png_set_unknown_chunks my_png_set_unknown_chunks
#define png_set_unknown_chunk_location my_png_set_unknown_chunk_location
#define png_get_unknown_chunks my_png_get_unknown_chunks
#define png_set_invalid my_png_set_invalid
#define png_read_png my_png_read_png
#define png_write_png my_png_write_png
#define png_get_copyright my_png_get_copyright
#define png_get_header_ver my_png_get_header_ver
#define png_get_header_version my_png_get_header_version
#define png_get_libpng_ver my_png_get_libpng_ver
#define png_permit_mng_features my_png_permit_mng_features
#define png_set_strip_error_numbers my_png_set_strip_error_numbers
#define png_set_user_limits my_png_set_user_limits
#define png_get_user_width_max my_png_get_user_width_max
#define png_get_user_height_max my_png_get_user_height_max
#define png_set_chunk_cache_max my_png_set_chunk_cache_max
#define png_get_chunk_cache_max my_png_get_chunk_cache_max
#define png_set_chunk_malloc_max my_png_set_chunk_malloc_max
#define png_get_chunk_malloc_max my_png_get_chunk_malloc_max
#define png_get_pixels_per_inch my_png_get_pixels_per_inch
#define png_get_x_pixels_per_inch my_png_get_x_pixels_per_inch
#define png_get_y_pixels_per_inch my_png_get_y_pixels_per_inch
#define png_get_x_offset_inches my_png_get_x_offset_inches
#define png_get_y_offset_inches my_png_get_y_offset_inches
#define png_get_pHYs_dpi my_png_get_pHYs_dpi
#define png_get_io_state my_png_get_io_state
#define png_get_io_chunk_name my_png_get_io_chunk_name
//#define png_get_uint_32 my_png_get_uint_32
//#define png_get_uint_16 my_png_get_uint_16
#define png_get_int_32 my_png_get_int_32
#define png_get_uint_31 my_png_get_uint_31
#define png_save_uint_32 my_png_save_uint_32
#define png_save_int_32 my_png_save_int_32
#define png_save_uint_16 my_png_save_uint_16

#endif /* MINWRPNGCONF_H */
