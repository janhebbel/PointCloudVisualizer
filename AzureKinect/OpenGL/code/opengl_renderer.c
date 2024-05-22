#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h>

#include "opengl_renderer.h"
#include "linalg.h"

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;

typedef void (APIENTRY *GLDEBUGPROC)(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam);

typedef GLuint type_glCreateShader(GLenum shaderType);
typedef void   type_glShaderSource(GLuint shader, GLsizei count, const GLchar **string, const GLint *length);
typedef void   type_glCompileShader(GLuint shader);
typedef void   type_glGetShaderiv(GLuint shader, GLenum pname, GLint *params);
typedef GLuint type_glCreateProgram(void);
typedef void   type_glAttachShader(GLuint program, GLuint shader);
typedef void   type_glLinkProgram(GLuint program);
typedef void   type_glGetProgramiv(GLuint program, GLenum pname, GLint *params);
typedef void   type_glDetachShader(GLuint program, GLuint shader);
typedef void   type_glDeleteShader(GLuint shader);
typedef void   type_glUseProgram(GLuint program);
typedef void   type_glGenBuffers(GLsizei n, GLuint * buffers);
typedef void   type_glBufferData(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void   type_glNamedBufferData(GLuint buffer, GLsizeiptr size, const void *data, GLenum usage);
typedef void   type_glGenVertexArrays(GLsizei n, GLuint *arrays);
typedef void   type_glBindVertexArray(GLuint array);
typedef void   type_glBindBuffer(GLenum target, GLuint buffer);
typedef void   type_glVertexAttribPointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void * pointer);
typedef void   type_glEnableVertexAttribArray(GLuint index);
typedef GLint  type_glGetUniformLocation(GLuint program, const GLchar *name);
typedef void   type_glUniformMatrix4fv(GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void   type_glProgramUniformMatrix4fv(GLuint program, GLint location, GLsizei count, GLboolean transpose, const GLfloat *value);
typedef void   type_glGetProgramInfoLog(GLuint program, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void   type_glGetShaderInfoLog(GLuint shader, GLsizei maxLength, GLsizei *length, GLchar *infoLog);
typedef void   type_glGenSamplers(GLsizei n, GLuint *samplers);
typedef void   type_glDeleteSamplers(GLsizei n, const GLuint * samplers);
typedef void   type_glBindSampler(GLuint unit, GLuint sampler);
typedef void   type_glSamplerParameteri(GLuint sampler, GLenum pname, GLint param);
typedef void   type_glActiveTexture(GLenum texture);
typedef void   type_glProgramUniform1i(GLuint program, GLint location, GLint v0);
typedef void   type_glDebugMessageCallback(GLDEBUGPROC Callback, const void *UserParam);
typedef void   type_glValidateProgram(GLuint program);
typedef void   type_glDrawBuffers(GLsizei n, const GLenum *bufs);
typedef void   type_glBindImageTexture(GLuint unit, GLuint texture, GLint level, GLboolean layered, GLint layer, GLenum access, GLenum format);
typedef void   type_glDispatchCompute(GLuint num_groups_x, GLuint num_groups_y, GLuint num_groups_z);
typedef void   type_glMemoryBarrier(GLbitfield barriers);
typedef void   type_glUniform1i(GLint location, GLint v0);
typedef void   type_glUniform1f(GLint location, GLfloat v0);
typedef void   type_glTexStorage2D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height);

#define GL_DEBUG_SEVERITY_HIGH                  0x9146
#define GL_DEBUG_SEVERITY_MEDIUM                0x9147
#define GL_DEBUG_SEVERITY_LOW                   0x9148
#define GL_DEBUG_OUTPUT                         0x92E0
#define GL_DEBUG_OUTPUT_SYNCHRONOUS             0x8242
#define GL_FRAGMENT_SHADER                      0x8B30
#define GL_VERTEX_SHADER                        0x8B31
#define GL_COMPUTE_SHADER                       0x91B9
#define GL_COMPILE_STATUS                       0x8B81
#define GL_LINK_STATUS                          0x8B82
#define GL_ARRAY_BUFFER                         0x8892
#define GL_ELEMENT_ARRAY_BUFFER                 0x8893
#define GL_STATIC_DRAW                          0x88E4
#define GL_PROGRAM_POINT_SIZE                   0x8642
#define GL_TEXTURE0                             0x84C0
#define GL_TEXTURE1                             0x84C1
#define GL_TEXTURE2                             0x84C2
#define GL_TEXTURE3                             0x84C3
#define GL_TEXTURE4                             0x84C4
#define GL_TEXTURE5                             0x84C5
#define GL_TEXTURE6                             0x84C6
#define GL_TEXTURE7                             0x84C7
#define GL_TEXTURE8                             0x84C8
#define GL_TEXTURE9                             0x84C9
#define GL_TEXTURE10                            0x84CA
#define GL_TEXTURE11                            0x84CB
#define GL_TEXTURE12                            0x84CC
#define GL_TEXTURE13                            0x84CD
#define GL_TEXTURE14                            0x84CE
#define GL_TEXTURE15                            0x84CF
#define GL_TEXTURE16                            0x84D0
#define GL_TEXTURE17                            0x84D1
#define GL_TEXTURE18                            0x84D2
#define GL_TEXTURE19                            0x84D3
#define GL_TEXTURE20                            0x84D4
#define GL_TEXTURE21                            0x84D5
#define GL_TEXTURE22                            0x84D6
#define GL_TEXTURE23                            0x84D7
#define GL_TEXTURE24                            0x84D8
#define GL_TEXTURE25                            0x84D9
#define GL_TEXTURE26                            0x84DA
#define GL_TEXTURE27                            0x84DB
#define GL_TEXTURE28                            0x84DC
#define GL_TEXTURE29                            0x84DD
#define GL_TEXTURE30                            0x84DE
#define GL_TEXTURE31                            0x84DF
#define GL_CLAMP_TO_EDGE                        0x812F
#define GL_RGBA32F                              0x8814
#define GL_RG32F                                0x8230
#define GL_R16UI                                0x8234
#define GL_RG                                   0x8227
#define GL_RED_INTEGER                          0x8D94
#define GL_READ_ONLY                            0x88B8
#define GL_WRITE_ONLY                           0x88B9
#define GL_READ_WRITE                           0x88BA
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT      0x00000020
#define GL_TEXTURE_FETCH_BARRIER_BIT            0x00000008

typedef struct
{
    uint32_t w;
    uint32_t h;
} dimensions;

#define opengl_function(name) type_##name *name

typedef struct
{
    GLuint default_program;
    GLuint compute_program;
    
    GLuint depth_map_texture;
    GLuint xy_table_texture;
    GLuint xyzw_table_texture;
    GLuint rgba_color_texture;
    
    dimensions depth_image_dimensions;
    
    opengl_function(glDebugMessageCallback);
    opengl_function(glCreateShader);
    opengl_function(glShaderSource);
    opengl_function(glCompileShader);
    opengl_function(glGetShaderiv);
    opengl_function(glCreateProgram);
    opengl_function(glAttachShader);
    opengl_function(glLinkProgram);
    opengl_function(glGetProgramiv);
    opengl_function(glDetachShader);
    opengl_function(glDeleteShader);
    opengl_function(glUseProgram);
    opengl_function(glGenBuffers);
    opengl_function(glBufferData);
    opengl_function(glNamedBufferData);
    opengl_function(glGenVertexArrays);
    opengl_function(glBindVertexArray);
    opengl_function(glBindBuffer);
    opengl_function(glVertexAttribPointer);
    opengl_function(glEnableVertexAttribArray);
    opengl_function(glGetUniformLocation);
    opengl_function(glUniformMatrix4fv);
    opengl_function(glProgramUniformMatrix4fv);
    opengl_function(glGetProgramInfoLog);
    opengl_function(glGetShaderInfoLog);
    opengl_function(glGenSamplers);
    opengl_function(glDeleteSamplers);
    opengl_function(glBindSampler);
    opengl_function(glSamplerParameteri);
    opengl_function(glActiveTexture);
    opengl_function(glProgramUniform1i);
    opengl_function(glValidateProgram);
    opengl_function(glDrawBuffers);
    opengl_function(glBindImageTexture);
    opengl_function(glDispatchCompute);
    opengl_function(glMemoryBarrier);
    opengl_function(glUniform1i);
    opengl_function(glUniform1f);
    opengl_function(glTexStorage2D);

} open_gl;

void APIENTRY 
opengl_debug_message_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar *message, const void *userParam)
{
    if(severity == GL_DEBUG_SEVERITY_HIGH)
    {
        printf("OPENGL: %s\n", message);
        
        assert(0 && "OpenGL Error occurred.\n");
    }
}

