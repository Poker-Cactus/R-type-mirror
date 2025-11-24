# Configuration de l'Environnement de Développement

Ce guide vous aidera à configurer votre environnement de développement pour le projet R-Type.

## Prérequis

- **CMake** >= 3.15
- **Conan** (gestionnaire de paquets C++)
- **Compilateur C++20** :
  - macOS : Clang (Xcode Command Line Tools)
  - Linux : GCC >= 10 ou Clang >= 12
  - Windows : MSVC 2019 ou plus récent
- **VS Code** avec les extensions :
  - C/C++ Extension Pack
  - CMake Tools

## Installation de Conan

### macOS / Linux

```bash
pip install conan
```

### Windows

```powershell
pip install conan
```

## Configuration du Projet

### 1. Cloner le dépôt

```bash
git clone https://github.com/Poker-Cactus/R-type-mirror.git
cd R-type-mirror
```

### 2. Installer les dépendances avec Conan

```bash
conan install . --output-folder=build --build=missing --profile=conan_profile
```

Cette commande va :
- Télécharger et compiler SDL2 et ASIO
- Générer les fichiers CMake nécessaires
- Créer un preset CMake (`conan-release`)

### 3. Configurer CMake

```bash
cmake --preset conan-release
```

### 4. Compiler le projet

```bash
cmake --build build/build/Release
```

## Configuration de VS Code

### IntelliSense

Le fichier `.vscode/c_cpp_properties.json` est déjà configuré pour tous les systèmes d'exploitation. Il utilise des variables d'environnement pour être portable :

- **macOS/Linux** : `${env:HOME}/.conan2/...`
- **Windows** : `${env:USERPROFILE}/.conan2/...`

VS Code détectera automatiquement votre système et utilisera la bonne configuration.

### Configuration CMake

Le fichier `.vscode/settings.json` est configuré pour :
- Utiliser CMake Tools comme provider de configuration
- Ne pas configurer automatiquement à l'ouverture (évite les erreurs si Conan n'est pas encore installé)

## Structure du Projet

```
.
├── engine_core/        # Moteur ECS
├── common/            # Code partagé (Components, Protocol)
├── server/            # Serveur de jeu
├── client/            # Client de jeu
├── docs/              # Documentation
├── conanfile.txt      # Dépendances Conan
└── CMakeLists.txt     # Configuration CMake principale
```

## Exécution

### Serveur

```bash
./build/build/Release/server/server
```

### Client

```bash
./build/build/Release/client/client
```

## Dépendances du Projet

- **ASIO** (1.28.0) : Bibliothèque réseau asynchrone standalone
- **SDL2** (2.28.3) : Bibliothèque graphique pour le rendu

## Résolution des Problèmes

### Erreurs d'include dans VS Code

Si vous voyez des erreurs comme "impossible d'ouvrir le fichier source asio.hpp" :

1. Assurez-vous d'avoir exécuté `conan install`
2. Rechargez la fenêtre VS Code (`Cmd+Shift+P` > "Reload Window")
3. Vérifiez que le chemin Conan existe : `~/.conan2/p/asiof472f53aca4d3/p/include`

### Erreurs de compilation

Si la compilation échoue :

1. Nettoyez le dossier build : `rm -rf build`
2. Réinstallez les dépendances : `conan install . --output-folder=build --build=missing --profile=conan_profile`
3. Reconfigurez CMake : `cmake --preset conan-release`

### Problèmes de version C++

Le projet nécessite C++20. Assurez-vous que votre compilateur le supporte :

```bash
# GCC
g++ --version  # >= 10

# Clang
clang++ --version  # >= 12

# MSVC
cl.exe  # Visual Studio 2019+
```

## Workflow de Développement

1. **Créer une branche** : `git checkout -b feat/ma-feature`
2. **Développer** : Modifier les fichiers, tester
3. **Compiler** : `cmake --build build/build/Release`
4. **Tester** : Exécuter les binaires
5. **Commit** : `git add . && git commit -m "feat: ma feature"`
6. **Push** : `git push origin feat/ma-feature`

## Support

Pour toute question ou problème, consultez :
- [Architecture du Projet](./ARCHITECTURE.md)
- [Guide de Démarrage](./HOW_TO_START.md)
- Les benchmarks dans `docs/BENCHMARK/`
