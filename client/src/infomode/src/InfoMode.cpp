/**
 * @file InfoMode.cpp
 * @brief Implementation of R-Type debug overlay with cross-platform system monitoring
 *
 * Provides real-time CPU/RAM usage tracking using platform-specific APIs:
 * - macOS: mach kernel APIs (host_statistics, vm_statistics64)
 * - Linux: /proc filesystem (/proc/stat, /proc/meminfo)
 */

#include "../include/InfoMode.hpp"
#include "../../../include/Settings.hpp"
#include "../../../interface/KeyCodes.hpp"
#include <fstream>
#include <iomanip>
#include <sstream>

#ifdef __APPLE__
#include <mach/mach.h>
#include <mach/mach_host.h>
#include <mach/processor_info.h>
#include <sys/sysctl.h>
#elif __linux__
#include <sys/sysinfo.h>
#include <unistd.h>
#endif

namespace rtype
{

namespace
{
/// R-Type arcade color palette
constexpr Color CYAN = {0, 255, 255, 255}; ///< Section headers
constexpr Color WHITE = {255, 255, 255, 255}; ///< Normal values
constexpr Color YELLOW = {255, 255, 0, 255}; ///< Labels
constexpr Color GREEN = {0, 255, 0, 255}; ///< Positive status
constexpr Color RED = {255, 80, 80, 255}; ///< Warning/error
constexpr Color BG = {10, 10, 35, 220}; ///< Panel background
constexpr Color BAR_BG = {30, 30, 60, 255}; ///< Progress bar background
constexpr Color BAR_GREEN = {0, 200, 100, 255}; ///< < 50% usage
constexpr Color BAR_YELLOW = {255, 200, 0, 255}; ///< 50-80% usage
constexpr Color BAR_RED = {255, 60, 60, 255}; ///< > 80% usage

/** @brief Get color based on usage percentage (green < 50% < yellow < 80% < red) */
Color getBarColor(float percent)
{
  if (percent < 50.0f)
    return BAR_GREEN;
  if (percent < 80.0f)
    return BAR_YELLOW;
  return BAR_RED;
}

/** @brief Format bytes to human-readable string (KB/MB/GB) */
std::string formatBytes(uint64_t bytes)
{
  char buffer[32];
  if (bytes >= 1024ULL * 1024 * 1024) {
    snprintf(buffer, sizeof(buffer), "%.1f GB", bytes / (1024.0 * 1024 * 1024));
  } else if (bytes >= 1024ULL * 1024) {
    snprintf(buffer, sizeof(buffer), "%.0f MB", bytes / (1024.0 * 1024));
  } else {
    snprintf(buffer, sizeof(buffer), "%llu KB", bytes / 1024);
  }
  return buffer;
}
} // namespace

InfoMode::InfoMode(std::shared_ptr<IRenderer> renderer, std::shared_ptr<void> hudFont, Settings &settings)
    : m_renderer(std::move(renderer)), m_hudFont(std::move(hudFont)), m_settings(settings), m_isActive(false),
      m_lastSystemUpdate(std::chrono::steady_clock::now())
{
  m_settings.showInfoMode = false;
  initStaticSystemInfo();
  updateDynamicSystemInfo();
}

void InfoMode::initStaticSystemInfo()
{
#ifdef __APPLE__
  m_osName = "macOS";

  // CPU name
  char cpuBrand[256];
  size_t size = sizeof(cpuBrand);
  if (sysctlbyname("machdep.cpu.brand_string", &cpuBrand, &size, nullptr, 0) == 0) {
    m_cpuName.assign(cpuBrand, size > 22 ? 19 : size);
    if (size > 22)
      m_cpuName += "...";
  } else {
    m_cpuName = "Apple Silicon";
  }

  // CPU cores
  int cores;
  size = sizeof(cores);
  if (sysctlbyname("hw.ncpu", &cores, &size, nullptr, 0) == 0) {
    m_cpuCores = cores;
  }

  // Total RAM
  int64_t memSize;
  size = sizeof(memSize);
  if (sysctlbyname("hw.memsize", &memSize, &size, nullptr, 0) == 0) {
    m_totalRamBytes = static_cast<uint64_t>(memSize);
  }

#elif __linux__
  m_osName = "Linux";

  // CPU name from /proc/cpuinfo
  std::ifstream cpuinfo("/proc/cpuinfo");
  std::string line;
  line.reserve(128);
  while (std::getline(cpuinfo, line)) {
    if (line.compare(0, 10, "model name") == 0) {
      size_t pos = line.find(':');
      if (pos != std::string::npos && pos + 2 < line.size()) {
        size_t len = line.size() - pos - 2;
        m_cpuName.assign(line, pos + 2, len > 22 ? 19 : len);
        if (len > 22)
          m_cpuName += "...";
      }
      break;
    }
  }
  if (m_cpuName.empty())
    m_cpuName = "Unknown CPU";

  // CPU cores
  m_cpuCores = static_cast<int>(sysconf(_SC_NPROCESSORS_ONLN));

  // Total RAM
  struct sysinfo info;
  if (sysinfo(&info) == 0) {
    m_totalRamBytes = info.totalram * info.mem_unit;
  }
#else
  m_osName = "Unknown";
  m_cpuName = "Unknown";
  m_cpuCores = 1;
  m_totalRamBytes = 0;
#endif
}

void InfoMode::updateDynamicSystemInfo()
{
#ifdef __APPLE__
  // CPU Usage via host_statistics
  host_cpu_load_info_data_t cpuInfo;
  mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;

  if (host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO, reinterpret_cast<host_info_t>(&cpuInfo), &count) ==
      KERN_SUCCESS) {
    uint64_t idle = cpuInfo.cpu_ticks[CPU_STATE_IDLE];
    uint64_t total = idle + cpuInfo.cpu_ticks[CPU_STATE_USER] + cpuInfo.cpu_ticks[CPU_STATE_SYSTEM] +
      cpuInfo.cpu_ticks[CPU_STATE_NICE];

    if (m_prevTotalTime > 0) {
      uint64_t idleDiff = idle - m_prevIdleTime;
      uint64_t totalDiff = total - m_prevTotalTime;
      if (totalDiff > 0) {
        m_cpuUsage = 100.0f * (1.0f - static_cast<float>(idleDiff) / static_cast<float>(totalDiff));
      }
    }
    m_prevIdleTime = idle;
    m_prevTotalTime = total;
  }

