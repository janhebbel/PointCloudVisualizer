#include <GL/gl.h>

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
typedef void   type_glClearTexImage(GLuint texture, GLint level, GLenum format, GLenum type, const void * data);
typedef void   type_glGenFramebuffers(GLsizei n, GLuint *ids);
typedef void   type_glBindFramebuffer(GLenum target, GLuint framebuffer);
typedef void   type_glGenRenderbuffers(GLsizei n, GLuint *renderbuffers);
typedef void   type_glBindRenderbuffer(GLenum target, GLuint renderbuffer);
typedef void   type_glRenderbufferStorage(GLenum target, GLenum internalformat, GLsizei width, GLsizei height);
typedef void   type_glFramebufferRenderbuffer(GLenum target, GLenum attachment, GLenum renderbuffertarget, GLuint renderbuffer);
typedef void   type_glTexStorage3D(GLenum target, GLsizei levels, GLenum internalformat, GLsizei width, GLsizei height, GLsizei depth);
typedef void   type_glGenQueries(GLsizei n, GLuint * ids);
typedef void   type_glBeginQuery(GLenum target, GLuint id);
typedef void   type_glEndQuery(GLenum target);
typedef void   type_glGetQueryObjectiv(GLuint id, GLenum pname, GLint * params);
typedef void   type_glGetQueryObjectui64v(GLuint id, GLenum pname, GLuint64 * params);
typedef void   type_glQueryCounter(GLuint id, GLenum target);

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
#define GL_TEXTURE_2D_ARRAY                     0x8C1A
#define GL_CLAMP_TO_EDGE                        0x812F
#define GL_RGBA32F                              0x8814
#define GL_RG32F                                0x8230
#define GL_R16UI                                0x8234
#define GL_RG                                   0x8227
#define GL_DEPTH_COMPONENT32F                   0x8CAC
#define GL_RED_INTEGER                          0x8D94
#define GL_READ_ONLY                            0x88B8
#define GL_WRITE_ONLY                           0x88B9
#define GL_READ_WRITE                           0x88BA
#define GL_FRAMEBUFFER                          0x8D40
#define GL_RENDERBUFFER                         0x8D41
#define GL_COLOR_ATTACHMENT0                    0x8CE0
#define GL_COLOR_ATTACHMENT1                    0x8CE1
#define GL_COLOR_ATTACHMENT2                    0x8CE2
#define GL_COLOR_ATTACHMENT3                    0x8CE3
#define GL_COLOR_ATTACHMENT4                    0x8CE4
#define GL_COLOR_ATTACHMENT5                    0x8CE5
#define GL_COLOR_ATTACHMENT6                    0x8CE6
#define GL_COLOR_ATTACHMENT7                    0x8CE7
#define GL_COLOR_ATTACHMENT8                    0x8CE8
#define GL_COLOR_ATTACHMENT9                    0x8CE9
#define GL_COLOR_ATTACHMENT10                   0x8CEA
#define GL_COLOR_ATTACHMENT11                   0x8CEB
#define GL_COLOR_ATTACHMENT12                   0x8CEC
#define GL_COLOR_ATTACHMENT13                   0x8CED
#define GL_COLOR_ATTACHMENT14                   0x8CEE
#define GL_COLOR_ATTACHMENT15                   0x8CEF
#define GL_COLOR_ATTACHMENT16                   0x8CF0
#define GL_COLOR_ATTACHMENT17                   0x8CF1
#define GL_COLOR_ATTACHMENT18                   0x8CF2
#define GL_COLOR_ATTACHMENT19                   0x8CF3
#define GL_COLOR_ATTACHMENT20                   0x8CF4
#define GL_COLOR_ATTACHMENT21                   0x8CF5
#define GL_COLOR_ATTACHMENT22                   0x8CF6
#define GL_COLOR_ATTACHMENT23                   0x8CF7
#define GL_COLOR_ATTACHMENT24                   0x8CF8
#define GL_COLOR_ATTACHMENT25                   0x8CF9
#define GL_COLOR_ATTACHMENT26                   0x8CFA
#define GL_COLOR_ATTACHMENT27                   0x8CFB
#define GL_COLOR_ATTACHMENT28                   0x8CFC
#define GL_COLOR_ATTACHMENT29                   0x8CFD
#define GL_COLOR_ATTACHMENT30                   0x8CFE
#define GL_COLOR_ATTACHMENT31                   0x8CFF
#define GL_DEPTH_ATTACHMENT                     0x8D00
#define GL_SHADER_IMAGE_ACCESS_BARRIER_BIT      0x00000020
#define GL_TEXTURE_FETCH_BARRIER_BIT            0x00000008
#define GL_TIME_ELAPSED                         0x88BF
#define GL_QUERY_RESULT                         0x8866
#define GL_QUERY_RESULT_AVAILABLE               0x8867
#define GL_TIMESTAMP                            0x8E28

#define opengl_function(name) type_##name *name

#define QUERY_COUNT 5