#define GLSL(Code) "#version 430 core\n" #Code

static void compile_default_program(open_gl *opengl)
{
    GLuint vertex_shader = opengl->glCreateShader(GL_VERTEX_SHADER);
    char *vertex_code = GLSL(layout(binding = 0, rgba32f) readonly uniform image2D xyzw_tex;
                             layout(binding = 1, rgba32f) readonly uniform image2D rgba_tex;
                             
                             layout(location = 0) uniform mat4 mvp;
                             layout(location = 1) uniform float point_size;
                             
                             out vec4 color;
                             
                             void main() {
                                 int width = imageSize(xyzw_tex).x;
                                 ivec2 pixel = ivec2(gl_VertexID % width, gl_VertexID / width);

                                 vec4 vertex_position = imageLoad(xyzw_tex, pixel);
                                 vec4 vertex_color = imageLoad(rgba_tex, pixel);

                                 color = vertex_color;
                                 gl_Position = mvp * vertex_position;
                                 gl_PointSize = point_size;
                             }
                             );
    opengl->glShaderSource(vertex_shader, 1, &vertex_code, NULL);
    opengl->glCompileShader(vertex_shader);
    
    GLuint fragment_shader = opengl->glCreateShader(GL_FRAGMENT_SHADER);
    char *fragment_code = GLSL(in vec4 color;
                               
                               layout(location = 0) out vec4 frag_color;
                               
                               void main() {
                                   if(color.a == 0.0f)
                                   {
                                       discard;
                                   }

                                   // converting from hsv to rgb below
                                   // code from: https://stackoverflow.com/questions/15095909/from-rgb-to-hsv-in-opengl-glsl
                                   vec4 k = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
                                   vec3 p = abs(fract(color.xxx + k.xyz) * 6.0 - k.www);
                                   vec3 color_rgb = color.z * mix(k.xxx, clamp(p - k.xxx, 0.0, 1.0), color.y);
                                   frag_color = vec4(color_rgb.rgb, 1.0);
                               }
                               );
    opengl->glShaderSource(fragment_shader, 1, &fragment_code, NULL);
    opengl->glCompileShader(fragment_shader);
    
    GLuint program = opengl->glCreateProgram();
    opengl->glAttachShader(program, vertex_shader);
    opengl->glAttachShader(program, fragment_shader);
    opengl->glLinkProgram(program);
    
    opengl->glValidateProgram(program);
    GLint linked = false;
    opengl->glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        char vertex_errors[1024];
        char fragment_errors[1024];
        char program_errors[1024];
        
        opengl->glGetShaderInfoLog(vertex_shader, sizeof(vertex_errors), NULL, vertex_errors);
        opengl->glGetShaderInfoLog(fragment_shader, sizeof(fragment_errors), NULL, fragment_errors);
        opengl->glGetProgramInfoLog(program, sizeof(program_errors), NULL, program_errors);
        
        printf("Error in vertex shader compilation: %s\n", vertex_errors);
        printf("Error in fragment shader compilation: %s\n", fragment_errors);
        printf("Error when linking attached shaders: %s\n", program_errors);
        
        assert(0 && "Shader validation failed!\n");
    }
    
    opengl->default_program = program;
}

