#include <GL/extgl.h>
#include <stdio.h>
#include <string.h>

#ifdef __APPLE__
#include "SDL.h"
#endif

/* Global state */
struct ExtensionTypes extgl_Extensions;
struct ExtensionTypes SupportedExtensions; /* deprecated, please do not use */

/**
 * Initialize extension support flags based on GLEW
 */
void extgl_InitSupportedExtensions(void) {
    /* Initialize all extensions to 0 first */
    memset(&extgl_Extensions, 0, sizeof(struct ExtensionTypes));
    
    /* OpenGL versions */
    extgl_Extensions.OpenGL12 = GLEW_VERSION_1_2;
    extgl_Extensions.OpenGL13 = GLEW_VERSION_1_3;
    extgl_Extensions.OpenGL14 = GLEW_VERSION_1_4;
    
    /* ARB extensions */
    extgl_Extensions.ARB_depth_texture = GLEW_ARB_depth_texture;
    extgl_Extensions.ARB_imaging = GLEW_ARB_imaging;
    extgl_Extensions.ARB_matrix_palette = GLEW_ARB_matrix_palette;
    extgl_Extensions.ARB_multisample = GLEW_ARB_multisample;
    extgl_Extensions.ARB_multitexture = GLEW_ARB_multitexture;
    extgl_Extensions.ARB_point_parameters = GLEW_ARB_point_parameters;
    extgl_Extensions.ARB_shadow = GLEW_ARB_shadow;
    extgl_Extensions.ARB_shadow_ambient = GLEW_ARB_shadow_ambient;
    extgl_Extensions.ARB_texture_border_clamp = GLEW_ARB_texture_border_clamp;
    extgl_Extensions.ARB_texture_compression = GLEW_ARB_texture_compression;
    extgl_Extensions.ARB_texture_cube_map = GLEW_ARB_texture_cube_map;
    extgl_Extensions.ARB_texture_env_add = GLEW_ARB_texture_env_add;
    extgl_Extensions.ARB_texture_env_combine = GLEW_ARB_texture_env_combine;
    extgl_Extensions.ARB_texture_env_crossbar = GLEW_ARB_texture_env_crossbar;
    extgl_Extensions.ARB_texture_env_dot3 = GLEW_ARB_texture_env_dot3;
    extgl_Extensions.ARB_texture_mirrored_repeat = GLEW_ARB_texture_mirrored_repeat;
    extgl_Extensions.ARB_transpose_matrix = GLEW_ARB_transpose_matrix;
    extgl_Extensions.ARB_vertex_blend = GLEW_ARB_vertex_blend;
    extgl_Extensions.ARB_vertex_program = GLEW_ARB_vertex_program;
    extgl_Extensions.ARB_window_pos = GLEW_ARB_window_pos;
    
    /* EXT extensions */
    extgl_Extensions.EXT_abgr = GLEW_EXT_abgr;
    extgl_Extensions.EXT_bgra = GLEW_EXT_bgra;
    extgl_Extensions.EXT_blend_func_separate = GLEW_EXT_blend_func_separate;
    extgl_Extensions.EXT_compiled_vertex_array = GLEW_EXT_compiled_vertex_array;
    extgl_Extensions.EXT_cull_vertex = GLEW_EXT_cull_vertex;
    extgl_Extensions.EXT_draw_range_elements = GLEW_EXT_draw_range_elements;
    extgl_Extensions.EXT_fog_coord = GLEW_EXT_fog_coord;
    extgl_Extensions.EXT_multi_draw_arrays = GLEW_EXT_multi_draw_arrays;
    extgl_Extensions.EXT_point_parameters = GLEW_EXT_point_parameters;
    extgl_Extensions.EXT_secondary_color = GLEW_EXT_secondary_color;
    extgl_Extensions.EXT_separate_specular_color = GLEW_EXT_separate_specular_color;
    extgl_Extensions.EXT_shadow_funcs = GLEW_EXT_shadow_funcs;
    extgl_Extensions.EXT_stencil_two_side = GLEW_EXT_stencil_two_side;
    extgl_Extensions.EXT_stencil_wrap = GLEW_EXT_stencil_wrap;
    extgl_Extensions.EXT_texture_compression_s3tc = GLEW_EXT_texture_compression_s3tc;
    extgl_Extensions.EXT_texture_filter_anisotropic = GLEW_EXT_texture_filter_anisotropic;
    extgl_Extensions.EXT_texture_lod_bias = GLEW_EXT_texture_lod_bias;
    extgl_Extensions.EXT_vertex_shader = GLEW_EXT_vertex_shader;
    extgl_Extensions.EXT_vertex_weighting = GLEW_EXT_vertex_weighting;
    
    /* ATI extensions */
    extgl_Extensions.ATI_element_array = GLEW_ATI_element_array;
    extgl_Extensions.ATI_envmap_bumpmap = GLEW_ATI_envmap_bumpmap;
    extgl_Extensions.ATI_fragment_shader = GLEW_ATI_fragment_shader;
    extgl_Extensions.ATI_pn_triangles = GLEW_ATI_pn_triangles;
    
    /* ATI_point_cull_mode may not be defined in all GLEW versions */
#ifdef GLEW_ATI_point_cull_mode
    extgl_Extensions.ATI_point_cull_mode = GLEW_ATI_point_cull_mode;
#endif
    
    extgl_Extensions.ATI_texture_mirror_once = GLEW_ATI_texture_mirror_once;
    extgl_Extensions.ATI_vertex_array_object = GLEW_ATI_vertex_array_object;
    extgl_Extensions.ATI_vertex_streams = GLEW_ATI_vertex_streams;
    
    /* ATIX extensions */
#ifdef GLEW_ATIX_point_sprites
    extgl_Extensions.ATIX_point_sprites = GLEW_ATIX_point_sprites;
#endif
#ifdef GLEW_ATIX_texture_env_route
    extgl_Extensions.ATIX_texture_env_route = GLEW_ATIX_texture_env_route;
#endif
    
    /* HP extensions */
    extgl_Extensions.HP_occlusion_test = GLEW_HP_occlusion_test;
    
    /* NV extensions */
    extgl_Extensions.NV_blend_square = GLEW_NV_blend_square;
    extgl_Extensions.NV_copy_depth_to_color = GLEW_NV_copy_depth_to_color;
    extgl_Extensions.NV_depth_clamp = GLEW_NV_depth_clamp;
    
    /* NV_element_array may not be defined in all GLEW versions */
#ifdef GLEW_NV_element_array
    extgl_Extensions.NV_element_array = GLEW_NV_element_array;
#endif
    
    extgl_Extensions.NV_evaluators = GLEW_NV_evaluators;
    extgl_Extensions.NV_fence = GLEW_NV_fence;
    extgl_Extensions.NV_float_buffer = GLEW_NV_float_buffer;
    extgl_Extensions.NV_fog_distance = GLEW_NV_fog_distance;
    extgl_Extensions.NV_fragment_program = GLEW_NV_fragment_program;
    extgl_Extensions.NV_light_max_exponent = GLEW_NV_light_max_exponent;
    extgl_Extensions.NV_occlusion_query = GLEW_NV_occlusion_query;
    extgl_Extensions.NV_packed_depth_stencil = GLEW_NV_packed_depth_stencil;
    extgl_Extensions.NV_point_sprite = GLEW_NV_point_sprite;
    extgl_Extensions.NV_primitive_restart = GLEW_NV_primitive_restart;
    extgl_Extensions.NV_register_combiners = GLEW_NV_register_combiners;
    extgl_Extensions.NV_register_combiners2 = GLEW_NV_register_combiners2;
    extgl_Extensions.NV_texgen_reflection = GLEW_NV_texgen_reflection;
    extgl_Extensions.NV_texture_env_combine4 = GLEW_NV_texture_env_combine4;
    extgl_Extensions.NV_texture_rectangle = GLEW_NV_texture_rectangle;
    extgl_Extensions.NV_texture_shader = GLEW_NV_texture_shader;
    extgl_Extensions.NV_texture_shader2 = GLEW_NV_texture_shader2;
    extgl_Extensions.NV_texture_shader3 = GLEW_NV_texture_shader3;
    extgl_Extensions.NV_vertex_array_range = GLEW_NV_vertex_array_range;
    extgl_Extensions.NV_vertex_array_range2 = GLEW_NV_vertex_array_range2;
    extgl_Extensions.NV_vertex_program = GLEW_NV_vertex_program;
    extgl_Extensions.NV_vertex_program1_1 = GLEW_NV_vertex_program1_1;
    extgl_Extensions.NV_vertex_program2 = GLEW_NV_vertex_program2;
    
    /* SGIS/SGIX extensions */
    extgl_Extensions.SGIS_generate_mipmap = GLEW_SGIS_generate_mipmap;
    extgl_Extensions.SGIX_depth_texture = GLEW_SGIX_depth_texture;
    extgl_Extensions.SGIX_shadow = GLEW_SGIX_shadow;
    
#ifdef _WIN32
    /* WGL extensions */
    /* Initialize WGL extensions to 0 first */
    memset(&extgl_Extensions.wgl, 0, sizeof(extgl_Extensions.wgl));
    
#ifdef WGLEW_ARB_extensions_string
    extgl_Extensions.wgl.ARB_extensions_string = WGLEW_ARB_extensions_string;
#endif
#ifdef WGLEW_EXT_extensions_string
    extgl_Extensions.wgl.EXT_extensions_string = WGLEW_EXT_extensions_string;
#endif
#ifdef WGLEW_ARB_buffer_region
    extgl_Extensions.wgl.ARB_buffer_region = WGLEW_ARB_buffer_region;
#endif
#ifdef WGLEW_ARB_make_current_read
    extgl_Extensions.wgl.ARB_make_current_read = WGLEW_ARB_make_current_read;
#endif
#ifdef WGLEW_ARB_multisample
    extgl_Extensions.wgl.ARB_multisample = WGLEW_ARB_multisample;
#endif
#ifdef WGLEW_ARB_pbuffer
    extgl_Extensions.wgl.ARB_pbuffer = WGLEW_ARB_pbuffer;
#endif
#ifdef WGLEW_ARB_pixel_format
    extgl_Extensions.wgl.ARB_pixel_format = WGLEW_ARB_pixel_format;
#endif
#ifdef WGLEW_ARB_render_texture
    extgl_Extensions.wgl.ARB_render_texture = WGLEW_ARB_render_texture;
#endif
#ifdef WGLEW_EXT_swap_control
    extgl_Extensions.wgl.EXT_swap_control = WGLEW_EXT_swap_control;
#endif
#ifdef WGLEW_NV_render_depth_texture
    extgl_Extensions.wgl.NV_render_depth_texture = WGLEW_NV_render_depth_texture;
#endif
#ifdef WGLEW_NV_render_texture_rectangle
    extgl_Extensions.wgl.NV_render_texture_rectangle = WGLEW_NV_render_texture_rectangle;
#endif
#endif /* _WIN32 */

    /* Copy to deprecated structure for backward compatibility */
    memcpy(&SupportedExtensions, &extgl_Extensions, sizeof(struct ExtensionTypes));
}

/**
 * Initialize OpenGL extensions using GLEW
 */
int extgl_Initialize(void) {
    GLenum err;
    
    /* Initialize GLEW */
    err = glewInit();
    if (err != GLEW_OK) {
        fprintf(stderr, "GLEW initialization error: %s\n", glewGetErrorString(err));
        return 0;
    }
    
    /* Initialize extension support flags */
    extgl_InitSupportedExtensions();
    
    return 1;
}