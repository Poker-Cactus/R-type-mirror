/*
** EPITECH PROJECT, 2025
** rtype
** File description:
** Example: Simple ECS with SystemManager
*/

#include "ecs/ComponentSignature.hpp"
#include "ecs/Entity.hpp"
#include "ecs/ISystem.hpp"
#include "ecs/World.hpp"
#include <iostream>
#include <string>
#include <utility>

// ============================================================================
// COMPOSANTS (à mettre normalement dans common/src/components/)
// ============================================================================

struct Position {
  float x;
  float y;
};

struct Velocity {
  float x;
  float y;
};

struct Health {
  int current;
  int max;
};

struct Name {
  std::string value;
};

// ============================================================================
// SYSTÈMES
// ============================================================================

constexpr float DefaultDeltaTime = 0.016F; // ~60 FPS
constexpr int DefaultLogInterval = 60;
constexpr int BasicUsageFrames = 5;
constexpr int RTypeTicks = 3;
constexpr int NetworkLogInterval = 10;

class MovementSystem : public ecs::ISystem
{
public:
  void update(ecs::World &world, float deltaTime) override
  {
    std::cout << "[MovementSystem] Updating positions...\n";
    (void)world;
    (void)deltaTime;
  }

  ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<Position>());
    sig.set(ecs::getComponentId<Velocity>());
    return sig;
  }
};

class HealthSystem : public ecs::ISystem
{
private:
  int updateCount = 0;

public:
  void update(ecs::World &world, float deltaTime) override
  {
    updateCount++;
    std::cout << "[HealthSystem] Check health (update #" << updateCount << ")\n";
    (void)world;
    (void)deltaTime;
  }

  [[nodiscard]] int getUpdateCount() const { return updateCount; }

  [[nodiscard]] ecs::ComponentSignature getSignature() const override
  {
    ecs::ComponentSignature sig;
    sig.set(ecs::getComponentId<Health>());
    return sig;
  }
};

class LogSystem : public ecs::ISystem
{
private:
  std::string prefix;
  int logInterval;
  int frameCount = 0;

public:
  LogSystem(std::string prefix, int interval = DefaultLogInterval) : prefix(std::move(prefix)), logInterval(interval) {}

  void update(ecs::World &world, float deltaTime) override
  {
    frameCount++;
    if (frameCount % logInterval == 0) {
      std::cout << "[" << prefix << "] Frame " << frameCount << " (dt: " << deltaTime << "s)\n";
    }
    (void)world;
  }

  ecs::ComponentSignature getSignature() const override
  {
    // LogSystem doesn't require any specific components
    return ecs::ComponentSignature();
  }
};

// ============================================================================
// DÉMONSTRATION
// ============================================================================

void demonstrateBasicUsage()
{
  std::cout << "\n=== DÉMONSTRATION 1: Utilisation basique ===\n";
  ecs::World world;
  std::cout << "Enregistrement des systèmes...\n";
  world.registerSystem<MovementSystem>();
  world.registerSystem<HealthSystem>();
  world.registerSystem<LogSystem>("GameLoop", 3);
  std::cout << "\nSimulation de " << BasicUsageFrames << " frames...\n";
  for (int i = 0; i < BasicUsageFrames; i++) {
    std::cout << "\n--- Frame " << (i + 1) << " ---\n";
    world.update(DefaultDeltaTime);
  }
}

void demonstrateSystemAccess()
{
  std::cout << "\n\n=== DÉMONSTRATION 2: Accès aux systèmes ===\n";
  ecs::World world;
  world.registerSystem<HealthSystem>();
  world.registerSystem<MovementSystem>();
  auto *healthSys = world.getSystem<HealthSystem>();
  if (healthSys != nullptr) {
    std::cout << "HealthSystem trouvé !\n";
    std::cout << "Nombre d'updates avant: " << healthSys->getUpdateCount() << "\n";
  }
  world.update(DefaultDeltaTime);
  if (healthSys != nullptr) {
    std::cout << "Nombre d'updates après: " << healthSys->getUpdateCount() << "\n";
  }
}

