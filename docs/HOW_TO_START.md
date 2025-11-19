# Comment Démarrer le Projet R-Type

Ce document explique comment configurer l'environnement de développement, installer les dépendances et compiler le projet R-Type.

## 1. Installation des Outils par OS

### Ubuntu/Debian
```bash
# Mise à jour
sudo apt update

# Compilateur et CMake
sudo apt install build-essential cmake

# Python (pour Conan)
sudo apt install python3 python3-pip

# Dépendances système pour SDL2 (souvent requises par Conan pour compiler)
sudo apt install libsdl2-dev
```

### macOS
Nécessite [Homebrew](https://brew.sh/).
```bash
# CMake
brew install cmake

# Python
brew install python

# SDL2 (Optionnel si géré par Conan, mais utile)
brew install sdl2
```

### Windows
1.  **Visual Studio** : Installez Visual Studio 2019 ou 2022 avec le workload "Développement Desktop en C++".
2.  **CMake** : Téléchargez et installez depuis [cmake.org/download](https://cmake.org/download/).
3.  **Python** : Téléchargez et installez depuis [python.org](https://www.python.org/).

## 2. Lancer le Projet

Une fois les outils installés, suivez ces étapes pour compiler et lancer le jeu.

### Étape 1 : Installation de Conan
```bash
pip install conan
conan profile detect --force
```

### Étape 2 : Création du dossier de build et compilation
À la racine du projet, exécutez les commandes suivantes :
```bash
rm -rf build ; mkdir build
cd build
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Étape 3 : Exécution
Les exécutables se trouvent dans les dossiers respectifs :

*   **Serveur** : `./server/server` (ou `server\Release\server.exe` sur Windows)
*   **Client** : `./client/client` (ou `client\Release\client.exe` sur Windows)