  // RAM Usage via vm_statistics64
  vm_statistics64_data_t vmStats;
  count = HOST_VM_INFO64_COUNT;

  if (host_statistics64(mach_host_self(), HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vmStats), &count) ==
      KERN_SUCCESS) {
    vm_size_t pageSize;
    host_page_size(mach_host_self(), &pageSize);

    uint64_t active = vmStats.active_count * pageSize;
    uint64_t wired = vmStats.wire_count * pageSize;
    uint64_t compressed = vmStats.compressor_page_count * pageSize;
    m_usedRamBytes = active + wired + compressed;
  }

#elif __linux__
  // CPU Usage from /proc/stat
  std::ifstream statFile("/proc/stat");
  std::string line;
  if (std::getline(statFile, line)) {
    std::istringstream iss(line);
    std::string cpu;
    uint64_t user, nice, system, idle, iowait, irq, softirq;
    iss >> cpu >> user >> nice >> system >> idle >> iowait >> irq >> softirq;

    uint64_t idleTime = idle + iowait;
    uint64_t totalTime = user + nice + system + idle + iowait + irq + softirq;

    if (m_prevTotalTime > 0) {
      uint64_t idleDiff = idleTime - m_prevIdleTime;
      uint64_t totalDiff = totalTime - m_prevTotalTime;
      if (totalDiff > 0) {
        m_cpuUsage = 100.0f * (1.0f - static_cast<float>(idleDiff) / static_cast<float>(totalDiff));
      }
    }
    m_prevIdleTime = idleTime;
    m_prevTotalTime = totalTime;
  }

  // RAM Usage from /proc/meminfo
  std::ifstream meminfo("/proc/meminfo");
  uint64_t memTotal = 0, memAvailable = 0;
  while (std::getline(meminfo, line)) {
    if (line.find("MemTotal:") == 0) {
      std::istringstream iss(line.substr(9));
      iss >> memTotal;
      memTotal *= 1024; // Convert from KB to bytes
    } else if (line.find("MemAvailable:") == 0) {
      std::istringstream iss(line.substr(13));
      iss >> memAvailable;
      memAvailable *= 1024;
    }
  }
  if (memTotal > 0) {
    m_usedRamBytes = memTotal - memAvailable;
  }
