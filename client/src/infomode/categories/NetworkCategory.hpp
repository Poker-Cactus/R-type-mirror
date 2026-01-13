/**
 * @file NetworkCategory.hpp
 * @brief Network information category
 */

#pragma once

#include "../InfoCategory.hpp"

/**
 * @class NetworkCategory
 * @brief Displays network and connectivity information
 */
class NetworkCategory : public InfoCategory
{
public:
  /**
   * @brief Constructor
   */
  NetworkCategory();

  /**
   * @brief Destructor
   */
  ~NetworkCategory() override = default;

  /**
   * @brief Get category name
   * @return "Network"
   */
  std::string getName() const override { return "Network"; }

  /**
   * @brief Get information lines
   * @return Vector with network info
   */
  std::vector<std::string> getInfoLines() const override;

  /**
   * @brief Update network information
   * @param deltaTime Time elapsed since last update
   */
  void update(float deltaTime) override;

  /**
   * @brief Set network latency for display
   * @param latency Current latency in milliseconds
   */
  void setLatency(float latency) { m_latency = latency; }

  /**
   * @brief Set connection status
   * @param connected Whether connected to server
   */
  void setConnected(bool connected) { m_connected = connected; }

  /**
   * @brief Set packets per second
   * @param packets Number of packets per second
   */
  void setPacketsPerSecond(int packets) { m_packetsPerSecond = packets; }

  /**
   * @brief Set bytes per second for upload/download
   * @param uploadBytes Upload bytes per second
   * @param downloadBytes Download bytes per second
   */
  void setBandwidth(int uploadBytes, int downloadBytes) {
    m_uploadBytesPerSecond = uploadBytes;
    m_downloadBytesPerSecond = downloadBytes;
  }

private:
  /**
   * @brief Get local IP address
   * @return Local IP address as string
   */
  std::string getLocalIPAddress() const;

  /**
   * @brief Get network interface information
   * @return Network interface details
   */
  std::string getNetworkInterface() const;

  /**
   * @brief Format bytes per second to human readable format
   * @param bytes Bytes per second
   * @return Formatted string (e.g., "1.2 MB/s")
   */
  std::string formatBytesPerSecond(int bytes) const;

  // Network stats
  float m_latency = 0.0f;
  bool m_connected = false;
  int m_packetsPerSecond = 0;
  int m_uploadBytesPerSecond = 0;
  int m_downloadBytesPerSecond = 0;
  mutable std::string m_localIP;
  mutable std::string m_interfaceInfo;
  mutable bool m_networkInfoCached = false;
};
