#pragma once

#include "gfx/common.h"
#include "gfx/gl/buffer.h"
#include "gfx/gl/program.h"
#include "gfx/gl/sampler.h"
#include "gfx/gl/texture.h"
#include "gfx/gl/vertex_array.h"

#include <GL/glew.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct GFX_FBO_Renderer {
    GLuint fbo;
    GLuint rbo;

    GLuint filter;

    GFX_GL_VertexArray vertex_array;
    GFX_GL_Buffer buffer;
    GFX_GL_Texture texture;
    GFX_GL_Sampler sampler;
    GFX_GL_Program program;
} GFX_FBO_Renderer;

void GFX_FBO_Renderer_Init(GFX_FBO_Renderer *renderer);
void GFX_FBO_Renderer_Close(GFX_FBO_Renderer *renderer);
void GFX_FBO_Renderer_Render(GFX_FBO_Renderer *renderer);
void GFX_FBO_Renderer_SetFilter(
    GFX_FBO_Renderer *renderer, GFX_TEXTURE_FILTER filter);
void GFX_FBO_Renderer_Bind(const GFX_FBO_Renderer *renderer);
void GFX_FBO_Renderer_Unbind(const GFX_FBO_Renderer *renderer);
