#ifndef GL_EXTGL_H
#define GL_EXTGL_H

#include "GL/glew.h"

#ifdef __cplusplus
extern "C" {
#endif

/* OpenGL extension handling structure */
struct ExtensionTypes {
    /* OpenGL versions */
    int OpenGL12;
    int OpenGL13;
    int OpenGL14;
    
    /* ARB extensions */
    int ARB_depth_texture;
    int ARB_imaging;
    int ARB_matrix_palette;
    int ARB_multisample;
    int ARB_multitexture;
    int ARB_point_parameters;
    int ARB_shadow;
    int ARB_shadow_ambient;
    int ARB_texture_border_clamp;
    int ARB_texture_compression;
    int ARB_texture_cube_map;
    int ARB_texture_env_add;
    int ARB_texture_env_combine;
    int ARB_texture_env_crossbar;
    int ARB_texture_env_dot3;
    int ARB_texture_mirrored_repeat;
    int ARB_transpose_matrix;
    int ARB_vertex_blend;
    int ARB_vertex_program;
    int ARB_window_pos;
    
    /* EXT extensions */
    int EXT_abgr;
    int EXT_bgra;
    int EXT_blend_func_separate;
    int EXT_compiled_vertex_array;
    int EXT_cull_vertex;
    int EXT_draw_range_elements;
    int EXT_fog_coord;
    int EXT_multi_draw_arrays;
    int EXT_point_parameters;
    int EXT_secondary_color;
    int EXT_separate_specular_color;
    int EXT_shadow_funcs;
    int EXT_stencil_two_side;
    int EXT_stencil_wrap;
    int EXT_texture_compression_s3tc;
    int EXT_texture_filter_anisotropic;
    int EXT_texture_lod_bias;
    int EXT_vertex_shader;
    int EXT_vertex_weighting;
    
    /* ATI extensions */
    int ATI_element_array;
    int ATI_envmap_bumpmap;
    int ATI_fragment_shader;
    int ATI_pn_triangles;
    int ATI_point_cull_mode;
    int ATI_texture_mirror_once;
    int ATI_vertex_array_object;
    int ATI_vertex_streams;
    
    /* ATIX extensions */
    int ATIX_point_sprites;
    int ATIX_texture_env_route;
    
    /* HP extensions */
    int HP_occlusion_test;
    
    /* NV extensions */
    int NV_blend_square;
    int NV_copy_depth_to_color;
    int NV_depth_clamp;
    int NV_element_array;
    int NV_evaluators;
    int NV_fence;
    int NV_float_buffer;
    int NV_fog_distance;
    int NV_fragment_program;
    int NV_light_max_exponent;
    int NV_occlusion_query;
    int NV_packed_depth_stencil;
    int NV_point_sprite;
    int NV_primitive_restart;
    int NV_register_combiners;
    int NV_register_combiners2;
    int NV_texgen_reflection;
    int NV_texture_env_combine4;
    int NV_texture_rectangle;
    int NV_texture_shader;
    int NV_texture_shader2;
    int NV_texture_shader3;
    int NV_vertex_array_range;
    int NV_vertex_array_range2;
    int NV_vertex_program;
    int NV_vertex_program1_1;
    int NV_vertex_program2;
    
    /* SGIS/SGIX extensions */
    int SGIS_generate_mipmap;
    int SGIX_depth_texture;
    int SGIX_shadow;
    
#ifdef _WIN32
    /* WGL extensions */
    struct {
        int ARB_buffer_region;
        int ARB_extensions_string;
        int ARB_make_current_read;
        int ARB_multisample;
        int ARB_pbuffer;
        int ARB_pixel_format;
        int ARB_render_texture;
        int EXT_extensions_string;
        int EXT_swap_control;
        int NV_render_depth_texture;
        int NV_render_texture_rectangle;
    } wgl;
#endif
};

/* Global extension support flags */
extern struct ExtensionTypes extgl_Extensions;
extern struct ExtensionTypes SupportedExtensions; /* deprecated */

/* Initialize OpenGL extensions */
int extgl_Initialize(void);

#ifdef __cplusplus
}
#endif

#endif /* GL_EXTGL_H */