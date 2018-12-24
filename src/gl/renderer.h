#pragma once
#ifdef EMSCRIPTEN
#include <GLES3/gl3.h>
#else
#include <GL/glew.h>
#endif
#include <string>
#include "imgui/renderer.h"

struct ImDrawData;

namespace imgui {
namespace gl {
struct Renderer : imgui::Renderer {
   public:
    Renderer();
    Renderer(int glsl_version, std::string const& version);
    void acquire_cached_resources() override;
    void render_imgui_data(ImDrawData& draw_data) override;
    void finish_frame() override;
    ~Renderer();

   private:
    void create_programs();
    void create_font_texture();
    void cleanup_font_texture();
    void cleanup_programs();
    int glsl_version{300};
    std::string glsl_version_text{"#version 300 es\n"};
    GLuint font_texture{0};
    GLuint shader_handle;
    GLuint vert_handle;
    GLuint frag_handle;
    unsigned int vbo_handle{0};
    unsigned int element_handle{0};
    int texture_loc;
    int proj_matrix_loc;
    int position_loc;
    int uv_loc;
    int color_loc;
};

}  // namespace gl
}  // namespace imgui
