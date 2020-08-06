#define GL_EXTERN
#include "gl-link.h"

#define GL_LINK(x)                                                             \
do {                                                                           \
  if (!(x = reinterpret_cast<decltype(x)>(wglGetProcAddress(#x)))) {           \
    return false;                                                              \
  }                                                                            \
} while (false)

bool linkWgl()
{
  GL_LINK(wglChoosePixelFormatARB);
  GL_LINK(wglCreateContextAttribsARB);
  return true;
}

bool linkGl()
{
  GL_LINK(wglChoosePixelFormatARB);
  GL_LINK(wglCreateContextAttribsARB);
  GL_LINK(glGenBuffers);
  GL_LINK(glBindBuffer);
  GL_LINK(glBufferData);
  GL_LINK(glGenVertexArrays);
  GL_LINK(glBindVertexArray);
  GL_LINK(glEnableVertexAttribArray);
  GL_LINK(glDisableVertexAttribArray);
  GL_LINK(glVertexAttribPointer);
  GL_LINK(glCreateProgram);
  GL_LINK(glCreateShader);
  GL_LINK(glShaderSource);
  GL_LINK(glCompileShader);
  GL_LINK(glGetShaderiv);
  GL_LINK(glGetShaderInfoLog);
  GL_LINK(glAttachShader);
  GL_LINK(glLinkProgram);
  GL_LINK(glUseProgram);
  GL_LINK(glGetUniformLocation);
  GL_LINK(glUniformMatrix4fv);
  GL_LINK(glGenerateMipmap);
  GL_LINK(glActiveTexture);
  GL_LINK(glGenQueries);
  GL_LINK(glBeginQuery);
  GL_LINK(glEndQuery);
  GL_LINK(glGetQueryObjectuiv);
  GL_LINK(glGenFramebuffers);
  GL_LINK(glBindFramebuffer);
  GL_LINK(glFramebufferTexture2D);
  GL_LINK(glFramebufferRenderbuffer);
  GL_LINK(glGenRenderbuffers);
  GL_LINK(glBindRenderbuffer);
  GL_LINK(glRenderbufferStorage);
  GL_LINK(glUniform2f);
  return true;
}
