/**
 * @file CpuRamCategory.cpp
 * @brief Implementation of CPU and RAM information category
 */

#include "CpuRamCategory.hpp"
#include <iostream>
#include <iomanip>
#include <thread>
#include <chrono>

#ifdef _WIN32
#include <pdh.h>
#include <pdhmsg.h>
#pragma comment(lib, "pdh.lib")
#endif

CpuRamCategory::CpuRamCategory()
{
}

std::vector<std::string> CpuRamCategory::getInfoLines() const
{
  std::vector<std::string> lines;

  // CPU Information
  int cpuCores = getCpuCores();
  std::stringstream cpuText;
  cpuText << "CPU: " << std::fixed << std::setprecision(1) << m_lastCpuUsage << "% (" << cpuCores << " cores)";
  lines.push_back(cpuText.str());

  // RAM Information
  auto [ramUsed, ramTotal] = getRamUsage();
  double ramUsedGB = static_cast<double>(ramUsed) / 1024.0;
  double ramTotalGB = static_cast<double>(ramTotal) / 1024.0;

  std::stringstream ramText;
  ramText << std::fixed << std::setprecision(1);
  ramText << "RAM: " << ramUsedGB << "/" << ramTotalGB << " GB";
  lines.push_back(ramText.str());

  return lines;
}

void CpuRamCategory::update(float deltaTime)
{
  m_sampleCount++;

  // Update usage periodically to avoid excessive system calls
  if (m_sampleCount >= SAMPLES_PER_UPDATE) {
    m_lastCpuUsage = getCpuUsage();
    m_sampleCount = 0;
  }
}

float CpuRamCategory::getCpuUsage()
{
#ifdef _WIN32
  // Windows implementation using PDH
  static PDH_HQUERY query = nullptr;
  static PDH_HCOUNTER counter = nullptr;
  static bool initialized = false;

  if (!initialized) {
    PdhOpenQuery(nullptr, 0, &query);
    PdhAddEnglishCounter(query, "\\Processor(_Total)\\% Processor Time", 0, &counter);
    PdhCollectQueryData(query);
    initialized = true;
    // Wait a bit for first reading
    std::this_thread::sleep_for(std::chrono::milliseconds(100));
  }

  PDH_FMT_COUNTERVALUE value;
  PdhCollectQueryData(query);
  PdhGetFormattedCounterValue(counter, PDH_FMT_DOUBLE, nullptr, &value);
  return static_cast<float>(value.doubleValue);

#elif __APPLE__
  // macOS implementation using host_statistics
  static unsigned long long prev_idle = 0;
  static unsigned long long prev_total = 0;
  static bool initialized = false;

  host_cpu_load_info_data_t cpuinfo;
  mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
  kern_return_t result = host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, (host_info_t)&cpuinfo, &count);

  if (result == KERN_SUCCESS) {
    unsigned long long current_idle = cpuinfo.cpu_ticks[CPU_STATE_IDLE];
    unsigned long long current_total = 0;

    for (int i = 0; i < 4; i++) {
      current_total += cpuinfo.cpu_ticks[i];
    }

    if (initialized && prev_total > 0 && current_total > prev_total) {
      unsigned long long idle_diff = current_idle - prev_idle;
      unsigned long long total_diff = current_total - prev_total;

      if (total_diff > 0) {
        float usage = 100.0f * (1.0f - (float)idle_diff / (float)total_diff);
        if (usage >= 0.0f && usage <= 100.0f) {
          prev_idle = current_idle;
          prev_total = current_total;
          return usage;
        }
      }
    } else {
      initialized = true;
      prev_idle = current_idle;
      prev_total = current_total;
    }
  } else {
    // Fallback to load average if host_statistics fails
    struct loadavg load_avg;
    size_t size = sizeof(load_avg);

    if (sysctlbyname("vm.loadavg", &load_avg, &size, nullptr, 0) == 0) {
      int cores = getCpuCores();
      if (cores > 0) {
        // Load average fallback
        float load_percent = (load_avg.ldavg[0] / (float)cores) * 100.0f;
        if (load_percent > 100.0f) load_percent = 100.0f;
        if (load_percent < 0.0f) load_percent = 0.0f;

        return load_percent;
      }
    }
  }

  return 0.0f;
