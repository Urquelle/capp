#include <gl/gl.h>

typedef struct Gfx_Win32Meta Gfx_Win32Meta;

struct Gfx_Win32Meta {
    HWND window_handle;
};

/* wgl {{{ */
#define WGL_CONTEXT_MAJOR_VERSION_ARB                0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB                0x2092
#define WGL_CONTEXT_LAYER_PLANE_ARB                  0x2093
#define WGL_CONTEXT_FLAGS_ARB                        0x2094
#define WGL_CONTEXT_PROFILE_MASK_ARB                 0x9126

#define WGL_CONTEXT_DEBUG_BIT_ARB                    0x0001
#define WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB       0x0002

#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB             0x00000001
#define WGL_CONTEXT_COMPATIBILITY_PROFILE_BIT_ARB    0x00000002

#define WGL_DRAW_TO_WINDOW_ARB                       0x2001
#define WGL_ACCELERATION_ARB                         0x2003
#define WGL_SUPPORT_OPENGL_ARB                       0x2010
#define WGL_DOUBLE_BUFFER_ARB                        0x2011
#define WGL_PIXEL_TYPE_ARB                           0x2013

#define WGL_TYPE_RGBA_ARB                            0x202B
#define WGL_FULL_ACCELERATION_ARB                    0x2027

#define WGL_FRAMEBUFFER_SRGB_CAPABLE_ARB             0x20A9