void demonstrateSystemRemoval()
{
  std::cout << "\n\n=== DÉMONSTRATION 3: Suppression de systèmes ===\n";
  ecs::World world;
  world.registerSystem<MovementSystem>();
  world.registerSystem<HealthSystem>();
  world.registerSystem<LogSystem>("Demo", 1);
  std::cout << "\nUpdate avec tous les systèmes:\n";
  world.update(DefaultDeltaTime);
  std::cout << "\nSuppression de MovementSystem...\n";
  world.removeSystem<MovementSystem>();
  std::cout << "\nUpdate sans MovementSystem:\n";
  world.update(DefaultDeltaTime);
}

void demonstrateExecutionOrder()
{
  std::cout << "\n\n=== DÉMONSTRATION 4: Ordre d'exécution ===\n";
  std::cout << "L'ordre d'enregistrement détermine l'ordre d'exécution:\n\n";
  ecs::World world;
  world.registerSystem<LogSystem>("1-Input", 1);
  world.registerSystem<MovementSystem>();
  world.registerSystem<LogSystem>("3-Collision", 1);
  world.registerSystem<HealthSystem>();
  world.registerSystem<LogSystem>("5-Render", 1);
  std::cout << "Ordre d'exécution lors d'un update:\n";
  world.update(DefaultDeltaTime);
}

void demonstrateRTypeScenario()
{
  std::cout << "\n\n=== DÉMONSTRATION 5: Scénario R-Type ===\n";
  std::cout << "Configuration typique pour le serveur R-Type:\n\n";
  ecs::World world;
  world.registerSystem<LogSystem>("Network-Receive", NetworkLogInterval);
  world.registerSystem<MovementSystem>();
  world.registerSystem<LogSystem>("Collision", NetworkLogInterval);
  world.registerSystem<HealthSystem>();
  world.registerSystem<LogSystem>("Spawn", NetworkLogInterval);
  world.registerSystem<LogSystem>("Network-Send", NetworkLogInterval);
  std::cout << "\nSimulation de " << RTypeTicks << " ticks serveur (60 Hz):\n";
  for (int i = 0; i < RTypeTicks; i++) {
    std::cout << "\n=== TICK " << (i + 1) << " ===\n";
    world.update(DefaultDeltaTime);
  }
}

