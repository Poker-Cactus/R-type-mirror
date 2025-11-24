# R-Type

Un moteur de jeu multi-joueurs en réseau basé sur l'architecture ECS (Entity Component System), inspiré du jeu classique R-Type.

## 🚀 Démarrage Rapide

```bash
# Compilation
rm -rf build && mkdir build && cd build
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .

# Exécution
./server/server  # Lancer le serveur
./client/client  # Lancer le client
```

## 📚 Documentation

👉 **[Consultez le Wiki complet](https://github.com/Poker-Cactus/R-type-mirror/wiki)**

Liens rapides :
- [Guide de Démarrage](docs/HOW_TO_START.md) - Installation et compilation
- [Architecture](docs/ARCHITECTURE.md) - Structure du projet

## 🛠️ Technologies

**Langage** : C++20 | **Build** : CMake + Conan | **Graphisme** : SDL2 | **Réseau** : Asio

## 📝 Licence

EPITECH PROJECT - 2025