#define WGL_RED_BITS_ARB                             0x2015
#define WGL_GREEN_BITS_ARB                           0x2017
#define WGL_BLUE_BITS_ARB                            0x2019
#define WGL_ALPHA_BITS_ARB                           0x201B
#define WGL_DEPTH_BITS_ARB                           0x2022
/* }}} */
typedef HGLRC WINAPI Wgl_Create_Context_Attribs(HDC hDC, HGLRC hShareContext, const int *attribList);
static Wgl_Create_Context_Attribs *wglCreateContextAttribs;
typedef char GLchar;
typedef unsigned int GLhandle;
typedef unsigned int GLenum;
typedef ptrdiff_t GLintptr;
typedef ptrdiff_t GLsizeiptr;
/* proc typedefs {{{ */
typedef GLuint WINAPI Opengl_Create_Program(void);
typedef void WINAPI Opengl_Delete_Program(GLuint program);
typedef void WINAPI Opengl_Use_Program(GLuint program);
typedef void WINAPI Opengl_Link_Program(GLhandle programObj);
typedef GLboolean WINAPI Opengl_Is_Program(GLuint program);
typedef void WINAPI Opengl_Get_Programiv(GLenum target, GLenum pname, GLint* params);
typedef void WINAPI Opengl_Get_Program_Info_Log(GLuint program, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef void WINAPI Opengl_Attach_Shader(GLuint program, GLuint shader);
typedef void WINAPI Opengl_Compile_Shader(GLuint shader);
typedef GLuint WINAPI Opengl_Create_Shader(GLenum type);
typedef void WINAPI Opengl_Detach_Shader(GLuint program, GLuint shader);
typedef void WINAPI Opengl_Delete_Shader(GLuint shader);
typedef void WINAPI Opengl_Shader_Source(GLuint shader, GLsizei count, GLchar **string, GLint *length);
typedef void WINAPI Opengl_Get_Shaderiv(GLuint shader, GLenum pname, GLint* param);
typedef void WINAPI Opengl_Get_Shader_Info_Log(GLuint shader, GLsizei bufSize, GLsizei* length, GLchar* infoLog);
typedef GLint WINAPI Opengl_Get_Uniform_Location(GLuint program, const GLchar* name);
typedef void WINAPI Opengl_Uniform1f(GLint location, GLfloat v0);
typedef void WINAPI Opengl_Uniform1fv(GLint location, GLsizei count, const GLfloat* value);
typedef void WINAPI Opengl_Uniform1i(GLint location, GLint v0);
typedef void WINAPI Opengl_Uniform1iv(GLint location, GLsizei count, const GLint* value);
typedef void WINAPI Opengl_Uniform2f(GLint location, GLfloat v0, GLfloat v1);
typedef void WINAPI Opengl_Uniform2fv(GLint location, GLsizei count, const GLfloat* value);
typedef void WINAPI Opengl_Uniform2i(GLint location, GLint v0, GLint v1);
typedef void WINAPI Opengl_Uniform2iv(GLint location, GLsizei count, const GLint* value);
typedef void WINAPI Opengl_Uniform3f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2);
typedef void WINAPI Opengl_Uniform3fv(GLint location, GLsizei count, const GLfloat* value);
typedef void WINAPI Opengl_Uniform3i(GLint location, GLint v0, GLint v1, GLint v2);
typedef void WINAPI Opengl_Uniform3iv(GLint location, GLsizei count, const GLint* value);
typedef void WINAPI Opengl_Uniform4f(GLint location, GLfloat v0, GLfloat v1, GLfloat v2, GLfloat v3);
typedef void WINAPI Opengl_Uniform4fv(GLint location, GLsizei count, const GLfloat* value);
typedef void WINAPI Opengl_Uniform4i(GLint location, GLint v0, GLint v1, GLint v2, GLint v3);
typedef void WINAPI Opengl_Uniform4iv(GLint location, GLsizei count, const GLint* value);
typedef GLuint WINAPI Opengl_Get_Uniform_Block_Index(GLuint program, const GLchar* uniformBlockName);
typedef void WINAPI Opengl_Get_Active_Uniform_Blockiv(GLuint program, GLuint uniformBlockIndex, GLenum pname, GLint* params);
typedef void WINAPI Opengl_Gen_Buffers(GLsizei n, GLuint* buffers);
typedef void WINAPI Opengl_Create_Buffers(GLsizei n, GLuint* buffers);
typedef void WINAPI Opengl_Delete_Buffers(GLsizei n, const GLuint* buffers);
typedef void WINAPI Opengl_Bind_Buffer(GLenum target, GLuint buffer);
typedef void* WINAPI Opengl_Map_Buffer(GLenum target, GLenum access);
typedef GLboolean WINAPI Opengl_Unmap_Buffer(GLenum target);
typedef void WINAPI Opengl_Bind_Buffer_Base(GLenum target, GLuint index, GLuint buffer);
typedef void WINAPI Opengl_Buffer_Data(GLenum target, GLsizeiptr size, const void *data, GLenum usage);
typedef void WINAPI Opengl_Buffer_Sub_Data(GLenum target, GLintptr offset, GLsizeiptr size, const void *data);
typedef void WINAPI Opengl_Buffer_Storage(GLenum target, GLsizeiptr size, const void *data, GLbitfield flags);
typedef void WINAPI Opengl_Gen_Vertex_Arrays(GLsizei n, GLuint* arrays);
typedef void WINAPI Opengl_Bind_Vertex_Array(GLuint array);
typedef void WINAPI Opengl_Vertex_Attrib_Pointer(GLuint index, GLint size, GLenum type, GLboolean normalized, GLsizei stride, const void* pointer);
typedef void WINAPI Opengl_Enable_Vertex_Attrib_Array(GLuint index);
typedef void WINAPI Opengl_Disable_Vertex_Attrib_Array(GLuint index);
typedef void WINAPI Opengl_Clear_Bufferfv(GLenum buffer, GLint drawBuffer, const GLfloat* value);
typedef void WINAPI Opengl_Vertex_Attrib1d(GLuint index, GLdouble x);
typedef void WINAPI Opengl_Vertex_Attrib1dv(GLuint index, const GLdouble* v);
typedef void WINAPI Opengl_Vertex_Attrib1f(GLuint index, GLfloat x);
typedef void WINAPI Opengl_Vertex_Attrib1fv(GLuint index, const GLfloat* v);
typedef void WINAPI Opengl_Vertex_Attrib1s(GLuint index, GLshort x);
typedef void WINAPI Opengl_Vertex_Attrib1sv(GLuint index, const GLshort* v);
typedef void WINAPI Opengl_Vertex_Attrib2d(GLuint index, GLdouble x, GLdouble y);
typedef void WINAPI Opengl_Vertex_Attrib2dv(GLuint index, const GLdouble* v);
typedef void WINAPI Opengl_Vertex_Attrib2f(GLuint index, GLfloat x, GLfloat y);
typedef void WINAPI Opengl_Vertex_Attrib2fv(GLuint index, const GLfloat* v);
typedef void WINAPI Opengl_Vertex_Attrib2s(GLuint index, GLshort x, GLshort y);
typedef void WINAPI Opengl_Vertex_Attrib2sv(GLuint index, const GLshort* v);
typedef void WINAPI Opengl_Vertex_Attrib3d(GLuint index, GLdouble x, GLdouble y, GLdouble z);
typedef void WINAPI Opengl_Vertex_Attrib3dv(GLuint index, const GLdouble* v);
typedef void WINAPI Opengl_Vertex_Attrib3f(GLuint index, GLfloat x, GLfloat y, GLfloat z);
typedef void WINAPI Opengl_Vertex_Attrib3fv(GLuint index, const GLfloat* v);
typedef void WINAPI Opengl_Vertex_Attrib3s(GLuint index, GLshort x, GLshort y, GLshort z);
typedef void WINAPI Opengl_Vertex_Attrib3sv(GLuint index, const GLshort* v);
typedef void WINAPI Opengl_Vertex_Attrib4Nbv(GLuint index, const GLbyte* v);
typedef void WINAPI Opengl_Vertex_Attrib4Niv(GLuint index, const GLint* v);
typedef void WINAPI Opengl_Vertex_Attrib4Nsv(GLuint index, const GLshort* v);
typedef void WINAPI Opengl_Vertex_Attrib4Nub(GLuint index, GLubyte x, GLubyte y, GLubyte z, GLubyte w);
typedef void WINAPI Opengl_Vertex_Attrib4Nubv(GLuint index, const GLubyte* v);
typedef void WINAPI Opengl_Vertex_Attrib4Nuiv(GLuint index, const GLuint* v);
typedef void WINAPI Opengl_Vertex_Attrib4Nusv(GLuint index, const GLushort* v);
typedef void WINAPI Opengl_Vertex_Attrib4bv(GLuint index, const GLbyte* v);
typedef void WINAPI Opengl_Vertex_Attrib4d(GLuint index, GLdouble x, GLdouble y, GLdouble z, GLdouble w);
typedef void WINAPI Opengl_Vertex_Attrib4dv(GLuint index, const GLdouble* v);
typedef void WINAPI Opengl_Vertex_Attrib4f(GLuint index, GLfloat x, GLfloat y, GLfloat z, GLfloat w);
typedef void WINAPI Opengl_Vertex_Attrib4fv(GLuint index, const GLfloat* v);
typedef void WINAPI Opengl_Vertex_Attrib4iv(GLuint index, const GLint* v);
typedef void WINAPI Opengl_Vertex_Attrib4s(GLuint index, GLshort x, GLshort y, GLshort z, GLshort w);
typedef void WINAPI Opengl_Vertex_Attrib4sv(GLuint index, const GLshort* v);
typedef void WINAPI Opengl_Vertex_Attrib4ubv(GLuint index, const GLubyte* v);
typedef void WINAPI Opengl_Vertex_Attrib4uiv(GLuint index, const GLuint* v);
typedef void WINAPI Opengl_Vertex_Attrib4usv(GLuint index, const GLushort* v);
/* }}} */
/* proc pointers {{{ */
static Opengl_Create_Program                  *glCreateProgram;
static Opengl_Delete_Program                  *glDeleteProgram;
static Opengl_Is_Program                      *glIsProgram;
static Opengl_Use_Program                     *glUseProgram;
static Opengl_Get_Programiv                   *glGetProgramiv;
static Opengl_Get_Program_Info_Log            *glGetProgramInfoLog;
static Opengl_Link_Program                    *glLinkProgram;
static Opengl_Attach_Shader                   *glAttachShader;
static Opengl_Compile_Shader                  *glCompileShader;
static Opengl_Create_Shader                   *glCreateShader;
static Opengl_Detach_Shader                   *glDetachShader;
static Opengl_Delete_Shader                   *glDeleteShader;
static Opengl_Shader_Source                   *glShaderSource;
static Opengl_Get_Shaderiv                    *glGetShaderiv;
static Opengl_Get_Shader_Info_Log             *glGetShaderInfoLog;
static Opengl_Get_Uniform_Location            *glGetUniformLocation;
static Opengl_Uniform1f                       *glUniform1f;
static Opengl_Uniform1fv                      *glUniform1fv;
static Opengl_Uniform1i                       *glUniform1i;
static Opengl_Uniform1iv                      *glUniform1iv;
static Opengl_Uniform2f                       *glUniform2f;
static Opengl_Uniform2fv                      *glUniform2fv;
static Opengl_Uniform2i                       *glUniform2i;
static Opengl_Uniform2iv                      *glUniform2iv;
static Opengl_Uniform3f                       *glUniform3f;
static Opengl_Uniform3fv                      *glUniform3fv;
static Opengl_Uniform3i                       *glUniform3i;
static Opengl_Uniform3iv                      *glUniform3iv;
static Opengl_Uniform4f                       *glUniform4f;
static Opengl_Uniform4fv                      *glUniform4fv;
static Opengl_Uniform4i                       *glUniform4i;
static Opengl_Uniform4iv                      *glUniform4iv;
static Opengl_Get_Uniform_Block_Index         *glGetUniformBlockIndex;
static Opengl_Get_Active_Uniform_Blockiv      *glGetActiveUniformBlockiv;
static Opengl_Gen_Buffers                     *glGenBuffers;
static Opengl_Create_Buffers                  *glCreateBuffers;
static Opengl_Delete_Buffers                  *glDeleteBuffers;
static Opengl_Bind_Buffer                     *glBindBuffer;
static Opengl_Map_Buffer                      *glMapBuffer;
static Opengl_Unmap_Buffer                    *glUnmapBuffer;
static Opengl_Bind_Buffer_Base                *glBindBufferBase;
static Opengl_Buffer_Data                     *glBufferData;
static Opengl_Buffer_Sub_Data                 *glBufferSubData;
static Opengl_Buffer_Storage                  *glBufferStorage;
static Opengl_Clear_Bufferfv                  *glClearBufferfv;
static Opengl_Gen_Vertex_Arrays               *glGenVertexArrays;
static Opengl_Bind_Vertex_Array               *glBindVertexArray;
static Opengl_Vertex_Attrib_Pointer           *glVertexAttribPointer;
static Opengl_Enable_Vertex_Attrib_Array      *glEnableVertexAttribArray;
static Opengl_Disable_Vertex_Attrib_Array     *glDisableVertexAttribArray;
static Opengl_Vertex_Attrib1d                 *glVertexAttrib1d;
static Opengl_Vertex_Attrib1dv                *glVertexAttrib1dv;
static Opengl_Vertex_Attrib1f                 *glVertexAttrib1f;
static Opengl_Vertex_Attrib1fv                *glVertexAttrib1fv;
static Opengl_Vertex_Attrib1s                 *glVertexAttrib1s;
static Opengl_Vertex_Attrib1sv                *glVertexAttrib1sv;
static Opengl_Vertex_Attrib2d                 *glVertexAttrib2d;
static Opengl_Vertex_Attrib2dv                *glVertexAttrib2dv;
static Opengl_Vertex_Attrib2f                 *glVertexAttrib2f;
static Opengl_Vertex_Attrib2fv                *glVertexAttrib2fv;
static Opengl_Vertex_Attrib2s                 *glVertexAttrib2s;
static Opengl_Vertex_Attrib2sv                *glVertexAttrib2sv;
static Opengl_Vertex_Attrib3d                 *glVertexAttrib3d;
static Opengl_Vertex_Attrib3dv                *glVertexAttrib3dv;
static Opengl_Vertex_Attrib3f                 *glVertexAttrib3f;
static Opengl_Vertex_Attrib3fv                *glVertexAttrib3fv;
static Opengl_Vertex_Attrib3s                 *glVertexAttrib3s;
static Opengl_Vertex_Attrib3sv                *glVertexAttrib3sv;
static Opengl_Vertex_Attrib4Nbv               *glVertexAttrib4Nbv;
static Opengl_Vertex_Attrib4Niv               *glVertexAttrib4Niv;
static Opengl_Vertex_Attrib4Nsv               *glVertexAttrib4Nsv;
static Opengl_Vertex_Attrib4Nub               *glVertexAttrib4Nub;
static Opengl_Vertex_Attrib4Nubv              *glVertexAttrib4Nubv;
static Opengl_Vertex_Attrib4Nuiv              *glVertexAttrib4Nuiv;
static Opengl_Vertex_Attrib4Nusv              *glVertexAttrib4Nusv;
static Opengl_Vertex_Attrib4bv                *glVertexAttrib4bv;
static Opengl_Vertex_Attrib4d                 *glVertexAttrib4d;
static Opengl_Vertex_Attrib4dv                *glVertexAttrib4dv;
static Opengl_Vertex_Attrib4f                 *glVertexAttrib4f;
static Opengl_Vertex_Attrib4fv                *glVertexAttrib4fv;
static Opengl_Vertex_Attrib4iv                *glVertexAttrib4iv;
static Opengl_Vertex_Attrib4s                 *glVertexAttrib4s;
static Opengl_Vertex_Attrib4sv                *glVertexAttrib4sv;
static Opengl_Vertex_Attrib4ubv               *glVertexAttrib4ubv;
static Opengl_Vertex_Attrib4uiv               *glVertexAttrib4uiv;
static Opengl_Vertex_Attrib4usv               *glVertexAttrib4usv;
/* }}} */
/* set_opengl_proc_pointer {{{ */
static void
internal__gfx_set_opengl_proc_pointer() {
    glCreateProgram             = (Opengl_Create_Program *)wglGetProcAddress("glCreateProgram");
    glIsProgram                 = (Opengl_Is_Program *)wglGetProcAddress("glIsProgram");
    glDeleteProgram             = (Opengl_Delete_Program *)wglGetProcAddress("glDeleteProgram");
    glUseProgram                = (Opengl_Use_Program *)wglGetProcAddress("glUseProgram");
    glLinkProgram               = (Opengl_Link_Program *)wglGetProcAddress("glLinkProgram");
    glGetProgramiv              = (Opengl_Get_Programiv *)wglGetProcAddress("glGetProgramiv");
    glGetProgramInfoLog         = (Opengl_Get_Program_Info_Log *)wglGetProcAddress("glGetProgramInfoLog");
    glAttachShader              = (Opengl_Attach_Shader *)wglGetProcAddress("glAttachShader");
    glCompileShader             = (Opengl_Compile_Shader *)wglGetProcAddress("glCompileShader");
    glCreateShader              = (Opengl_Create_Shader *)wglGetProcAddress("glCreateShader");
    glDetachShader              = (Opengl_Detach_Shader *)wglGetProcAddress("glDetachShader");
    glDeleteShader              = (Opengl_Delete_Shader *)wglGetProcAddress("glDeleteShader");
    glShaderSource              = (Opengl_Shader_Source *)wglGetProcAddress("glShaderSource");
    glGetShaderiv               = (Opengl_Get_Shaderiv *)wglGetProcAddress("glGetShaderiv");
    glGetShaderInfoLog          = (Opengl_Get_Shader_Info_Log *)wglGetProcAddress("glGetShaderInfoLog");
    glGetUniformLocation        = (Opengl_Get_Uniform_Location *)wglGetProcAddress("glGetUniformLocation");
    glUniform1f                 = (Opengl_Uniform1f *)wglGetProcAddress("glUniform1f");
    glUniform1fv                = (Opengl_Uniform1fv *)wglGetProcAddress("glUniform1fv");
    glUniform1i                 = (Opengl_Uniform1i *)wglGetProcAddress("glUniform1i");
    glUniform1iv                = (Opengl_Uniform1iv *)wglGetProcAddress("glUniform1iv");
    glUniform2f                 = (Opengl_Uniform2f *)wglGetProcAddress("glUniform2f");
    glUniform2fv                = (Opengl_Uniform2fv *)wglGetProcAddress("glUniform2fv");
    glUniform2i                 = (Opengl_Uniform2i *)wglGetProcAddress("glUniform2i");
    glUniform2iv                = (Opengl_Uniform2iv *)wglGetProcAddress("glUniform2iv");
    glUniform3f                 = (Opengl_Uniform3f *)wglGetProcAddress("glUniform3f");
    glUniform3fv                = (Opengl_Uniform3fv *)wglGetProcAddress("glUniform3fv");
    glUniform3i                 = (Opengl_Uniform3i *)wglGetProcAddress("glUniform3i");
    glUniform3iv                = (Opengl_Uniform3iv *)wglGetProcAddress("glUniform3iv");
    glUniform4f                 = (Opengl_Uniform4f *)wglGetProcAddress("glUniform4f");
    glUniform4fv                = (Opengl_Uniform4fv *)wglGetProcAddress("glUniform4fv");
    glUniform4i                 = (Opengl_Uniform4i *)wglGetProcAddress("glUniform4i");
    glUniform4iv                = (Opengl_Uniform4iv *)wglGetProcAddress("glUniform4iv");
    glGetUniformBlockIndex      = (Opengl_Get_Uniform_Block_Index *)wglGetProcAddress("glGetUniformBlockIndex");
    glGetActiveUniformBlockiv   = (Opengl_Get_Active_Uniform_Blockiv *)wglGetProcAddress("glGetActiveUniformBlockiv");
    glGenBuffers                = (Opengl_Gen_Buffers *)wglGetProcAddress("glGenBuffers");
    glCreateBuffers             = (Opengl_Create_Buffers *)wglGetProcAddress("glCreateBuffers");
    glDeleteBuffers             = (Opengl_Delete_Buffers *)wglGetProcAddress("glDeleteBuffers");
    glBindBuffer                = (Opengl_Bind_Buffer *)wglGetProcAddress("glBindBuffer");
    glMapBuffer                 = (Opengl_Map_Buffer *)wglGetProcAddress("glMapBuffer");
    glUnmapBuffer               = (Opengl_Unmap_Buffer *)wglGetProcAddress("glUnmapBuffer");
    glBindBufferBase            = (Opengl_Bind_Buffer_Base *)wglGetProcAddress("glBindBufferBase");
    glBufferData                = (Opengl_Buffer_Data *)wglGetProcAddress("glBufferData");
    glBufferSubData             = (Opengl_Buffer_Sub_Data *)wglGetProcAddress("glBufferSubData");
    glBufferStorage             = (Opengl_Buffer_Storage *)wglGetProcAddress("glBufferStorage");
    glGenVertexArrays           = (Opengl_Gen_Vertex_Arrays *)wglGetProcAddress("glGenVertexArrays");
    glBindVertexArray           = (Opengl_Bind_Vertex_Array *)wglGetProcAddress("glBindVertexArray");
    glVertexAttribPointer       = (Opengl_Vertex_Attrib_Pointer *)wglGetProcAddress("glVertexAttribPointer");
    glEnableVertexAttribArray   = (Opengl_Enable_Vertex_Attrib_Array *)wglGetProcAddress("glEnableVertexAttribArray");
    glDisableVertexAttribArray  = (Opengl_Disable_Vertex_Attrib_Array *)wglGetProcAddress("glDisableVertexAttribArray");
    glClearBufferfv             = (Opengl_Clear_Bufferfv *)wglGetProcAddress("glClearBufferfv");
    glVertexAttrib1d            = (Opengl_Vertex_Attrib1d *)wglGetProcAddress("glVertexAttrib1d");
    glVertexAttrib1dv           = (Opengl_Vertex_Attrib1dv *)wglGetProcAddress("glVertexAttrib1dv");
    glVertexAttrib1f            = (Opengl_Vertex_Attrib1f *)wglGetProcAddress("glVertexAttrib1f");
    glVertexAttrib1fv           = (Opengl_Vertex_Attrib1fv *)wglGetProcAddress("glVertexAttrib1fv");
    glVertexAttrib1s            = (Opengl_Vertex_Attrib1s *)wglGetProcAddress("glVertexAttrib1s");
    glVertexAttrib1sv           = (Opengl_Vertex_Attrib1sv *)wglGetProcAddress("glVertexAttrib1sv");
    glVertexAttrib2d            = (Opengl_Vertex_Attrib2d *)wglGetProcAddress("glVertexAttrib2d");
    glVertexAttrib2dv           = (Opengl_Vertex_Attrib2dv *)wglGetProcAddress("glVertexAttrib2dv");
    glVertexAttrib2f            = (Opengl_Vertex_Attrib2f *)wglGetProcAddress("glVertexAttrib2f");
    glVertexAttrib2fv           = (Opengl_Vertex_Attrib2fv *)wglGetProcAddress("glVertexAttrib2fv");
    glVertexAttrib2s            = (Opengl_Vertex_Attrib2s *)wglGetProcAddress("glVertexAttrib2s");
    glVertexAttrib2sv           = (Opengl_Vertex_Attrib2sv *)wglGetProcAddress("glVertexAttrib2sv");
    glVertexAttrib3d            = (Opengl_Vertex_Attrib3d *)wglGetProcAddress("glVertexAttrib3d");
    glVertexAttrib3dv           = (Opengl_Vertex_Attrib3dv *)wglGetProcAddress("glVertexAttrib3dv");
    glVertexAttrib3f            = (Opengl_Vertex_Attrib3f *)wglGetProcAddress("glVertexAttrib3f");
    glVertexAttrib3fv           = (Opengl_Vertex_Attrib3fv *)wglGetProcAddress("glVertexAttrib3fv");
    glVertexAttrib3s            = (Opengl_Vertex_Attrib3s *)wglGetProcAddress("glVertexAttrib3s");
    glVertexAttrib3sv           = (Opengl_Vertex_Attrib3sv *)wglGetProcAddress("glVertexAttrib3sv");
    glVertexAttrib4Nbv          = (Opengl_Vertex_Attrib4Nbv *)wglGetProcAddress("glVertexAttrib4Nbv");
    glVertexAttrib4Niv          = (Opengl_Vertex_Attrib4Niv *)wglGetProcAddress("glVertexAttrib4Niv");
    glVertexAttrib4Nsv          = (Opengl_Vertex_Attrib4Nsv *)wglGetProcAddress("glVertexAttrib4Nsv");
    glVertexAttrib4Nub          = (Opengl_Vertex_Attrib4Nub *)wglGetProcAddress("glVertexAttrib4Nub");
    glVertexAttrib4Nubv         = (Opengl_Vertex_Attrib4Nubv *)wglGetProcAddress("glVertexAttrib4Nubv");
    glVertexAttrib4Nuiv         = (Opengl_Vertex_Attrib4Nuiv *)wglGetProcAddress("glVertexAttrib4Nuiv");
    glVertexAttrib4Nusv         = (Opengl_Vertex_Attrib4Nusv *)wglGetProcAddress("glVertexAttrib4Nusv");
    glVertexAttrib4bv           = (Opengl_Vertex_Attrib4bv *)wglGetProcAddress("glVertexAttrib4bv");
    glVertexAttrib4d            = (Opengl_Vertex_Attrib4d *)wglGetProcAddress("glVertexAttrib4d");
    glVertexAttrib4dv           = (Opengl_Vertex_Attrib4dv *)wglGetProcAddress("glVertexAttrib4dv");
    glVertexAttrib4f            = (Opengl_Vertex_Attrib4f *)wglGetProcAddress("glVertexAttrib4f");
    glVertexAttrib4fv           = (Opengl_Vertex_Attrib4fv *)wglGetProcAddress("glVertexAttrib4fv");
    glVertexAttrib4iv           = (Opengl_Vertex_Attrib4iv *)wglGetProcAddress("glVertexAttrib4iv");
    glVertexAttrib4s            = (Opengl_Vertex_Attrib4s *)wglGetProcAddress("glVertexAttrib4s");
    glVertexAttrib4sv           = (Opengl_Vertex_Attrib4sv *)wglGetProcAddress("glVertexAttrib4sv");
    glVertexAttrib4ubv          = (Opengl_Vertex_Attrib4ubv *)wglGetProcAddress("glVertexAttrib4ubv");
    glVertexAttrib4uiv          = (Opengl_Vertex_Attrib4uiv *)wglGetProcAddress("glVertexAttrib4uiv");
    glVertexAttrib4usv          = (Opengl_Vertex_Attrib4usv *)wglGetProcAddress("glVertexAttrib4usv");
}
/* }}} */
/* win32_set_pixelformat {{{ */
static void
internal__gfx_set_pixelformat(HDC win_dc) {
    int suggested_pixelformat_index = 0;

    PIXELFORMATDESCRIPTOR desired_pixel_format = {0};
    desired_pixel_format.nSize = sizeof(desired_pixel_format);
    desired_pixel_format.nVersion = 1;
    desired_pixel_format.iPixelType = PFD_TYPE_RGBA;
    desired_pixel_format.dwFlags = PFD_SUPPORT_OPENGL|PFD_DRAW_TO_WINDOW|PFD_DOUBLEBUFFER;
    desired_pixel_format.cColorBits = 32;
    desired_pixel_format.cAlphaBits = 8;
    desired_pixel_format.cDepthBits = 24;
    desired_pixel_format.iLayerType = PFD_MAIN_PLANE;

    suggested_pixelformat_index = ChoosePixelFormat(win_dc, &desired_pixel_format);

    PIXELFORMATDESCRIPTOR suggested_pixelformat;

    DescribePixelFormat(win_dc, suggested_pixelformat_index,
            sizeof(suggested_pixelformat), &suggested_pixelformat);

    SetPixelFormat(win_dc, suggested_pixelformat_index, &suggested_pixelformat);
#if 0
    Error err = get_error(GetLastError());
#endif
}
/* }}} */
/* win32_load_extensions {{{ */
static void
internal__gfx_load_extensions() {
    WNDCLASSA window_class = {0};

    window_class.lpfnWndProc   = DefWindowProcA;
    window_class.hInstance     = GetModuleHandle(0);
    window_class.lpszClassName = "app wgl";

    if(RegisterClassA(&window_class)) {
        HWND window = CreateWindowExA(
                0,
                window_class.lpszClassName,
                "app dummy",
                0,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                0,
                0,
                window_class.hInstance,
                0);

        HDC win_dc = GetDC(window);
        internal__gfx_set_pixelformat(win_dc);
        HGLRC o_dc = wglCreateContext(win_dc);

        if (wglMakeCurrent(win_dc, o_dc)) {
            wglCreateContextAttribs =
                (Wgl_Create_Context_Attribs *)wglGetProcAddress("wglCreateContextAttribsARB");

            wglMakeCurrent(0, 0);
        }

        wglDeleteContext(o_dc);
        ReleaseDC(window, win_dc);
        DestroyWindow(window);
    }
}
/* }}} */
bool
gfx_init(Gfx *gfx, Os *os, Mem_Arena *arena) {
    Gfx_Win32Meta *meta = (Gfx_Win32Meta *)malloc(sizeof(Gfx_Win32Meta));
    meta->window_handle = ((Os_Win32Meta *)os->meta)->window_handle;

    gfx->arena = arena;
    gfx->meta = meta;

    internal__gfx_load_extensions();
    HDC win_dc = GetDC(meta->window_handle);
    internal__gfx_set_pixelformat(win_dc);
    HGLRC ogl_rc = wglCreateContext(win_dc);

    if ( !wglMakeCurrent(win_dc, ogl_rc) ) {
        return false;
    }

    if ( !wglCreateContextAttribs ) {
        return false;
    }

    int32_t win32_opengl_attribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
        WGL_CONTEXT_MINOR_VERSION_ARB, 3,
        WGL_CONTEXT_FLAGS_ARB, WGL_CONTEXT_FORWARD_COMPATIBLE_BIT_ARB,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0,
    };

    wglCreateContextAttribs(win_dc, 0, win32_opengl_attribs);

    // char *ogl_ext = (char *)glGetString(GL_EXTENSIONS);

    os->graphics.device  = (char *)glGetString(GL_RENDERER);
    os->graphics.version = (char *)glGetString(GL_VERSION);
    os->graphics.vendor  = (char *)glGetString(GL_VENDOR);

    internal__gfx_set_opengl_proc_pointer();

    ReleaseDC(meta->window_handle, win_dc);

    return true;
}

void
gfx_rect(Rect rect, Color color) {
    float
        u_min = 0.0f, u_max = 1.0f,
        v_min = 0.0f, v_max = 1.0f;

    Pos min_p = { .x = rect.x,            .y = rect.y             };
    Pos max_p = { .x = rect.x+rect.width, .y = rect.y+rect.height };

    glDisable(GL_TEXTURE_2D);
    glBegin(GL_TRIANGLES);
        glColor4f(color.elems.r, color.elems.g, color.elems.b, color.elems.a);

        glTexCoord2f(u_min, v_max);
        glVertex2f(min_p.x, max_p.y);

        glTexCoord2f(u_max, v_max);
        glVertex2f(max_p.x, max_p.y);

        glTexCoord2f(u_max, v_min);
        glVertex2f(max_p.x, min_p.y);

        glTexCoord2f(u_min, v_max);
        glVertex2f(min_p.x, max_p.y);

        glTexCoord2f(u_max, v_min);
        glVertex2f(max_p.x, min_p.y);

        glTexCoord2f(u_min, v_min);
        glVertex2f(min_p.x, min_p.y);
    glEnd();
    glEnable(GL_TEXTURE_2D);
}

