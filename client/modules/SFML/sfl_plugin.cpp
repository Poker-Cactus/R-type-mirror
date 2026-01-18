#include "RendererSFML.hpp"

#if defined(_WIN32)
#define EXPORT_FUNC __declspec(dllexport)
#else
#define EXPORT_FUNC
#endif

extern "C" {

EXPORT_FUNC IRenderer *createRenderer()
{
  return new RendererSFML();
}

EXPORT_FUNC void destroyRenderer(IRenderer *r)
{
  delete r;
}
}