static void compile_compute_program(open_gl *opengl)
{
    GLuint compute_shader = opengl->glCreateShader(GL_COMPUTE_SHADER);
    // the following compute shader code for fast point cloud calculation is similar to and inspired by:
    // https://github.com/microsoft/Azure-Kinect-Sensor-SDK/blob/develop/tools/k4aviewer/gpudepthtopointcloudconverter.cpp#L24
    char *compute_code = GLSL(layout(binding = 0, r16ui) readonly uniform uimage2D depth_image;
                              layout(binding = 1, rg32f) readonly uniform image2D xy_table;

                              layout(binding = 2, rgba32f) writeonly uniform image2D xyzw_tex;
                              layout(binding = 3, rgba32f) writeonly uniform image2D rgba_tex;

                              layout(location = 0) uniform float min_depth;
                              layout(location = 1) uniform float max_depth;
                              
                              layout(local_size_x = 1, local_size_y = 1) in;
                              
                              void main()
                              {
                                  ivec2 pixel = ivec2(gl_GlobalInvocationID.xy);
                                  
                                  //
                                  // Computing 3D position.

                                  float depth = float(imageLoad(depth_image, pixel));
                                  vec2 xy_value = imageLoad(xy_table, pixel).xy;
                                  
                                  float w = 1.0;
                                  
                                  if(xy_value.x == 0.0 && xy_value.y == 0.0)
                                  {
                                      w = 0.0;
                                  }
                                  
                                  vec3 position = vec3(xy_value.x * depth, -xy_value.y * depth, -depth);

                                  position /= 1000.0;

                                  //
                                  // Computing color using HSV.
                                  float hue = (-position.z - min_depth) / (max_depth - min_depth);
                                  hue = clamp(hue, 0.0, 1.0);
                                  
                                  float range = 2.0 / 3.0;

                                  hue *= range;
                                  hue = range - hue;

                                  vec3 color = vec3(hue, 1.0, 1.0);
                                  
                                  // 
                                  // Saving the position and color to a texture each.

                                  imageStore(xyzw_tex, pixel, vec4(position, w));
                                  imageStore(rgba_tex, pixel, vec4(color, w));
                              }
                              );

    opengl->glShaderSource(compute_shader, 1, &compute_code, NULL);
    opengl->glCompileShader(compute_shader);
    
    GLuint program = opengl->glCreateProgram();
    opengl->glAttachShader(program, compute_shader);
    opengl->glLinkProgram(program);
    
    opengl->glValidateProgram(program);
    GLint linked = false;
    opengl->glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if(!linked)
    {
        char shader_errors[1024];
        char program_errors[1024];
        
        opengl->glGetShaderInfoLog(compute_shader, sizeof(shader_errors), NULL, shader_errors);
        opengl->glGetProgramInfoLog(program, sizeof(program_errors), NULL, program_errors);
        
        printf("Error in compute shader compilation: %s\n", shader_errors);
        printf("Error when linking attached shaders: %s\n", program_errors);
        
        assert(0 && "Shader validation failed!\n");
    }
    
    opengl->compute_program = program;
}

