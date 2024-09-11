#include <stdlib.h>
#include <stdio.h>

#include <GL/gl.h>

#include "opengl_renderer.h"
#include "linalg.h"

typedef char GLchar;
typedef ptrdiff_t GLsizeiptr;
typedef uint64_t GLuint64;

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
typedef void   type_glGenQueries(GLsizei n, GLuint * ids);
typedef void   type_glBeginQuery(GLenum target, GLuint id);
typedef void   type_glEndQuery(GLenum target);
typedef void   type_glGetQueryObjectiv(GLuint id, GLenum pname, GLint * params);
typedef void   type_glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 * params);
typedef void   type_glQueryCounter(GLuint id, GLenum target);

typedef struct
{
    uint32_t w;
    uint32_t h;
} depth_image_dimension;

typedef struct
{
    color_point *vertex_array;
    uint32_t max_vertex_count;
    uint32_t vertex_count;
    
    v2u render_dim;
} opengl_frame;

#define opengl_function(name) type_##name *name

typedef struct
{
    GLuint default_program;
    
    GLuint vertex_buffer;
    
    GLuint query;
    
    color_point *vertex_array;
    uint32_t max_vertex_count;
    
    uint32_t depth_image_width;
    uint32_t depth_image_height;
    
    opengl_frame frame;
    
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
    opengl_function(glGenQueries);
    opengl_function(glBeginQuery);
    opengl_function(glEndQuery);
    opengl_function(glGetQueryObjectiv);
    opengl_function(glGetQueryObjectui64v);
    opengl_function(glQueryCounter);

} open_gl;

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
#define GL_TIME_ELAPSED                         0x88BF
#define GL_QUERY_RESULT                         0x8866
#define GL_QUERY_RESULT_AVAILABLE               0x8867
#define GL_TIMESTAMP                            0x8E28

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
    char *vertex_code = GLSL(layout(location = 0) in vec3 a_position;
                             layout(location = 1) in vec3 a_color;
                             
                             layout(location = 0) uniform mat4 mvp;
                             
                             out vec3 color;
                             
                             void main() {
                                 color = a_color;
                                 gl_Position = mvp * vec4(a_position, 1.0);
                                 gl_PointSize = 1.0;
                             }
                             );
    opengl->glShaderSource(vertex_shader, 1, &vertex_code, NULL);
    opengl->glCompileShader(vertex_shader);
    
    GLuint fragment_shader = opengl->glCreateShader(GL_FRAGMENT_SHADER);
    char *fragment_code = GLSL(in vec3 color;
                               
                               layout(location = 0) out vec4 frag_color;
                               
                               void main() {
                                   // converting from hsv to rgb below
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

open_gl *opengl_init(depth_image_dimension *dim)
{
    open_gl *opengl = (open_gl *)malloc(sizeof(open_gl));
    
    opengl->depth_image_width = dim->w;
    opengl->depth_image_height = dim->h;
    
    uint32_t max_vertex_count = dim->w * dim->h;
    opengl->vertex_array = (color_point *)malloc(sizeof(color_point) * max_vertex_count);
    opengl->max_vertex_count = max_vertex_count;
    
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
    get_opengl_function(glGenQueries);
    get_opengl_function(glBeginQuery);
    get_opengl_function(glEndQuery);
    get_opengl_function(glGetQueryObjectiv);
    get_opengl_function(glGetQueryObjectui64v);
    get_opengl_function(glQueryCounter);
    
#ifdef DEBUG
    if(opengl->glDebugMessageCallback)
    {
        glEnable(GL_DEBUG_OUTPUT);
        glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
        opengl->glDebugMessageCallback(opengl_debug_message_callback, NULL);
    }
#endif
    
    compile_default_program(opengl);
    
    GLuint universal_vertex_array;
    opengl->glGenVertexArrays(1, &universal_vertex_array);
    opengl->glBindVertexArray(universal_vertex_array);
    
    opengl->glGenBuffers(1, &opengl->vertex_buffer);
    opengl->glBindBuffer(GL_ARRAY_BUFFER, opengl->vertex_buffer);
    
    opengl->glGenQueries(1, &opengl->query);
    opengl->glBeginQuery(GL_TIME_ELAPSED, opengl->query);
    opengl->glEndQuery(GL_TIME_ELAPSED);
    
    return(opengl);
}

opengl_frame *opengl_begin_frame(open_gl *opengl, v2u render_dim)
{
    opengl_frame *frame = &opengl->frame;
    
    frame->render_dim = render_dim;
    
    frame->vertex_array = opengl->vertex_array;
    frame->max_vertex_count = opengl->max_vertex_count;
    frame->vertex_count = 0;
    
    return(frame);
}

void opengl_end_frame(open_gl *opengl, opengl_frame *frame, view_control *control/*, uint16_t *depth_map, float *xy_table*/)
{
    static timer RenderTimer = {1000};
    static unsigned frame_counter = 0;
    GLuint64 time_elapsed;
    
    // measure time
    GLint available = 0;
    opengl->glGetQueryObjectiv(opengl->query, GL_QUERY_RESULT_AVAILABLE, &available);
    if (available) {
        opengl->glGetQueryObjectui64v(opengl->query, GL_QUERY_RESULT, &time_elapsed);
        opengl->glBeginQuery(GL_TIME_ELAPSED, opengl->query);
        
        // printf("Draw Time: %f ms\n", time_elapsed / 1e+6f);
        PrintAverageTime(&RenderTimer, time_elapsed / 1e+9f, "Draw");
    }
    
    // start rendering set up
    glEnable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
    
    //
    // Draw the point cloud.
    opengl->glNamedBufferData(opengl->vertex_buffer, frame->vertex_count * sizeof(color_point), frame->vertex_array, GL_STATIC_DRAW);
    
    opengl->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(color_point), (void *)offsetof(color_point, xyz));
    opengl->glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(color_point), (void *)offsetof(color_point, rgb));
    opengl->glEnableVertexAttribArray(0);
    opengl->glEnableVertexAttribArray(1);
    
    opengl->glUseProgram(opengl->default_program);
    
    uint32_t render_width = frame->render_dim.x;
    uint32_t render_height = frame->render_dim.y;
    
    mat4 model = control->model;
    mat4 view = look_at(control->position, v3f_add(control->position, control->forward), control->up);
    mat4 proj = perspective(control->fov, (float)render_width / (float)render_height, 0.1f, 100.0f);
    mat4 mvp = mat4_mul(proj, mat4_mul(view, model));
    opengl->glUniformMatrix4fv(0, 1, GL_TRUE, (float *)mvp.p);
    
    glViewport(0, 0, render_width, render_height);
    
    glDrawArrays(GL_POINTS, 0, frame->vertex_count);
    
    // measure time
    if (available)
        opengl->glEndQuery(GL_TIME_ELAPSED);
    
    // glFinish();
    
    // unsigned prev_query_index = (query_index - 1) % QUERY_COUNT;
    // if (frame_counter >= 1) {
    //     GLint prev_query_available;
    //     opengl->glGetQueryObjectiv(opengl->queries[prev_query_index], GL_QUERY_RESULT_AVAILABLE, &prev_query_available);
    //     if(prev_query_available) {
    //         GLuint64 time_elapsed;
    //         opengl->glGetQueryObjectui64v(opengl->queries[prev_query_index], GL_QUERY_RESULT, &time_elapsed);
    //         PrintAverageTime(&RenderTimer, time_elapsed / 1e+6f);
    //         // printf("%f\n", (time_end - time_begin) / 1e+6f);
    //     }
    // }
    
    
    frame_counter++;
}