typedef struct
{
    GLuint default_program;
    
    GLuint framebuffer_texture;
    
    GLsizei vertex_count;
    
    GLuint queries[QUERY_COUNT];
    
    uint32_t framebuffer_width;
    uint32_t framebuffer_height;
    
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
    opengl_function(glClearTexImage);
    opengl_function(glGenFramebuffers);
    opengl_function(glBindFramebuffer);
    opengl_function(glGenRenderbuffers);
    opengl_function(glBindRenderbuffer);
    opengl_function(glRenderbufferStorage);
    opengl_function(glFramebufferRenderbuffer);
    opengl_function(glTexStorage3D);
    opengl_function(glGenQueries);
    opengl_function(glBeginQuery);
    opengl_function(glEndQuery);
    opengl_function(glGetQueryObjectiv);
    opengl_function(glGetQueryObjectui64v);
    opengl_function(glQueryCounter);

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
    char *vertex_code = GLSL(
        layout(location=0) in vec2 APos;
        layout(location=1) in vec2 ATexCoords;
    
        out vec2 TexCoords;
    
        void main() 
        {
            TexCoords = ATexCoords;
            gl_Position = vec4(APos.x, APos.y, 0.0f, 1.0f);
        }
    );
    opengl->glShaderSource(vertex_shader, 1, &vertex_code, NULL);
    opengl->glCompileShader(vertex_shader);
    
    GLuint fragment_shader = opengl->glCreateShader(GL_FRAGMENT_SHADER);
    char *fragment_code = GLSL(
        in vec2 TexCoords;
        
        out vec4 FragColor;
        
        layout(location=0) uniform sampler2D ScreenTexture;
        
        void main() 
        {
            FragColor = texture(ScreenTexture, TexCoords);
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

open_gl *OpenGLInit(GLsizei WindowWidth, GLsizei WindowHeight)
{
    open_gl *opengl = (open_gl *)malloc(sizeof(open_gl));
    
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
    get_opengl_function(glClearTexImage);
    get_opengl_function(glGenFramebuffers);
    get_opengl_function(glBindFramebuffer);
    get_opengl_function(glGenRenderbuffers);
    get_opengl_function(glBindRenderbuffer);
    get_opengl_function(glRenderbufferStorage);
    get_opengl_function(glFramebufferRenderbuffer);
    get_opengl_function(glTexStorage3D);
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

    glGenTextures(1, &opengl->framebuffer_texture);
    opengl->glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, opengl->framebuffer_texture);
    opengl->glTexStorage2D(GL_TEXTURE_2D, 1, GL_RGBA8, WindowWidth, WindowHeight);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    
    opengl->framebuffer_width = WindowWidth;
    opengl->framebuffer_height = WindowHeight;
    
    GLuint dummy_vertex_array;
    opengl->glGenVertexArrays(1, &dummy_vertex_array);
    opengl->glBindVertexArray(dummy_vertex_array);
    
    GLuint vertex_buffer;
    opengl->glGenBuffers(1, &vertex_buffer);
    opengl->glBindBuffer(GL_ARRAY_BUFFER, vertex_buffer);
    
    opengl->glGenQueries(QUERY_COUNT, opengl->queries);
    
    float QuadVertices[] = 
    {
        -1.0f, -1.0f, 0.0f, 0.0f,
        -1.0f,  1.0f, 0.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
         1.0f,  1.0f, 1.0f, 1.0f,
         1.0f, -1.0f, 1.0f, 0.0f,
        -1.0f, -1.0f, 0.0f, 0.0f
    };
    
    GLsizei Stride = 4 * sizeof(float);
    opengl->vertex_count = sizeof(QuadVertices) / Stride;
    
    opengl->glNamedBufferData(vertex_buffer, sizeof(QuadVertices), QuadVertices, GL_STATIC_DRAW);
    
    opengl->glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, Stride, (void *)(sizeof(float) * 0));
    opengl->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, Stride, (void *)(sizeof(float) * 2));
    opengl->glEnableVertexAttribArray(0);
    opengl->glEnableVertexAttribArray(1);

    return(opengl);
}

void OpenGLRenderToScreen(open_gl *OpenGL, uint32_t RenderWidth, uint32_t RenderHeight)
{
    static average AvgDrawTimeGPU = {.CountTo = 1000, .Msg = "Draw GPU", "ms\n"};
    static unsigned frame_counter = 0;
    unsigned query_index = frame_counter % QUERY_COUNT;
    
    // measure time
    OpenGL->glBeginQuery(GL_TIME_ELAPSED, OpenGL->queries[query_index]);
    
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_PROGRAM_POINT_SIZE);
    glFrontFace(GL_CW);
    
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    OpenGL->glUseProgram(OpenGL->default_program);

    OpenGL->glActiveTexture(GL_TEXTURE0);
    OpenGL->glUniform1i(0, 0); // Sampler at location 0 is set to GL_TEXTURE0
    
    glViewport(0, 0, RenderWidth, RenderHeight);
    
    glDrawArrays(GL_TRIANGLES, 0, OpenGL->vertex_count);
    
    // measure time
    OpenGL->glEndQuery(GL_TIME_ELAPSED);
    
    // look back 4 frames to make sure all queries are finished once requested
    unsigned prev_query_index = (query_index + 1) % QUERY_COUNT;
    if (frame_counter >= 4) {
        GLint prev_query_available;
        OpenGL->glGetQueryObjectiv(OpenGL->queries[prev_query_index], GL_QUERY_RESULT_AVAILABLE, &prev_query_available);
        if(prev_query_available) {
            GLuint64 time_elapsed;
            OpenGL->glGetQueryObjectui64v(OpenGL->queries[prev_query_index], GL_QUERY_RESULT, &time_elapsed);
            PrintAverage(&AvgDrawTimeGPU, time_elapsed / 1e6);
        }
    }
    
    frame_counter++;
}
