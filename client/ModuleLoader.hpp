#pragma once

#include <stdexcept>
#include <string>

#if defined(_WIN32)
#include <windows.h>
using LibHandle = HMODULE;
#define LOAD_LIB(path) uLoadLibraryA(path)
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
    if (!handle)
      throw std::runtime_error("Failed to load module: " + path);

    createFn = reinterpret_cast<CreateFn>(LOAD_SYM(handle, createName.c_str()));
    if (!createFn) {
      CLOSE_LIB(handle);
      throw std::runtime_error("Failed to load symbol: " + createName);
    }

    destroyFn = reinterpret_cast<DestroyFn>(LOAD_SYM(handle, destroyName.c_str()));
    if (!destroyFn) {
      CLOSE_LIB(handle);
      throw std::runtime_error("Failed to load symbol: " + destroyName);
    }
  }

  ~Module()
  {
    if (handle) {
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
