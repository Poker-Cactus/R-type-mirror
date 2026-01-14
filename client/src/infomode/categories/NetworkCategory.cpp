/**
 * @file NetworkCategory.cpp
 * @brief Implementation of network information category
 */

#include "NetworkCategory.hpp"
#include <iomanip>
#include <sstream>

#ifdef _WIN32
#include <iphlpapi.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "iphlpapi.lib")
#else
#include <arpa/inet.h>
#include <cstring>
#include <ifaddrs.h>
#include <net/if.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <unistd.h>
#endif

NetworkCategory::NetworkCategory() {}

std::vector<std::string> NetworkCategory::getInfoLines() const
{
  std::vector<std::string> lines;

  // Connection Status with visual indicator
  std::string status = m_connected ? "Connected [OK]" : "Disconnected [FAIL]";
  lines.push_back("Status: " + status);

  // Latency with color coding
  std::stringstream latencyText;
  latencyText << std::fixed << std::setprecision(1);
  if (m_latency > 0.0f) {
    if (m_latency < 50.0f) {
      latencyText << "Latency: " << m_latency << "ms (Good)";
    } else if (m_latency < 100.0f) {
      latencyText << "Latency: " << m_latency << "ms (Fair)";
    } else {
      latencyText << "Latency: " << m_latency << "ms (Poor)";
    }
  } else {
    latencyText << "Latency: --ms";
  }
  lines.push_back(latencyText.str());

  // Packets per second
  std::stringstream packetsText;
  if (m_packetsPerSecond > 0) {
    packetsText << "Packets/sec: " << m_packetsPerSecond;
  } else {
    packetsText << "Packets/sec: --";
  }
  lines.push_back(packetsText.str());

  // Bandwidth information
  if (m_uploadBytesPerSecond > 0 || m_downloadBytesPerSecond > 0) {
    lines.push_back("Upload: " + formatBytesPerSecond(m_uploadBytesPerSecond));
    lines.push_back("Download: " + formatBytesPerSecond(m_downloadBytesPerSecond));
  }

  // Network interface information
  std::string interface = getNetworkInterface();
  if (interface != "Unknown") {
    lines.push_back("Interface: " + interface);
  } else {
    lines.push_back("Interface: Detecting...");
  }

  // Local IP address
  std::string localIP = getLocalIPAddress();
  lines.push_back("Local IP: " + localIP);

  // Add network quality indicator
  if (m_connected && m_latency > 0.0f) {
    std::string quality;
    if (m_latency < 30.0f && m_packetsPerSecond > 10) {
      quality = "Quality: Excellent";
    } else if (m_latency < 60.0f && m_packetsPerSecond > 5) {
      quality = "Quality: Good";
    } else if (m_latency < 100.0f) {
      quality = "Quality: Fair";
    } else {
      quality = "Quality: Poor";
    }
    lines.push_back(quality);
  }

  return lines;
}

void NetworkCategory::update()
{
  (void)deltaTime; // Unused parameter
  // Network info doesn't change frequently, cache it
  if (!m_networkInfoCached) {
    m_localIP = getLocalIPAddress();
    m_interfaceInfo = getNetworkInterface();
    m_networkInfoCached = true;
  }
}

std::string NetworkCategory::getLocalIPAddress() const
{
#ifdef _WIN32
  // Windows implementation
  char buffer[256];
  if (gethostname(buffer, sizeof(buffer)) == 0) {
    hostent *host = gethostbyname(buffer);
    if (host && host->h_addr_list[0]) {
      in_addr addr;
      addr.s_addr = *(u_long *)host->h_addr_list[0];
      return inet_ntoa(addr);
    }
  }
  return "Unknown";

#else
  // Unix-like systems
  ifaddrs *ifaddr, *ifa;
  char host[NI_MAXHOST];

  if (getifaddrs(&ifaddr) == -1) {
    return "Unknown";
  }

  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr)
      continue;

    int family = ifa->ifa_addr->sa_family;

    if (family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0) { // Skip loopback
      if (getnameinfo(ifa->ifa_addr, sizeof(sockaddr_in), host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST) == 0) {
        freeifaddrs(ifaddr);
        return std::string(host);
      }
    }
  }

  freeifaddrs(ifaddr);
  return "Unknown";
#endif
}

std::string NetworkCategory::formatBytesPerSecond(int bytes) const
{
  if (bytes == 0) {
    return "-- B/s";
  }

  const char *units[] = {"B/s", "KB/s", "MB/s", "GB/s"};
  int unitIndex = 0;
  double value = static_cast<double>(bytes);

  while (value >= 1024.0 && unitIndex < 3) {
    value /= 1024.0;
    unitIndex++;
  }

  std::stringstream ss;
  ss << std::fixed << std::setprecision(1) << value << " " << units[unitIndex];
  return ss.str();
}

std::string NetworkCategory::getNetworkInterface() const
{
#ifdef _WIN32
  // Windows implementation - get active network interface
  PIP_ADAPTER_INFO adapterInfo = nullptr;
  PIP_ADAPTER_INFO adapter = nullptr;
  ULONG bufferSize = sizeof(IP_ADAPTER_INFO);
  DWORD dwRetVal = 0;

  adapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
  if (adapterInfo == nullptr) {
    return "Unknown";
  }

  if (GetAdaptersInfo(adapterInfo, &bufferSize) == ERROR_BUFFER_OVERFLOW) {
    free(adapterInfo);
    adapterInfo = (IP_ADAPTER_INFO *)malloc(bufferSize);
    if (adapterInfo == nullptr) {
      return "Unknown";
    }
  }

  if ((dwRetVal = GetAdaptersInfo(adapterInfo, &bufferSize)) == NO_ERROR) {
    adapter = adapterInfo;
    while (adapter) {
      if (adapter->Type == MIB_IF_TYPE_ETHERNET || adapter->Type == IF_TYPE_IEEE80211) {
        std::string result = std::string(adapter->Description) + " (" + std::string(adapter->AdapterName) + ")";
        free(adapterInfo);
        return result;
      }
      adapter = adapter->Next;
    }
  }

  if (adapterInfo) {
    free(adapterInfo);
  }
  return "Unknown";

#else
  // Unix-like systems - get active network interface
  ifaddrs *ifaddr, *ifa;

  if (getifaddrs(&ifaddr) == -1) {
    return "Unknown";
  }

  std::string activeInterface = "Unknown";

  for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next) {
    if (ifa->ifa_addr == nullptr)
      continue;

    int family = ifa->ifa_addr->sa_family;

    if (family == AF_INET && strcmp(ifa->ifa_name, "lo") != 0) { // Skip loopback
      // Check if interface is up and running
      int sock = socket(AF_INET, SOCK_DGRAM, 0);
      if (sock >= 0) {
        ifreq ifr;
        memset(&ifr, 0, sizeof(ifr));
        strncpy(ifr.ifr_name, ifa->ifa_name, IFNAMSIZ - 1);

        if (ioctl(sock, SIOCGIFFLAGS, &ifr) >= 0) {
          if (ifr.ifr_flags & IFF_UP && ifr.ifr_flags & IFF_RUNNING) {
            activeInterface = std::string(ifa->ifa_name);
          }
        }
        close(sock);
      }
    }
  }

  freeifaddrs(ifaddr);
  return activeInterface;
#endif
}
