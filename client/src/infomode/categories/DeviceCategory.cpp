/**
 * @file DeviceCategory.cpp
 * @brief Implementation of device information category
 */

#include "DeviceCategory.hpp"
#include <sstream>

#ifdef _WIN32
#include <VersionHelpers.h>
#include <comdef.h>
#include <comutil.h>
#include <d3d11.h>
#include <dxgi.h>
#include <wbemidl.h>
#include <windows.h>
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "wbemuuid.lib")
#pragma comment(lib, "comsuppw.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#elif __APPLE__
#include <ApplicationServices/ApplicationServices.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <sys/sysctl.h>
#else
#include <fstream>
#include <sstream>
#include <sys/utsname.h>
#endif

DeviceCategory::DeviceCategory() {}

std::vector<std::string> DeviceCategory::getInfoLines() const
{
  std::vector<std::string> lines;

  // Operating System
  lines.push_back("OS: " + getOperatingSystem());

  // Architecture
  lines.push_back("Architecture: " + getArchitecture());

  // Graphics Information
  lines.push_back("Graphics: " + getGraphicsInfo());

  // Display Resolution
  lines.push_back("Display: " + getDisplayResolution());

  // Memory Information
  lines.push_back("Memory: " + getMemoryInfo());

  // Storage Information
  lines.push_back("Storage: " + getStorageInfo());

  return lines;
}

void DeviceCategory::update(float deltaTime)
{
  // Device info doesn't change frequently, cache it
  if (!m_infoCached) {
    m_osInfo = getOperatingSystem();
    m_architectureInfo = getArchitecture();
    m_graphicsInfo = getGraphicsInfo();
    m_displayInfo = getDisplayResolution();
    m_memoryInfo = getMemoryInfo();
    m_storageInfo = getStorageInfo();
    m_infoCached = true;
  }
}

std::string DeviceCategory::getOperatingSystem() const
{
#ifdef _WIN32
  // Windows version detection
  typedef NTSTATUS(WINAPI * RtlGetVersionPtr)(PRTL_OSVERSIONINFOW);
  HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
  if (hMod) {
    RtlGetVersionPtr fxPtr = (RtlGetVersionPtr)GetProcAddress(hMod, "RtlGetVersion");
    if (fxPtr != nullptr) {
      RTL_OSVERSIONINFOW rovi = {0};
      rovi.dwOSVersionInfoSize = sizeof(rovi);
      if (fxPtr(&rovi) == 0) {
        std::stringstream ss;
        ss << "Windows " << rovi.dwMajorVersion << "." << rovi.dwMinorVersion << " (Build " << rovi.dwBuildNumber
           << ")";
        return ss.str();
      }
    }
  }
  return "Windows (Unknown)";

#elif __APPLE__
  // macOS version detection using sysctl
  char version[256];
  size_t size = sizeof(version);
  if (sysctlbyname("kern.osproductversion", &version, &size, NULL, 0) == 0) {
    std::stringstream ss;
    ss << "macOS " << version;
    return ss.str();
  }
  return "macOS (Unknown)";

#else
  // Linux distribution detection
  std::ifstream file("/etc/os-release");
  std::string line;
  std::string distro = "Linux";

  while (std::getline(file, line)) {
    if (line.find("PRETTY_NAME=") == 0) {
      distro = line.substr(13, line.size() - 14); // Remove quotes
      break;
    }
  }

  // Add kernel version
  struct utsname unameData;
  if (uname(&unameData) == 0) {
    distro += " (" + std::string(unameData.release) + ")";
  }

  return distro;
#endif
}

std::string DeviceCategory::getGraphicsInfo() const
{
#ifdef _WIN32
  // Windows graphics detection using DXGI
  IDXGIFactory *pFactory = nullptr;
  IDXGIAdapter *pAdapter = nullptr;
  HRESULT hr = CreateDXGIFactory(__uuidof(IDXGIFactory), (void **)&pFactory);

  if (SUCCEEDED(hr) && pFactory) {
    hr = pFactory->EnumAdapters(0, &pAdapter);
    if (SUCCEEDED(hr) && pAdapter) {
      DXGI_ADAPTER_DESC desc;
      hr = pAdapter->GetDesc(&desc);
      if (SUCCEEDED(hr)) {
        // Convert WCHAR to std::string
        int size_needed = WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, nullptr, 0, nullptr, nullptr);
        std::string description(size_needed, 0);
        WideCharToMultiByte(CP_UTF8, 0, desc.Description, -1, &description[0], size_needed, nullptr, nullptr);
        description.resize(size_needed - 1); // Remove null terminator

        std::stringstream ss;
        ss << description << " (" << (desc.DedicatedVideoMemory / (1024 * 1024)) << " MB VRAM)";
        pAdapter->Release();
        pFactory->Release();
        return ss.str();
      }
      pAdapter->Release();
    }
    pFactory->Release();
  }
  return "Graphics: Unknown";

