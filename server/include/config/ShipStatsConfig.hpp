/*
** EPITECH PROJECT, 2025
** R-type-mirror
** File description:
** ShipStatsConfig - Loads ship statistics from configuration file
*/

#ifndef SERVER_CONFIG_SHIPSTATSCONFIG_HPP_
#define SERVER_CONFIG_SHIPSTATSCONFIG_HPP_

#include "../../../engineCore/include/ecs/components/ShipStats.hpp"
#include <fstream>
#include <iostream>
#include <nlohmann/json.hpp>
#include <string>

namespace server
{

class ShipStatsConfig
{
public:
  static ShipStatsConfig &getInstance()
  {
    static ShipStatsConfig instance;
    return instance;
  }

  bool loadFromFile(const std::string &filePath)
  {
    try {
      std::ifstream file(filePath);
      if (!file.is_open()) {
        std::cerr << "[ShipStatsConfig] Failed to open: " << filePath << std::endl;
        return false;
      }

      nlohmann::json json;
      file >> json;

      if (!json.contains("ships")) {
        std::cerr << "[ShipStatsConfig] Missing 'ships' key in config" << std::endl;
        return false;
      }

      const auto &ships = json["ships"];

      // Load default ship
      if (ships.contains("default")) {
        const auto &def = ships["default"];
        m_defaultShip = ecs::ShipStats(def.value("maxHP", 100), def.value("moveSpeed", 200.0f),
                                       def.value("fireRate", 0.25f), def.value("damage", 10));
      }

      // Load fast ship
      if (ships.contains("fast")) {
        const auto &fast = ships["fast"];
        m_fastShip = ecs::ShipStats(fast.value("maxHP", 70), fast.value("moveSpeed", 300.0f),
                                    fast.value("fireRate", 0.25f), fast.value("damage", 10));
      }

      // Load tank ship
      if (ships.contains("tank")) {
        const auto &tank = ships["tank"];
        m_tankShip = ecs::ShipStats(tank.value("maxHP", 150), tank.value("moveSpeed", 150.0f),
                                    tank.value("fireRate", 0.25f), tank.value("damage", 10));
      }

      // Load sniper ship
      if (ships.contains("sniper")) {
        const auto &sniper = ships["sniper"];
        m_sniperShip = ecs::ShipStats(sniper.value("maxHP", 100), sniper.value("moveSpeed", 200.0f),
                                      sniper.value("fireRate", 0.5f), sniper.value("damage", 20));
      }

      std::cout << "[ShipStatsConfig] Loaded ship stats from: " << filePath << std::endl;
      std::cout << "  DEFAULT: HP=" << m_defaultShip.maxHP << " Speed=" << m_defaultShip.moveSpeed << std::endl;
      std::cout << "  FAST:    HP=" << m_fastShip.maxHP << " Speed=" << m_fastShip.moveSpeed << std::endl;
      std::cout << "  TANK:    HP=" << m_tankShip.maxHP << " Speed=" << m_tankShip.moveSpeed << std::endl;
      std::cout << "  SNIPER:  HP=" << m_sniperShip.maxHP << " Speed=" << m_sniperShip.moveSpeed
                << " Damage=" << m_sniperShip.damage << " FireRate=" << m_sniperShip.fireRate << std::endl;

      return true;
    } catch (const std::exception &e) {
      std::cerr << "[ShipStatsConfig] Error loading config: " << e.what() << std::endl;
      return false;
    }
  }

  const ecs::ShipStats &getDefaultShip() const { return m_defaultShip; }
  const ecs::ShipStats &getFastShip() const { return m_fastShip; }
  const ecs::ShipStats &getTankShip() const { return m_tankShip; }
  const ecs::ShipStats &getSniperShip() const { return m_sniperShip; }

private:
  ShipStatsConfig() = default;

  ecs::ShipStats m_defaultShip{100, 200.0f, 0.25f, 10};
  ecs::ShipStats m_fastShip{70, 300.0f, 0.25f, 10};
  ecs::ShipStats m_tankShip{150, 150.0f, 0.25f, 10};
  ecs::ShipStats m_sniperShip{100, 200.0f, 0.5f, 20};
};

} // namespace server

#endif // SERVER_CONFIG_SHIPSTATSCONFIG_HPP_