#endif
}

void InfoMode::processInput()
{
  if (!m_renderer)
    return;

  bool keyPressed = m_renderer->isKeyPressed(KeyCode::KEY_I);
  if (keyPressed && !m_keyWasPressed) {
    m_isActive = !m_isActive;
    m_settings.showInfoMode = m_isActive;
  }
  m_keyWasPressed = keyPressed;
}

void InfoMode::update()
{
  // Sync with settings
  m_isActive = m_settings.showInfoMode;

  if (!m_isActive)
    return;

  // Only update system info if needed and periodically
  if (m_settings.showCPUUsage || m_settings.showRAMUsage) {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - m_lastSystemUpdate).count();

    if (elapsed >= SYSTEM_UPDATE_MS) {
      updateDynamicSystemInfo();
      m_lastSystemUpdate = now;
    }
  }
}

void InfoMode::render()
{
  if (!m_isActive || !m_renderer || !m_hudFont)
    return;

  calculateLayout();
  drawPanel();
}

void InfoMode::calculateLayout()
{
  const int screenW = m_renderer->getWindowWidth();
  const int screenH = m_renderer->getWindowHeight();

  // Panel plus large que haut (layout horizontal) - clamp in one operation
  m_panelWidth = std::max(450, std::min(650, static_cast<int>(screenW * 0.35f)));
  m_padding = std::max(10, static_cast<int>(screenH * PADDING_RATIO));
  m_lineHeight = std::max(16, std::min(26, static_cast<int>(screenH * LINE_HEIGHT_RATIO)));

  m_sectionGap = m_lineHeight >> 1; // Faster division by 2
  m_labelWidth = 90;
  m_barHeight = (m_lineHeight << 1) / 3; // Faster multiplication by 2

  // Position panel at top-right with margin
  const int margin = std::max(10, static_cast<int>(screenW * PANEL_MARGIN_RATIO));
  m_panelX = screenW - m_panelWidth - margin;
  m_panelY = margin;

  // Calculate height - count rows only once
  int rows = 3; // Title + 2 for headers
  if (m_settings.showCPUUsage || m_settings.showRAMUsage) {
    rows += 2 + (m_settings.showCPUUsage ? 2 : 0) + (m_settings.showRAMUsage ? 2 : 0);
  }
  rows += 2; // Separator

  m_panelHeight = rows * m_lineHeight + (m_padding << 1);
}

