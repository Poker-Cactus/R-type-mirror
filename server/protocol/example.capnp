@0xabcd1234abcd1234; # ID unique

# Structure racine qui représente ton paquet pour une frame
struct FrameUpdate {
  frameSequence @0 :UInt32;   # Pour gérer l'ordre et la perte de paquets
  timestamp     @1 :UInt64;
  entities      @2 :List(EntityUpdate);
}

struct EntityUpdate {
  entityId   @0 :UInt32;
  components @1 :List(Component);
}

# Wrapper pour tes composants.
# Utilise une UNION pour dire "ce composant est soit une Position, soit une Velocity, etc."
struct Component {
  union {
    position @0 :Position;
    velocity @1 :Velocity;
    health   @2 :Health;
    input    @3 :PlayerInput;
  }
}

# Tes composants ECS
struct Position {
  x @0 :Float32;
  y @1 :Float32;
}

struct Velocity {
  dx @0 :Float32;
  dy @1 :Float32;
}

struct Health {
  current @0 :UInt8;
  max     @1 :UInt8;
}

struct PlayerInput {
  buttons @0 :UInt16;
}
