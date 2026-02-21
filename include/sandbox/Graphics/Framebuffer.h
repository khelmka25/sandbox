#pragma once

#include <glad/glad.h>

namespace sb::gfx {
struct Framebuffer {
  unsigned width, height;

  unsigned fbo;
  unsigned rbo;
  unsigned cbo;
};

void createFramebuffer(unsigned width, unsigned height, Framebuffer* framebuffer) {
  glGenFramebuffers(1, &framebuffer->fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->fbo);

  glGenTextures(1, &framebuffer->cbo);
  glBindTexture(GL_TEXTURE_2D, framebuffer->cbo);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, framebuffer->cbo, 0);

  glGenRenderbuffers(1, &framebuffer->rbo);
  glBindRenderbuffer(GL_RENDERBUFFER, framebuffer->rbo);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, framebuffer->rbo);

  glBindFramebuffer(GL_FRAMEBUFFER, 0);
}
}  // namespace sb::gfx