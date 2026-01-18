/**
 * @file ModuleLoader.hpp
 * @brief Cross-platform dynamic module loader.
 */

#pragma once

#include <stdexcept>
#include <string>

#if defined(_WIN32)
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
using LibHandle = HMODULE;
#define LOAD_LIB(path) LoadLibraryA(path)
#define LOAD_SYM(handle, name) GetProcAddress(handle, name)
#define CLOSE_LIB(handle) FreeLibrary(handle)
#else
#include <dlfcn.h>
using LibHandle = void *;
#define LOAD_LIB(path) dlopen(path, RTLD_NOW)
#define LOAD_SYM(handle, name) dlsym(handle, name)
#define CLOSE_LIB(handle) dlclose(handle)
#endif

/**
 * @class Module
 * @brief RAII wrapper for dynamically loaded modules.
 * @tparam T Interface type exposed by the module.
 */
template <typename T>
class Module
{
public:
  using CreateFn = T *(*)();
  using DestroyFn = void (*)(T *);

  /**
   * @brief Load a module and resolve its create/destroy symbols.
   * @param path Shared library path.
   * @param createName Symbol name for factory creation.
   * @param destroyName Symbol name for factory destruction.
   */
  Module(const std::string &path, const std::string &createName, const std::string &destroyName)
      : handle(nullptr), createFn(nullptr), destroyFn(nullptr)
  {
    handle = LOAD_LIB(path.c_str());
    if (handle == nullptr) {
      throw std::runtime_error("Failed to load module: " + path);
    }

    createFn = reinterpret_cast<CreateFn>(LOAD_SYM(handle, createName.c_str()));
    if (createFn == nullptr) {
      CLOSE_LIB(handle);
      throw std::runtime_error("Failed to load symbol: " + createName);
    }

    destroyFn = reinterpret_cast<DestroyFn>(LOAD_SYM(handle, destroyName.c_str()));
    if (destroyFn == nullptr) {
      CLOSE_LIB(handle);
      throw std::runtime_error("Failed to load symbol: " + destroyName);
    }
  }

  /** @brief Unload the module. */
  ~Module()
  {
    if (handle != nullptr) {
      CLOSE_LIB(handle);
    }
  }

  /** @brief Create an instance from the module factory. */
  T *create() { return createFn(); }
  /** @brief Destroy an instance created by the module factory. */
  void destroy(T *instance) { destroyFn(instance); }

private:
  LibHandle handle;
  CreateFn createFn;
  DestroyFn destroyFn;
};