#elif __APPLE__
  // macOS graphics detection using IOKit
  io_iterator_t iterator;
  kern_return_t result = IOServiceGetMatchingServices(kIOMainPortDefault, IOServiceMatching("IOPCIDevice"), &iterator);

  if (result == KERN_SUCCESS) {
    io_service_t device;
    while ((device = IOIteratorNext(iterator))) {
      CFMutableDictionaryRef properties = nullptr;
      result = IORegistryEntryCreateCFProperties(device, &properties, kCFAllocatorDefault, kNilOptions);

      if (result == KERN_SUCCESS && properties) {
        CFStringRef classCode = (CFStringRef)CFDictionaryGetValue(properties, CFSTR("class-code"));
        if (classCode && CFEqual(classCode, CFSTR("0x030000"))) { // Display controller
          CFStringRef model = (CFStringRef)CFDictionaryGetValue(properties, CFSTR("model"));
          if (model) {
            char buffer[256];
            if (CFStringGetCString(model, buffer, sizeof(buffer), kCFStringEncodingUTF8)) {
              IOObjectRelease(device);
              CFRelease(properties);
              IOObjectRelease(iterator);
              return std::string(buffer);
            }
          }
        }
        CFRelease(properties);
      }
      IOObjectRelease(device);
    }
    IOObjectRelease(iterator);
  }
  return "Integrated Graphics";

#else
  // Linux graphics detection using lspci
  FILE *pipe = popen("lspci | grep VGA", "r");
  if (pipe) {
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), pipe)) {
      pclose(pipe);
      std::string result = buffer;
      // Remove the PCI address part
      size_t colonPos = result.find(": ");
      if (colonPos != std::string::npos) {
        result = result.substr(colonPos + 2);
      }
      // Remove trailing newline
      result.erase(result.find_last_not_of("\n\r") + 1);
      return result;
    }
    pclose(pipe);
  }
  return "Graphics: Unknown";
#endif
}

std::string DeviceCategory::getDisplayResolution() const
{
#ifdef _WIN32
  // Windows display resolution using EnumDisplaySettings
  DEVMODE dm;
  ZeroMemory(&dm, sizeof(dm));
  dm.dmSize = sizeof(dm);

  if (EnumDisplaySettings(nullptr, ENUM_CURRENT_SETTINGS, &dm)) {
    std::stringstream ss;
    ss << dm.dmPelsWidth << "x" << dm.dmPelsHeight << " @ " << dm.dmDisplayFrequency << "Hz";
    return ss.str();
  }
  return "Resolution: Unknown";

#elif __APPLE__
  // macOS display resolution using CoreGraphics
  CGDirectDisplayID display = CGMainDisplayID();
  size_t width = CGDisplayPixelsWide(display);
  size_t height = CGDisplayPixelsHigh(display);

  std::stringstream ss;
  ss << width << "x" << height;

  // Try to get refresh rate
  CGDisplayModeRef mode = CGDisplayCopyDisplayMode(display);
  if (mode) {
    double refresh = CGDisplayModeGetRefreshRate(mode);
    if (refresh > 0) {
      ss << " @ " << refresh << "Hz";
    }
    CGDisplayModeRelease(mode);
  }

  return ss.str();

#else
  // Linux display resolution using xrandr
  FILE *pipe = popen("xrandr | grep '*' | head -1", "r");
  if (pipe) {
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), pipe)) {
      pclose(pipe);
      std::string result = buffer;
      // Extract resolution (e.g., "1920x1080 60.00*+")
      size_t spacePos = result.find(' ');
      if (spacePos != std::string::npos) {
        std::string res = result.substr(0, spacePos);
        size_t starPos = result.find('*');
        if (starPos != std::string::npos) {
          std::string refresh = result.substr(spacePos + 1, starPos - spacePos - 1);
          return res + " @ " + refresh + "Hz";
        }
        return res;
      }
    }
    pclose(pipe);
  }

  // Fallback: try reading from /sys/class/drm
  std::ifstream drmFile("/sys/class/drm/card0/modes");
  std::string mode;
  if (std::getline(drmFile, mode)) {
    return mode;
  }

  return "Resolution: Unknown";
