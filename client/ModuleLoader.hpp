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

template <typename T>
class Module
{
public:
  using CreateFn = T *(*)();
  using DestroyFn = void (*)(T *);

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

  ~Module()
  {
    if (handle != nullptr) {
      CLOSE_LIB(handle);
    }
  }

  T *create() { return createFn(); }
  void destroy(T *instance) { destroyFn(instance); }

private:
  LibHandle handle;
  CreateFn createFn;
  DestroyFn destroyFn;
};
