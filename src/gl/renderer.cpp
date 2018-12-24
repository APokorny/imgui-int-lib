#include "renderer.h"
#include "imgui.h"

#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#include <GLES3/gl3platform.h>
#else
#include <GL/glew.h>
#endif
#include <unistd.h>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <vector>

namespace {
bool test_shader(GLuint handle, char const* desc) {
    GLint status = 0, log_length = 0;
    glGetShaderiv(handle, GL_COMPILE_STATUS, &status);
    glGetShaderiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if (status == GL_FALSE)
        std::cerr << "ERROR: imgui::gl::Renderer::create_programs : "
                     "failed to compiler "
                  << desc << std::endl;
    if (log_length > 0) {
        std::vector<char> buf;
        buf.resize(log_length + 1);
        glGetShaderInfoLog(handle, log_length, NULL, buf.data());
        std::cerr << buf.data() << std::endl;
    }
    return status == GL_TRUE;
}

bool test_program(GLuint handle, char const* desc) {
    GLint status = 0, log_length = 0;
    glGetProgramiv(handle, GL_LINK_STATUS, &status);
    glGetProgramiv(handle, GL_INFO_LOG_LENGTH, &log_length);
    if (status == GL_FALSE)
        std::cerr << "ERROR: imgui::gl::Renderer::create_programs : "
                     "failed to link "
                  << desc << std::endl;
    if (log_length > 0) {
        std::vector<char> buf;
        buf.resize(log_length + 1);
        glGetProgramInfoLog(handle, log_length, NULL, buf.data());
        std::cerr << buf.data() << std::endl;
    }
    return status == GL_TRUE;
}

struct RestoreEnabled {
    GLenum state;
    GLboolean enabled;
    RestoreEnabled(GLenum s) : state{s}, enabled{glIsEnabled(s)} {}
    ~RestoreEnabled() {
        if (enabled)
            glEnable(state);
        else
            glDisable(state);
    }
};

template <typename T, typename F>
struct RestoreState {
    T value;
    F fun;
    RestoreState(GLenum state, F&& f) : value{}, fun{std::move(f)} {
        glGetIntegerv(state, static_cast<GLint*>(&value));
    }
    ~RestoreState() { fun(value); }
};
template <typename T, size_t N, typename F>
struct RestoreState<T[N], F> {
    T value[N];
    F fun;
    RestoreState(GLenum state, F&& f) : value{}, fun{std::move(f)} {
        glGetIntegerv(state, value);
    }
    ~RestoreState() { fun(value); }
};
template <typename T, typename F>
RestoreState<T, F> restore(GLenum state, F&& f) {
    return {state, std::move(f)};
}

}  // namespace

namespace ig = imgui::gl;

ig::Renderer::Renderer() = default;

ig::Renderer::Renderer(int version_num, std::string const& version)
    : glsl_version{version_num}, glsl_version_text{version} {}

ig::Renderer::~Renderer() {
    cleanup_font_texture();
    cleanup_programs();
}

void ig::Renderer::create_font_texture() {
    // Build texture atlas
    ImGuiIO& io = ImGui::GetIO();
    io.Fonts->AddFontDefault();
    unsigned char* pixels;
    int width, height;
    io.Fonts->GetTexDataAsRGBA32(
        &pixels, &width,
        &height);  // Load as RGBA 32-bits (75% of the memory is wasted, but
                   // default font is so small) because it is more likely to be
                   // compatible with user's existing shaders. If your
                   // ImTextureId represent a higher-level concept than just a
                   // GL texture id, consider calling GetTexDataAsAlpha8()
                   // instead to save on GPU memory.

    auto last_texture = restore<GLint>(GL_TEXTURE_BINDING_2D, [](GLint tex) {
        glBindTexture(GL_TEXTURE_2D, tex);
    });
    glGenTextures(1, &font_texture);
    glBindTexture(GL_TEXTURE_2D, font_texture);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA,
                 GL_UNSIGNED_BYTE, pixels);

    // Store our identifier
    io.Fonts->TexID = reinterpret_cast<ImTextureID>(font_texture);
}