#endif
}

std::string DeviceCategory::getArchitecture() const
{
#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);

  switch (si.wProcessorArchitecture) {
  case PROCESSOR_ARCHITECTURE_AMD64:
    return "x64";
  case PROCESSOR_ARCHITECTURE_INTEL:
    return "x86";
  case PROCESSOR_ARCHITECTURE_ARM:
    return "ARM";
  case PROCESSOR_ARCHITECTURE_ARM64:
    return "ARM64";
  default:
    return "Unknown";
  }

#elif __APPLE__
  // macOS architecture detection
  cpu_type_t cpuType;
  size_t size = sizeof(cpuType);
  if (sysctlbyname("hw.cputype", &cpuType, &size, NULL, 0) == 0) {
    switch (cpuType) {
    case CPU_TYPE_X86_64:
      return "x64";
    case CPU_TYPE_ARM64:
      return "ARM64";
    case CPU_TYPE_X86:
      return "x86";
    default:
      return "Unknown";
    }
  }
  return "Unknown";

#else
  // Linux architecture detection
  struct utsname unameData;
  if (uname(&unameData) == 0) {
    std::string machine = unameData.machine;
    if (machine == "x86_64")
      return "x64";
    if (machine == "i686" || machine == "i386")
      return "x86";
    if (machine == "aarch64")
      return "ARM64";
    if (machine.find("arm") == 0)
      return "ARM";
    return machine;
  }
  return "Unknown";
#endif
}

std::string DeviceCategory::getMemoryInfo() const
{
#ifdef _WIN32
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  if (GlobalMemoryStatusEx(&memInfo)) {
    std::stringstream ss;
    ss << (memInfo.ullTotalPhys / (1024 * 1024)) << " MB total";
    return ss.str();
  }
  return "Memory: Unknown";

#elif __APPLE__
  // macOS memory detection
  uint64_t memSize;
  size_t size = sizeof(memSize);
  if (sysctlbyname("hw.memsize", &memSize, &size, NULL, 0) == 0) {
    std::stringstream ss;
    ss << (memSize / (1024 * 1024)) << " MB total";
    return ss.str();
  }
  return "Memory: Unknown";

#else
  // Linux memory detection
  std::ifstream memFile("/proc/meminfo");
  std::string line;
  while (std::getline(memFile, line)) {
    if (line.find("MemTotal:") == 0) {
      std::stringstream ss(line.substr(9));
      unsigned long kb;
      ss >> kb;
      std::stringstream result;
      result << (kb / 1024) << " MB total";
      return result.str();
    }
  }
  return "Memory: Unknown";
#endif
}

std::string DeviceCategory::getStorageInfo() const
{
#ifdef _WIN32
  // Windows storage detection
  ULARGE_INTEGER freeBytes, totalBytes, totalFreeBytes;
  if (GetDiskFreeSpaceEx("C:\\", &freeBytes, &totalBytes, &totalFreeBytes)) {
    std::stringstream ss;
    ss << (totalBytes.QuadPart / (1024 * 1024 * 1024)) << " GB total";
    return ss.str();
  }
  return "Storage: Unknown";

#elif __APPLE__
  // macOS storage detection using df
  FILE *pipe = popen("df -h / | tail -1", "r");
  if (pipe) {
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), pipe)) {
      pclose(pipe);
      std::string result = buffer;
      // Parse df output: Filesystem Size Used Avail Capacity Mounted
      std::stringstream ss(result);
      std::string filesystem, size, used, avail, capacity, mounted;
      ss >> filesystem >> size >> used >> avail >> capacity >> mounted;
      return size + " total";
    }
    pclose(pipe);
  }
  return "Storage: Unknown";

#else
  // Linux storage detection using df
  FILE *pipe = popen("df -h / | tail -1", "r");
  if (pipe) {
    char buffer[256];
    if (fgets(buffer, sizeof(buffer), pipe)) {
      pclose(pipe);
      std::string result = buffer;
      // Parse df output
      std::stringstream ss(result);
      std::string filesystem, size, used, avail, capacity, mounted;
      ss >> filesystem >> size >> used >> avail >> capacity >> mounted;
      return size + " total";
    }
    pclose(pipe);
  }
  return "Storage: Unknown";
#endif
}
