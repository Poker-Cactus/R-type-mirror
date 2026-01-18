/**
 * @file sdl_plugin.cpp
 * @brief SDL2 renderer plugin entry points.
 */

#include "RendererSDL2.hpp"

#if defined(_WIN32)
#define EXPORT_FUNC __declspec(dllexport)
#else
#define EXPORT_FUNC
#endif

extern "C" {

/**
 * @brief Create a new SDL2 renderer instance.
 * @return Newly allocated renderer.
 */
EXPORT_FUNC IRenderer *createRenderer()
{
  return new RendererSDL2();
}

/**
 * @brief Destroy an SDL2 renderer instance.
 * @param r Renderer instance to destroy.
 */
EXPORT_FUNC void destroyRenderer(IRenderer *r)
{
  delete r;
}
}
