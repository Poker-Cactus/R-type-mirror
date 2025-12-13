#include "../engineCore/include/ecs/components/EntityKind.hpp"

int main()
{
  ecs::EntityKind k(ecs::EntityKind::Kind::PLAYER);
  (void)k;
  return 0;
}