void ig::Renderer::render_imgui_data(ImDrawData& draw_data) {
    ImGuiIO& io = ImGui::GetIO();
    int fb_width = static_cast<int>(draw_data.DisplaySize.x *
                                    io.DisplayFramebufferScale.x);
    int fb_height = static_cast<int>(draw_data.DisplaySize.y *
                                     io.DisplayFramebufferScale.y);
    if (fb_width <= 0 || fb_height <= 0) return;

    draw_data.ScaleClipRects(io.DisplayFramebufferScale);

    // Backup GL state
    auto restore_active_texture =
        restore<GLint>(GL_ACTIVE_TEXTURE, [](GLint last_active_texture) {
            glActiveTexture(last_active_texture);
        });
    glActiveTexture(GL_TEXTURE0);
    auto restore_vertex_buffer = restore<GLint>(
        GL_VERTEX_ARRAY_BINDING,
        [](GLint vertex_array) { glBindVertexArray(vertex_array); });
    auto restore_array_buffer =
        restore<GLint>(GL_ARRAY_BUFFER_BINDING, [](GLint last_array_buffer) {
            glBindBuffer(GL_ARRAY_BUFFER, last_array_buffer);
        });
    auto restore_active_program =
        restore<GLint>(GL_CURRENT_PROGRAM,
                       [](GLint last_program) { glUseProgram(last_program); });
    auto restore_texture_binding = restore<GLint>(
        GL_TEXTURE_BINDING_2D,
        [](GLint last_texture) { glBindTexture(GL_TEXTURE_2D, last_texture); });
#ifdef GL_SAMPLER_BINDING
    auto restore_last_sampler = restore<GLint>(
        GL_SAMPLER_BINDING,
        [](GLint last_sampler) { glBindSampler(0, last_sampler); });
#endif

    GLenum last_blend_src_rgb;
    glGetIntegerv(GL_BLEND_SRC_RGB, (GLint*)&last_blend_src_rgb);
    GLenum last_blend_dst_rgb;
    glGetIntegerv(GL_BLEND_DST_RGB, (GLint*)&last_blend_dst_rgb);
    GLenum last_blend_src_alpha;
    glGetIntegerv(GL_BLEND_SRC_ALPHA, (GLint*)&last_blend_src_alpha);
    GLenum last_blend_dst_alpha;
    glGetIntegerv(GL_BLEND_DST_ALPHA, (GLint*)&last_blend_dst_alpha);
    GLenum last_blend_equation_rgb;
    glGetIntegerv(GL_BLEND_EQUATION_RGB, (GLint*)&last_blend_equation_rgb);
    GLenum last_blend_equation_alpha;
    glGetIntegerv(GL_BLEND_EQUATION_ALPHA, (GLint*)&last_blend_equation_alpha);
#ifdef GL_POLYGON_MODE
    auto restore_poly = restore<GLint[2]>(GL_POLYGON_MODE, [](GLint lm[2]) {
        glPolygonMode(GL_FRONT_AND_BACK, static_cast<GLenum>(lm[0]));
    });
#endif
    auto restore_view_port = restore<GLint[4]>(GL_VIEWPORT, [](GLint vp[4]) {
        glViewport(vp[0], vp[1], vp[2], vp[3]);
    });
    auto restore_scissor = restore<GLint[4]>(GL_SCISSOR_BOX, [](GLint sc[4]) {
        glViewport(sc[0], sc[1], sc[2], sc[3]);
    });

    RestoreEnabled blend(GL_BLEND);
    RestoreEnabled cull(GL_CULL_FACE);
    RestoreEnabled depth(GL_DEPTH_TEST);
    RestoreEnabled scissor(GL_SCISSOR_TEST);

    // Setup render state: alpha-blending enabled, no face culling, no depth
    // testing, scissor enabled, polygon fill
    glEnable(GL_BLEND);
    glBlendEquation(GL_FUNC_ADD);
    glBlendFunc(GL_SRC_ALPHA,
                GL_ONE_MINUS_SRC_ALPHA);  // non premultiplied blending
    glDisable(GL_CULL_FACE);
    glDisable(GL_DEPTH_TEST);
    glEnable(GL_SCISSOR_TEST);
#ifdef GL_POLYGON_MODE
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#endif
    // Setup viewport, orthographic projection matrix
    // Our visible imgui space lies from draw_data.DisplayPps (top left) to
    // draw_data.DisplayPos+data_data->DisplaySize (bottom right). DisplayMin
    // is typically (0,0) for single viewport apps.
    glViewport(0, 0, static_cast<GLsizei>(fb_width),
               static_cast<GLsizei>(fb_height));
    float L = draw_data.DisplayPos.x;
    float R = draw_data.DisplayPos.x + draw_data.DisplaySize.x;
    float T = draw_data.DisplayPos.y;
    float B = draw_data.DisplayPos.y + draw_data.DisplaySize.y;
    const float ortho_projection[4][4] = {
        {2.0f / (R - L), 0.0f, 0.0f, 0.0f},
        {0.0f, 2.0f / (T - B), 0.0f, 0.0f},
        {0.0f, 0.0f, -1.0f, 0.0f},
        {(R + L) / (L - R), (T + B) / (B - T), 0.0f, 1.0f},
    };
    glUseProgram(shader_handle);
    glUniform1i(texture_loc, 0);
    glUniformMatrix4fv(proj_matrix_loc, 1, GL_FALSE, &ortho_projection[0][0]);
#ifdef GL_SAMPLER_BINDING
    glBindSampler(0, 0);  // We use combined texture/sampler state. Applications
                          // using GL 3.3 may set that otherwise.
#endif
    // Recreate the VAO every time
    // (This is to easily allow multiple GL contexts. VAO are not shared among
    // GL contexts, and we don't track creation/deletion of windows so we don't
    // have an obvious key to use to cache them.)
    GLuint vao_handle = 0;
    glGenVertexArrays(1, &vao_handle);
    glBindVertexArray(vao_handle);
    glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
    glEnableVertexAttribArray(position_loc);
    glEnableVertexAttribArray(uv_loc);
    glEnableVertexAttribArray(color_loc);
    glVertexAttribPointer(position_loc, 2, GL_FLOAT, GL_FALSE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, pos));
    glVertexAttribPointer(uv_loc, 2, GL_FLOAT, GL_FALSE, sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, uv));
    glVertexAttribPointer(color_loc, 4, GL_UNSIGNED_BYTE, GL_TRUE,
                          sizeof(ImDrawVert),
                          (GLvoid*)IM_OFFSETOF(ImDrawVert, col));

    // Draw
    ImVec2 pos = draw_data.DisplayPos;
    for (int n = 0; n < draw_data.CmdListsCount; n++) {
        const ImDrawList* cmd_list = draw_data.CmdLists[n];
        const ImDrawIdx* idx_buffer_offset = 0;

        glBindBuffer(GL_ARRAY_BUFFER, vbo_handle);
        glBufferData(GL_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->VtxBuffer.Size * sizeof(ImDrawVert),
                     (const GLvoid*)cmd_list->VtxBuffer.Data, GL_STREAM_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, element_handle);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     (GLsizeiptr)cmd_list->IdxBuffer.Size * sizeof(ImDrawIdx),
                     (const GLvoid*)cmd_list->IdxBuffer.Data, GL_STREAM_DRAW);

        for (int cmd_i = 0; cmd_i < cmd_list->CmdBuffer.Size; cmd_i++) {
            const ImDrawCmd* pcmd = &cmd_list->CmdBuffer[cmd_i];
            if (pcmd->UserCallback) {
                // User callback (registered via ImDrawList::AddCallback)
                pcmd->UserCallback(cmd_list, pcmd);
            } else {
                ImVec4 clip_rect =
                    ImVec4(pcmd->ClipRect.x - pos.x, pcmd->ClipRect.y - pos.y,
                           pcmd->ClipRect.z - pos.x, pcmd->ClipRect.w - pos.y);
                if (clip_rect.x < fb_width && clip_rect.y < fb_height &&
                    clip_rect.z >= 0.0f && clip_rect.w >= 0.0f) {
                    // Apply scissor/clipping rectangle
                    glScissor(static_cast<int>(clip_rect.x),
                              static_cast<int>(fb_height - clip_rect.w),
                              static_cast<int>(clip_rect.z - clip_rect.x),
                              static_cast<int>(clip_rect.w - clip_rect.y));

                    // Bind texture, Draw
                    glBindTexture(
                        GL_TEXTURE_2D,
                        static_cast<GLuint>(
                            reinterpret_cast<ptrdiff_t>(pcmd->TextureId)));
                    glDrawElements(GL_TRIANGLES,
                                   static_cast<GLsizei>(pcmd->ElemCount),
                                   sizeof(ImDrawIdx) == 2 ? GL_UNSIGNED_SHORT
                                                          : GL_UNSIGNED_INT,
                                   idx_buffer_offset);
                }
            }
            idx_buffer_offset += pcmd->ElemCount;
        }
    }
    glDeleteVertexArrays(1, &vao_handle);

    // Restore modified GL state
    glBlendEquationSeparate(last_blend_equation_rgb, last_blend_equation_alpha);
    glBlendFuncSeparate(last_blend_src_rgb, last_blend_dst_rgb,
                        last_blend_src_alpha, last_blend_dst_alpha);
}