open_gl *opengl_init(dimensions depth_image_dimensions)
{
    open_gl *opengl = (open_gl *)malloc(sizeof(open_gl));

    opengl->depth_image_dimensions = depth_image_dimensions;
    uint32_t width = opengl->depth_image_dimensions.w;
    uint32_t height = opengl->depth_image_dimensions.h;
    
#define get_opengl_function(name) opengl->name = (type_##name *)glfwGetProcAddress(#name);
    
    get_opengl_function(glDebugMessageCallback);
    get_opengl_function(glCreateShader);
    get_opengl_function(glShaderSource);
    get_opengl_function(glCompileShader);
    get_opengl_function(glGetShaderiv);
    get_opengl_function(glCreateProgram);
    get_opengl_function(glAttachShader);
    get_opengl_function(glLinkProgram);
    get_opengl_function(glGetProgramiv);
    get_opengl_function(glDetachShader);
    get_opengl_function(glDeleteShader);
    get_opengl_function(glUseProgram);
    get_opengl_function(glGenBuffers);
    get_opengl_function(glBufferData);
    get_opengl_function(glNamedBufferData);
    get_opengl_function(glGenVertexArrays);
    get_opengl_function(glBindVertexArray);
    get_opengl_function(glBindBuffer);
    get_opengl_function(glVertexAttribPointer);
    get_opengl_function(glEnableVertexAttribArray);
    get_opengl_function(glGetUniformLocation);
    get_opengl_function(glUniformMatrix4fv);
    get_opengl_function(glProgramUniformMatrix4fv);
    get_opengl_function(glGetProgramInfoLog);
    get_opengl_function(glGetShaderInfoLog);
    get_opengl_function(glGenSamplers);
    get_opengl_function(glDeleteSamplers);
    get_opengl_function(glBindSampler);
    get_opengl_function(glSamplerParameteri);
    get_opengl_function(glActiveTexture);
    get_opengl_function(glProgramUniform1i);
    get_opengl_function(glValidateProgram);
    get_opengl_function(glDrawBuffers);
    get_opengl_function(glBindImageTexture);
    get_opengl_function(glDispatchCompute);
    get_opengl_function(glMemoryBarrier);
    get_opengl_function(glUniform1i);
    get_opengl_function(glUniform1f);
    get_opengl_function(glTexStorage2D);
    
#ifdef DEBUG
    if(opengl->glDebugMessageCallback)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        opengl->glDebugMessageCallback(opengl_debug_message_callback, NULL);
    }