#else
  // Linux implementation using /proc/stat
  static unsigned long long prevTotal = 0, prevIdle = 0;
  static auto prevTime = std::chrono::steady_clock::now();
  static bool firstCall = true;

  std::ifstream file("/proc/stat");
  std::string line;

  if (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string cpu;
    unsigned long long user, nice, system, idle, iowait, irq, softirq, steal;

    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq >> steal;

    unsigned long long total = user + nice + system + idle + iowait + irq + softirq + steal;
    unsigned long long idle_time = idle + iowait;

    auto currentTime = std::chrono::steady_clock::now();
    auto timeDiff = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime - prevTime).count();

    if (!firstCall && timeDiff > 100) { // Only calculate if enough time has passed
      unsigned long long totalDiff = total - prevTotal;
      unsigned long long idleDiff = idle_time - prevIdle;

      if (totalDiff == 0) {
        prevTotal = total;
        prevIdle = idle_time;
        prevTime = currentTime;
        return 0.0f;
      }

      float usage = 100.0f * (totalDiff - idleDiff) / totalDiff;
      prevTotal = total;
      prevIdle = idle_time;
      prevTime = currentTime;
      return usage;
    }

    prevTotal = total;
    prevIdle = idle_time;
    prevTime = currentTime;
    firstCall = false;
  }

  return 0.0f;
#endif
}

int CpuRamCategory::getCpuCores() const
{
#ifdef _WIN32
  SYSTEM_INFO sysInfo;
  GetSystemInfo(&sysInfo);
  return sysInfo.dwNumberOfProcessors;

#elif __APPLE__
  int cores = 0;
  size_t size = sizeof(cores);
  if (sysctlbyname("hw.ncpu", &cores, &size, nullptr, 0) == 0) {
    return cores;
  }
  return 1; // fallback

#else
  return sysconf(_SC_NPROCESSORS_ONLN);
#endif
}

std::pair<uint64_t, uint64_t> CpuRamCategory::getRamUsage() const
{
#ifdef _WIN32
  MEMORYSTATUSEX memInfo;
  memInfo.dwLength = sizeof(MEMORYSTATUSEX);
  GlobalMemoryStatusEx(&memInfo);

  uint64_t totalPhys = memInfo.ullTotalPhys / (1024 * 1024); // MB
  uint64_t availPhys = memInfo.ullAvailPhys / (1024 * 1024); // MB
  uint64_t usedPhys = totalPhys - availPhys;

  return {usedPhys, totalPhys};

#elif __APPLE__
  // macOS implementation using sysctl
  uint64_t totalMemory = 0;
  uint64_t usedMemory = 0;
  size_t size = sizeof(totalMemory);

  // Get total physical memory
  if (sysctlbyname("hw.memsize", &totalMemory, &size, nullptr, 0) != 0) {
    return {0, 0};
  }

  // Get used memory (total - available)
  vm_size_t page_size;
  mach_port_t mach_port = mach_host_self();
  vm_statistics64_data_t vm_stats;
  mach_msg_type_number_t count = sizeof(vm_stats) / sizeof(natural_t);

  if (host_page_size(mach_port, &page_size) != KERN_SUCCESS ||
      host_statistics64(mach_port, HOST_VM_INFO64, (host_info64_t)&vm_stats, &count) != KERN_SUCCESS) {
    return {0, 0};
  }

  // Calculate free memory in bytes
  uint64_t freeMemory = vm_stats.free_count * page_size;
  uint64_t inactiveMemory = vm_stats.inactive_count * page_size;
  uint64_t availableMemory = freeMemory + inactiveMemory;

  usedMemory = totalMemory - availableMemory;

  // Convert to MB
  totalMemory /= (1024 * 1024);
  usedMemory /= (1024 * 1024);

  return {usedMemory, totalMemory};

#else
  // Linux implementation using /proc/meminfo
  std::ifstream file("/proc/meminfo");
  std::string line;
  uint64_t total = 0, available = 0;

  while (std::getline(file, line)) {
    std::istringstream iss(line);
    std::string key;
    uint64_t value;
    std::string unit;

    iss >> key >> value >> unit;

    if (key == "MemTotal:") {
      total = value / 1024; // Convert to MB
    } else if (key == "MemAvailable:") {
      available = value / 1024; // Convert to MB
      break;
    }
  }

  uint64_t used = total - available;
  return {used, total};
#endif
}