void InfoMode::drawPanel()
{
  void *font = m_hudFont.get();
  const int padding2 = m_padding << 1;
  const int panelLeft = m_panelX - m_padding;
  int currentY = m_panelY + m_padding;

  // Background with border - single pass
  m_renderer->drawRect(panelLeft, m_panelY, m_panelWidth + padding2, m_panelHeight, BG);
  m_renderer->drawRectOutline(panelLeft, m_panelY, m_panelWidth + padding2, m_panelHeight, CYAN);

  // Title centered
  m_renderer->drawText(font, ">>> R-TYPE DEBUG <<<", m_panelX + ((m_panelWidth - 180) >> 1), currentY, CYAN);
  currentY += m_lineHeight + m_sectionGap;

  // Layout: 2 columns for compact display
  const int leftColX = m_panelX;
  const int rightColX = m_panelX + (m_panelWidth >> 1);
  int col1Y = currentY;
  int col2Y = currentY;

  // === LEFT COLUMN: SYSTEM ===
  if (m_settings.showCPUUsage || m_settings.showRAMUsage) {
    drawSection("SYSTEM", leftColX, col1Y);
    drawLine("OS", m_osName, leftColX, col1Y);
    drawLine("CPU", m_cpuName, leftColX, col1Y);

    if (m_settings.showCPUUsage) {
      char coresBuf[16];
      snprintf(coresBuf, sizeof(coresBuf), "%d", m_cpuCores);
      drawLine("CORES", coresBuf, leftColX, col1Y);

      // CPU bar - cache calculations
      const int barWidth = (m_panelWidth >> 1) - 60;
      const int barY = col1Y + 2;
      const int fillWidth = static_cast<int>((m_cpuUsage * 0.01f) * barWidth);

      m_renderer->drawRect(leftColX, barY, barWidth, m_barHeight, BAR_BG);
      if (fillWidth > 0) {
        m_renderer->drawRect(leftColX, barY, fillWidth, m_barHeight, getBarColor(m_cpuUsage));
      }
      m_renderer->drawRectOutline(leftColX, barY, barWidth, m_barHeight, CYAN);

      char cpuBuf[16];
      snprintf(cpuBuf, sizeof(cpuBuf), "%.1f%%", m_cpuUsage);
      m_renderer->drawText(font, cpuBuf, leftColX + barWidth + 8, col1Y, WHITE);
      col1Y += m_lineHeight;
    }

    if (m_settings.showRAMUsage) {
      const float ramPercent = m_totalRamBytes > 0 ? (100.0f * m_usedRamBytes / m_totalRamBytes) : 0.0f;
      const std::string ramStr = formatBytes(m_usedRamBytes) + " / " + formatBytes(m_totalRamBytes);
      drawLine("RAM", ramStr, leftColX, col1Y);

      // RAM bar - cache calculations
      const int barWidth = (m_panelWidth >> 1) - 60;
      const int barY = col1Y + 2;
      const int fillWidth = static_cast<int>((ramPercent * 0.01f) * barWidth);

      m_renderer->drawRect(leftColX, barY, barWidth, m_barHeight, BAR_BG);
      if (fillWidth > 0) {
        m_renderer->drawRect(leftColX, barY, fillWidth, m_barHeight, getBarColor(ramPercent));
      }
      m_renderer->drawRectOutline(leftColX, barY, barWidth, m_barHeight, CYAN);

      char ramBuf[16];
      snprintf(ramBuf, sizeof(ramBuf), "%.1f%%", ramPercent);
      m_renderer->drawText(font, ramBuf, leftColX + barWidth + 8, col1Y, WHITE);
      col1Y += m_lineHeight;
    }
  }

  // === RIGHT COLUMN: PERFORMANCE + ENTITIES + NETWORK ===
  if (m_settings.showFPS) {
    drawSection("PERFORMANCE", rightColX, col2Y);

    char fpsBuf[16];
    snprintf(fpsBuf, sizeof(fpsBuf), "%.0f", m_fps);
    m_renderer->drawText(font, "FPS", rightColX, col2Y, YELLOW);
    m_renderer->drawText(font, fpsBuf, rightColX + m_labelWidth, col2Y, m_fps < 30.0f ? RED : WHITE);
    col2Y += m_lineHeight;

    char timeBuf[16];
    snprintf(timeBuf, sizeof(timeBuf), "%.1fs", m_gameTime);
    m_renderer->drawText(font, "TIME", rightColX, col2Y, YELLOW);
    m_renderer->drawText(font, timeBuf, rightColX + m_labelWidth, col2Y, WHITE);
    col2Y += m_lineHeight + (m_sectionGap >> 1);
  }

  if (m_settings.showEntityCount) {
    m_renderer->drawText(font, "[ ENTITIES ]", rightColX, col2Y, CYAN);
    col2Y += m_lineHeight;

    char entityBuf[16];
    snprintf(entityBuf, sizeof(entityBuf), "%d", m_entityCount);
    m_renderer->drawText(font, "TOTAL", rightColX, col2Y, YELLOW);
    m_renderer->drawText(font, entityBuf, rightColX + m_labelWidth, col2Y, WHITE);
    col2Y += m_lineHeight;

    char playerBuf[16];
    snprintf(playerBuf, sizeof(playerBuf), "%d", m_playerCount);
    m_renderer->drawText(font, "PLAYERS", rightColX, col2Y, YELLOW);
    m_renderer->drawText(font, playerBuf, rightColX + m_labelWidth, col2Y, WHITE);
    col2Y += m_lineHeight + (m_sectionGap >> 1);
  }

  if (m_settings.showNetworkInfo) {
    m_renderer->drawText(font, "[ NETWORK ]", rightColX, col2Y, CYAN);
    col2Y += m_lineHeight;

    if (m_connected) {
      m_renderer->drawText(font, "STATUS", rightColX, col2Y, YELLOW);
      m_renderer->drawText(font, "ONLINE", rightColX + m_labelWidth, col2Y, GREEN);
      col2Y += m_lineHeight;

      char pingBuf[16];
      snprintf(pingBuf, sizeof(pingBuf), "%.0f ms", m_latency);
      m_renderer->drawText(font, "PING", rightColX, col2Y, YELLOW);
      m_renderer->drawText(font, pingBuf, rightColX + m_labelWidth, col2Y, m_latency > 100.0f ? RED : WHITE);
      col2Y += m_lineHeight;
    } else {
      m_renderer->drawText(font, "STATUS", rightColX, col2Y, YELLOW);
      m_renderer->drawText(font, "OFFLINE", rightColX + m_labelWidth, col2Y, RED);
      col2Y += m_lineHeight;
    }
  }
}