#endif
    
    compile_default_program(opengl);
    compile_compute_program(opengl);
    
    glGenTextures(1, &opengl->depth_map_texture);
    opengl->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl->depth_map_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glGenTextures(1, &opengl->xy_table_texture);
    opengl->glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, opengl->xy_table_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glGenTextures(1, &opengl->xyzw_table_texture);
    opengl->glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, opengl->xyzw_table_texture);
    opengl->glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, depth_image_dimensions.w, depth_image_dimensions.h);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    glGenTextures(1, &opengl->rgba_color_texture);
    opengl->glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, opengl->rgba_color_texture);
    opengl->glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA32F, depth_image_dimensions.w, depth_image_dimensions.h);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    GLuint dummy_vertex_array;
    opengl->glGenVertexArrays(1, &dummy_vertex_array);
    opengl->glBindVertexArray(dummy_vertex_array);

    return(opengl);
}

void calculate_point_cloud(open_gl *opengl, v2f *xy_map, uint16_t *depth_map)
{
    uint32_t width = opengl->depth_image_dimensions.w;
    uint32_t height = opengl->depth_image_dimensions.h;

    opengl->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl->depth_map_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_R16UI, width, height, 0, GL_RED_INTEGER, GL_UNSIGNED_SHORT, depth_map);
    opengl->glBindImageTexture(0, opengl->depth_map_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_R16UI);
    
    opengl->glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, opengl->xy_table_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RG32F, width, height, 0, GL_RG, GL_FLOAT, xy_map);
    opengl->glBindImageTexture(1, opengl->xy_table_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RG32F);
    
    opengl->glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, opengl->xyzw_table_texture);
    opengl->glBindImageTexture(2, opengl->xyzw_table_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    opengl->glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, opengl->rgba_color_texture);
    opengl->glBindImageTexture(3, opengl->rgba_color_texture, 0, GL_FALSE, 0, GL_WRITE_ONLY, GL_RGBA32F);
    
    opengl->glUseProgram(opengl->compute_program);
    
    opengl->glUniform1f(0, 0.5f);
    opengl->glUniform1f(1, 3.86f);
    
    opengl->glDispatchCompute(width, height, 1);
    opengl->glMemoryBarrier(GL_TEXTURE_FETCH_BARRIER_BIT);
}

void render_point_cloud(open_gl *opengl, dimensions render_dimensions, view_control *control, float point_size)
{
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    opengl->glUseProgram(opengl->default_program);

    opengl->glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, opengl->xyzw_table_texture);
    opengl->glBindImageTexture(0, opengl->xyzw_table_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);

    opengl->glActiveTexture(GL_TEXTURE3);
    glBindTexture(GL_TEXTURE_2D, opengl->rgba_color_texture);
    opengl->glBindImageTexture(1, opengl->rgba_color_texture, 0, GL_FALSE, 0, GL_READ_ONLY, GL_RGBA32F);
    
    uint32_t render_width = render_dimensions.w;
    uint32_t render_height = render_dimensions.h;

    uint32_t width = opengl->depth_image_dimensions.w;
    uint32_t height = opengl->depth_image_dimensions.h;
    
    mat4 model = control->model;
    mat4 view = look_at(control->position, v3f_add(control->position, control->forward), control->up);
    mat4 proj = perspective(control->fov, (float)render_width / (float)render_height, 0.1f, 100.0f);
    mat4 mvp = mat4_mul(proj, mat4_mul(view, model));
    opengl->glUniformMatrix4fv(0, 1, GL_TRUE, (float *)mvp.p);
    opengl->glUniform1f(1, point_size);
    
    glViewport(0, 0, render_width, render_height);
    
    glDrawArrays(GL_POINTS, 0, width * height);
}
