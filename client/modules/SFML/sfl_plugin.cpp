/**
 * @file sfl_plugin.cpp
 * @brief SFML renderer plugin entry points.
 */

#include "RendererSFML.hpp"

#if defined(_WIN32)
#define EXPORT_FUNC __declspec(dllexport)
#else
#define EXPORT_FUNC
#endif

extern "C" {

/**
 * @brief Create a new SFML renderer instance.
 * @return Newly allocated renderer.
 */
EXPORT_FUNC IRenderer *createRenderer()
{
  return new RendererSFML();
}

/**
 * @brief Destroy an SFML renderer instance.
 * @param r Renderer instance to destroy.
 */
EXPORT_FUNC void destroyRenderer(IRenderer *r)
{
  delete r;
}
}
