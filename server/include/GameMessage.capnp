@0xbf5147b1f1e3c5d2;

struct GameMessage {
  id @0 :UInt32;
  type @1 :UInt8;
  x @2 :Float32;
  y @3 :Float32;
}

struct GameState {
  messages @0 : Text;
}