#pragma once
#include "app.hpp"
// Only the shader entry points missing from Windows' OpenGL 1.1 header.
class Shader
{
    using CreateShader = GLuint(APIENTRY *)(GLenum);
    using ShaderSource = void(APIENTRY *)(GLuint, GLsizei, const char *const *, const GLint *);
    using One = void(APIENTRY *)(GLuint);
    using Get = void(APIENTRY *)(GLuint, GLenum, GLint *);
    using Log = void(APIENTRY *)(GLuint, GLsizei, GLsizei *, char *);
    using CreateProgram = GLuint(APIENTRY *)();
    using Attach = void(APIENTRY *)(GLuint, GLuint);
    using Location = GLint(APIENTRY *)(GLuint, const char *);
    using Float = void(APIENTRY *)(GLint, GLfloat);
    using Int = void(APIENTRY *)(GLint, GLint);
    using Pair = void(APIENTRY *)(GLint, GLfloat, GLfloat);
    using Triple = void(APIENTRY *)(GLint, GLfloat, GLfloat, GLfloat);
    template <class T> static T load(const char *n)
    {
        auto p = wglGetProcAddress(n);
        if (!p || p == reinterpret_cast<PROC>(1) || p == reinterpret_cast<PROC>(2) ||
            p == reinterpret_cast<PROC>(3) || p == reinterpret_cast<PROC>(-1))
            throw std::runtime_error(std::string("OpenGL shader function unavailable: ") + n);
        return reinterpret_cast<T>(p);
    }
    One use = load<One>("glUseProgram"), del = load<One>("glDeleteProgram");
    Location location = load<Location>("glGetUniformLocation");
    Float setFloat = load<Float>("glUniform1f");
    Int setInt = load<Int>("glUniform1i");
    Pair setPair = load<Pair>("glUniform2f");
    Triple setTriple = load<Triple>("glUniform3f");
    GLuint id = 0;
    static GLuint compile(GLenum type, const std::string &source)
    {
        GLuint s = load<CreateShader>("glCreateShader")(type);
        auto text = source.c_str();
        load<ShaderSource>("glShaderSource")(s, 1, &text, nullptr);
        load<One>("glCompileShader")(s);
        GLint ok;
        load<Get>("glGetShaderiv")(s, 0x8B81, &ok);
        if (!ok)
        {
            char log[8192]{};
            load<Log>("glGetShaderInfoLog")(s, sizeof(log), nullptr, log);
            load<One>("glDeleteShader")(s);
            throw std::runtime_error(log);
        }
        return s;
    }

  public:
    Shader(const std::string &vs, const std::string &fs)
    {
        GLuint v = compile(0x8B31, vs), f = 0;
        try
        {
            f = compile(0x8B30, fs);
        }
        catch (...)
        {
            load<One>("glDeleteShader")(v);
            throw;
        }
        id = load<CreateProgram>("glCreateProgram")();
        auto attach = load<Attach>("glAttachShader");
        attach(id, v);
        attach(id, f);
        load<One>("glLinkProgram")(id);
        load<One>("glDeleteShader")(v);
        load<One>("glDeleteShader")(f);
        GLint ok;
        load<Get>("glGetProgramiv")(id, 0x8B82, &ok);
        if (!ok)
        {
            char log[8192]{};
            load<Log>("glGetProgramInfoLog")(id, sizeof(log), nullptr, log);
            del(id);
            throw std::runtime_error(log);
        }
    }
    ~Shader()
    {
        del(id);
    }
    Shader(const Shader &) = delete;
    Shader &operator=(const Shader &) = delete;
    void bind()
    {
        use(id);
    }
    void stop()
    {
        use(0);
    }
    void uniform(const char *n, float v)
    {
        setFloat(location(id, n), v);
    }
    void integer(const char *n, int v)
    {
        setInt(location(id, n), v);
    }
    void pair(const char *n, float x, float y)
    {
        setPair(location(id, n), x, y);
    }
    void triple(const char *n, Vec3 v)
    {
        setTriple(location(id, n), float(v.x), float(v.y), float(v.z));
    }
};
inline const char *phongVertex = R"GLSL(#version 120
varying vec3 P,N;
void main(){P=vec3(gl_ModelViewMatrix*gl_Vertex);N=gl_NormalMatrix*gl_Normal;gl_Position=ftransform();gl_TexCoord[0]=gl_MultiTexCoord0;}
)GLSL";
inline const char *phongFragment = R"GLSL(#version 120
varying vec3 P,N;
uniform vec3 lightPosition;
uniform sampler2D colorMap;
uniform int textured;
void main(){vec3 n=normalize(N);if(!gl_FrontFacing)n=-n;vec3 l=normalize(lightPosition-P);vec3 v=normalize(-P);
float diffuse=max(dot(n,l),0.0);float spec=diffuse>0.0?pow(max(dot(reflect(-l,n),v),0.0),gl_FrontMaterial.shininess):0.0;
vec3 tex=textured==1?texture2D(colorMap,gl_TexCoord[0].xy).rgb:vec3(1.0);
vec3 c=gl_FrontMaterial.ambient.rgb*0.3+gl_FrontMaterial.diffuse.rgb*tex*diffuse+gl_FrontMaterial.specular.rgb*spec;
gl_FragColor=vec4(c,1.0);}
)GLSL";
inline Vec3 eyePosition(Vec3 world)
{
    GLdouble m[16];
    glGetDoublev(GL_MODELVIEW_MATRIX, m);
    return {m[0] * world.x + m[4] * world.y + m[8] * world.z + m[12],
            m[1] * world.x + m[5] * world.y + m[9] * world.z + m[13],
            m[2] * world.x + m[6] * world.y + m[10] * world.z + m[14]};
}