void demonstrateComponentSignatures()
{
  std::cout << "\n\n=== DÉMONSTRATION 6: Signatures de composants ===\n";
  std::cout << "Les signatures permettent de filtrer les entités efficacement\n\n";

  ecs::World world;
  auto &manager = world.getComponentManager();

  // Créer des entités avec différents composants
  constexpr Entity kPlayer = 1;
  constexpr Entity kEnemy = 2;
  constexpr Entity kHealthPack = 3;
  constexpr float kPlayerX = 100.0F;
  constexpr float kPlayerY = 100.0F;
  constexpr float kPlayerVelX = 5.0F;
  constexpr float kPlayerVelY = 0.0F;
  constexpr int kPlayerHealth = 100;
  constexpr float kEnemyX = 200.0F;
  constexpr float kEnemyY = 150.0F;
  constexpr float kEnemyVelX = -2.0F;
  constexpr float kEnemyVelY = 0.0F;
  constexpr int kEnemyHealth = 50;
  constexpr float kHealthPackX = 300.0F;
  constexpr float kHealthPackY = 200.0F;
  constexpr int kHealthPackAmount = 25;

  std::cout << "Création d'entités:\n";

  // Joueur : Position + Velocity + Health
  manager.addComponent(kPlayer, Position{kPlayerX, kPlayerY});
  manager.addComponent(kPlayer, Velocity{kPlayerVelX, kPlayerVelY});
  manager.addComponent(kPlayer, Health{kPlayerHealth, kPlayerHealth});
  std::cout << "  - Joueur (Position, Velocity, Health)\n";

  // Ennemi : Position + Velocity + Health
  manager.addComponent(kEnemy, Position{kEnemyX, kEnemyY});
  manager.addComponent(kEnemy, Velocity{kEnemyVelX, kEnemyVelY});
  manager.addComponent(kEnemy, Health{kEnemyHealth, kEnemyHealth});
  std::cout << "  - Ennemi (Position, Velocity, Health)\n";

  // Pack de vie : Position + Health (pas de mouvement)
  manager.addComponent(kHealthPack, Position{kHealthPackX, kHealthPackY});
  manager.addComponent(kHealthPack, Health{kHealthPackAmount, kHealthPackAmount});
  std::cout << "  - Pack de vie (Position, Health)\n";

  // Afficher les signatures
  std::cout << "\nSignatures des systèmes:\n";

  MovementSystem movementSys;
  auto moveSig = movementSys.getSignature();
  std::cout << "  - MovementSystem: Position + Velocity\n";
  std::cout << "    Bits activés: Position=" << moveSig.test(ecs::getComponentId<Position>())
            << ", Velocity=" << moveSig.test(ecs::getComponentId<Velocity>())
            << ", Health=" << moveSig.test(ecs::getComponentId<Health>()) << "\n";

  HealthSystem healthSys;
  auto healthSig = healthSys.getSignature();
  std::cout << "  - HealthSystem: Health\n";
  std::cout << "    Bits activés: Position=" << healthSig.test(ecs::getComponentId<Position>())
            << ", Velocity=" << healthSig.test(ecs::getComponentId<Velocity>())
            << ", Health=" << healthSig.test(ecs::getComponentId<Health>()) << "\n";

  // Vérifier quelles entités matchent
  std::cout << "\nFiltre des entités par système:\n";

  auto playerSig = world.getEntitySignature(kPlayer);
  auto enemySig = world.getEntitySignature(kEnemy);
  auto healthPackSig = world.getEntitySignature(kHealthPack);

  std::cout << "  MovementSystem (Position + Velocity):\n";
  std::cout << "    - Joueur: " << ((playerSig & moveSig) == moveSig ? "✓ Match" : "✗ Pas de match") << "\n";
  std::cout << "    - Ennemi: " << ((enemySig & moveSig) == moveSig ? "✓ Match" : "✗ Pas de match") << "\n";
  std::cout << "    - Pack de vie: " << ((healthPackSig & moveSig) == moveSig ? "✓ Match" : "✗ Pas de match") << "\n";

  std::cout << "  HealthSystem (Health):\n";
  std::cout << "    - Joueur: " << ((playerSig & healthSig) == healthSig ? "✓ Match" : "✗ Pas de match") << "\n";
  std::cout << "    - Ennemi: " << ((enemySig & healthSig) == healthSig ? "✓ Match" : "✗ Pas de match") << "\n";
  std::cout << "    - Pack de vie: " << ((healthPackSig & healthSig) == healthSig ? "✓ Match" : "✗ Pas de match")
            << "\n";

  std::cout << "\n💡 Avantage: Le filtrage se fait en 1 seule opération CPU (bitwise AND) !\n";
}

// ============================================================================
// MAIN
// ============================================================================

int main()
{
  std::cout << R"(
╔═══════════════════════════════════════════════════════════╗
║                                                           ║
║            SystemManager - Guide d'utilisation            ║
║                  avec Component Signatures                ║
║                                                           ║
╚═══════════════════════════════════════════════════════════╝
)" << "\n";

  try {
    demonstrateBasicUsage();
    demonstrateSystemAccess();
    demonstrateSystemRemoval();
    demonstrateExecutionOrder();
    demonstrateRTypeScenario();
    demonstrateComponentSignatures();

    std::cout << "\n\n";
    std::cout << "╔═══════════════════════════════════════════════════════════╗\n";
    std::cout << "║         ✅ Toutes les démonstrations réussies !           ║\n";
    std::cout << "╚═══════════════════════════════════════════════════════════╝\n";
    std::cout << "\nPour créer vos propres systèmes :\n";
    std::cout << "1. Héritez de ecs::ISystem\n";
    std::cout << "2. Implémentez update(World&, float)\n";
    std::cout << "3. Implémentez getSignature() pour déclarer vos composants requis\n";
    std::cout << "4. Enregistrez avec world.registerSystem<VotreSysteme>()\n";
    std::cout << "5. Appelez world.update(deltaTime) dans votre game loop\n";
    std::cout << "\nPour filtrer les entités :\n";
    std::cout << "- Utilisez (entitySig & systemSig) == systemSig\n";
    std::cout << "- C'est une opération O(1) ultra-rapide !\n";

  } catch (const std::exception &e) {
    std::cerr << "Erreur: " << e.what() << "\n";
    return 1;
  }

  return 0;
}