void InfoMode::drawSection(const std::string &title, int x, int &y)
{
  const std::string header = "[ " + title + " ]";
  m_renderer->drawText(m_hudFont.get(), header, x, y, CYAN);
  y += m_lineHeight;
}

void InfoMode::drawLine(const std::string &label, const std::string &value, int x, int &y, bool highlight)
{
  void *font = m_hudFont.get();
  m_renderer->drawText(font, label, x, y, YELLOW);
  m_renderer->drawText(font, value, x + m_labelWidth, y, highlight ? RED : WHITE);
  y += m_lineHeight;
}

void InfoMode::drawProgressBar(const std::string &label, float percent, int &y)
{
  (void)label;

  const int barWidth = m_panelWidth - 50;
  const int barY = y + 2;
  const int fillWidth = static_cast<int>((percent * 0.01f) * barWidth);

  // Background, fill, border
  m_renderer->drawRect(m_panelX, barY, barWidth, m_barHeight, BAR_BG);
  if (fillWidth > 0) {
    m_renderer->drawRect(m_panelX, barY, fillWidth, m_barHeight, getBarColor(percent));
  }
  m_renderer->drawRectOutline(m_panelX, barY, barWidth, m_barHeight, CYAN);

  // Percentage text
  char percentBuf[16];
  snprintf(percentBuf, sizeof(percentBuf), "%.1f%%", percent);
  m_renderer->drawText(m_hudFont.get(), percentBuf, m_panelX + barWidth + 8, y, WHITE);

  y += m_lineHeight;
}

void InfoMode::drawSeparator(int &y)
{
  y += m_sectionGap;
}

void InfoMode::setGameData(int health, int score, float fps)
{
  m_health = health;
  m_score = score;
  m_fps = fps;
}

void InfoMode::setGameStats(int entityCount, int playerCount, int enemyCount, int projectileCount, float gameTime)
{
  m_entityCount = entityCount;
  m_playerCount = playerCount;
  m_enemyCount = enemyCount;
  m_projectileCount = projectileCount;
  m_gameTime = gameTime;
}

void InfoMode::setNetworkData(float latency, bool connected, int packetsPerSecond)
{
  m_latency = latency;
  m_connected = connected;
  m_packetsPerSecond = packetsPerSecond;
}

void InfoMode::setNetworkBandwidth(int uploadBytes, int downloadBytes)
{
  m_uploadBytes = uploadBytes;
  m_downloadBytes = downloadBytes;
}

} // namespace rtype