void ig::Renderer::acquire_cached_resources() {
    create_programs();
    create_font_texture();
}

void ig::Renderer::create_programs() {
    auto restore_vertex_buffer = restore<GLint>(
        GL_VERTEX_ARRAY_BINDING,
        [](GLint vertex_array) { glBindVertexArray(vertex_array); });
    auto restore_array_buffer =
        restore<GLint>(GL_ARRAY_BUFFER_BINDING, [](GLint array_buffer) {
            glBindBuffer(GL_ARRAY_BUFFER, array_buffer);
        });
    auto restore_texture = restore<GLint>(
        GL_TEXTURE_BINDING_2D,
        [](GLint texture) { glBindTexture(GL_TEXTURE_2D, texture); });

    const GLchar vertex_shader_glsl_120[] =
        "uniform mat4 ProjMtx;\n"
        "attribute vec2 Position;\n"
        "attribute vec2 UV;\n"
        "attribute vec4 Color;\n"
        "varying vec2 Frag_UV;\n"
        "varying vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar vertex_shader_glsl_130[] =
        "uniform mat4 ProjMtx;\n"
        "in vec2 Position;\n"
        "in vec2 UV;\n"
        "in vec4 Color;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar vertex_shader_glsl_300_es[] =
        "precision mediump float;\n"
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "layout (location = 2) in vec4 Color;\n"
        "uniform mat4 ProjMtx;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar vertex_shader_glsl_410_core[] =
        "layout (location = 0) in vec2 Position;\n"
        "layout (location = 1) in vec2 UV;\n"
        "layout (location = 2) in vec4 Color;\n"
        "uniform mat4 ProjMtx;\n"
        "out vec2 Frag_UV;\n"
        "out vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    Frag_UV = UV;\n"
        "    Frag_Color = Color;\n"
        "    gl_Position = ProjMtx * vec4(Position.xy,0,1);\n"
        "}\n";

    const GLchar fragment_shader_glsl_120[] =
        "#ifdef GL_ES\n"
        "    precision mediump float;\n"
        "#endif\n"
        "uniform sampler2D Texture;\n"
        "varying vec2 Frag_UV;\n"
        "varying vec4 Frag_Color;\n"
        "void main()\n"
        "{\n"
        "    gl_FragColor = Frag_Color * texture2D(Texture, Frag_UV.st);\n"
        "}\n";

    const GLchar fragment_shader_glsl_130[] =
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    const GLchar fragment_shader_glsl_300_es[] =
        "precision mediump float;\n"
        "uniform sampler2D Texture;\n"
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "layout (location = 0) out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    const GLchar fragment_shader_glsl_410_core[] =
        "in vec2 Frag_UV;\n"
        "in vec4 Frag_Color;\n"
        "uniform sampler2D Texture;\n"
        "layout (location = 0) out vec4 Out_Color;\n"
        "void main()\n"
        "{\n"
        "    Out_Color = Frag_Color * texture(Texture, Frag_UV.st);\n"
        "}\n";

    // Select shaders matching our GLSL versions
    const GLchar* vertex_shader = NULL;
    const GLchar* fragment_shader = NULL;
    if (glsl_version < 130) {
        vertex_shader = vertex_shader_glsl_120;
        fragment_shader = fragment_shader_glsl_120;
    } else if (glsl_version == 410) {
        vertex_shader = vertex_shader_glsl_410_core;
        fragment_shader = fragment_shader_glsl_410_core;
    } else if (glsl_version == 300) {
        vertex_shader = vertex_shader_glsl_300_es;
        fragment_shader = fragment_shader_glsl_300_es;
    } else {
        vertex_shader = vertex_shader_glsl_130;
        fragment_shader = fragment_shader_glsl_130;
    }

    // Create shaders
    const GLchar* vertex_shader_with_version[2] = {glsl_version_text.c_str(),
                                                   vertex_shader};
    vert_handle = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vert_handle, 2, vertex_shader_with_version, NULL);
    glCompileShader(vert_handle);
    test_shader(vert_handle, "vertex shader");

    const GLchar* fragment_shader_with_version[2] = {glsl_version_text.c_str(),
                                                     fragment_shader};
    frag_handle = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(frag_handle, 2, fragment_shader_with_version, NULL);
    glCompileShader(frag_handle);
    test_shader(frag_handle, "fragment shader");

    shader_handle = glCreateProgram();
    glAttachShader(shader_handle, vert_handle);
    glAttachShader(shader_handle, frag_handle);
    glLinkProgram(shader_handle);
    test_program(shader_handle, "shader program");

    texture_loc = glGetUniformLocation(shader_handle, "Texture");
    proj_matrix_loc = glGetUniformLocation(shader_handle, "ProjMtx");
    position_loc = glGetAttribLocation(shader_handle, "Position");
    uv_loc = glGetAttribLocation(shader_handle, "UV");
    color_loc = glGetAttribLocation(shader_handle, "Color");

    // Create buffers
    glGenBuffers(1, &vbo_handle);
    glGenBuffers(1, &element_handle);
}

void ig::Renderer::finish_frame() {}

void ig::Renderer::cleanup_font_texture() {
    if (font_texture) {
        auto& io = ImGui::GetIO();
        glDeleteTextures(1, &font_texture);
        io.Fonts->TexID = 0;
        font_texture = 0;
    }
}

void ig::Renderer::cleanup_programs() {
    if (vbo_handle) glDeleteBuffers(1, &vbo_handle);
    if (element_handle) glDeleteBuffers(1, &element_handle);
    vbo_handle = element_handle = 0;

    if (shader_handle && vert_handle)
        glDetachShader(shader_handle, vert_handle);
    if (vert_handle) glDeleteShader(vert_handle);
    vert_handle = 0;

    if (shader_handle && frag_handle)
        glDetachShader(shader_handle, frag_handle);
    if (frag_handle) glDeleteShader(frag_handle);
    frag_handle = 0;

    if (shader_handle) glDeleteProgram(shader_handle);
    shader_handle = 0;
}

