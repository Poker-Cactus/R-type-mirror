# Comment Démarrer le Projet R-Type

Ce document explique comment configurer l'environnement de développement, installer les dépendances et compiler le projet R-Type.

## 1. Installation des Outils par OS

### Ubuntu/Debian

```bash
# Mise à jour
sudo apt update

# Compilateur et CMake
sudo apt install build-essential cmake

# pipx (pour installer Conan de manière isolée)
sudo apt install pipx
pipx ensurepath
```

Fermez et rouvrez votre terminal pour que `pipx` soit dans le PATH.

### macOS

Nécessite [Homebrew](https://brew.sh/).

```bash
# CMake
brew install cmake

# Python (inclut pip)
brew install python
```

### Windows

1. **Visual Studio** : Installez Visual Studio 2019 ou 2022 avec le workload "Développement Desktop en C++".
2. **CMake** : Téléchargez et installez depuis [cmake.org/download](https://cmake.org/download/).
3. **Python** : Téléchargez et installez depuis [python.org](https://www.python.org/).

## 2. Installation de Conan

### Ubuntu/Debian

```bash
# Installer Conan via pipx (installation isolée)
pipx install conan

# Configurer le profil Conan
conan profile detect --force
```

### macOS / Windows

```bash
# Installer Conan via pip
pip install conan

# Configurer le profil Conan
conan profile detect --force
```

## 3. Compilation du Projet

Une fois les outils installés, suivez ces étapes pour compiler le projet.

### Étape 1 : Création du dossier de build et installation des dépendances

À la racine du projet :

```bash
rm -rf build && mkdir build && cd build
conan install .. --output-folder=. --build=missing
cmake .. -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release
cmake --build .
```

### Étape 2 : Exécution

Les exécutables compilés se trouvent dans les dossiers respectifs :

- **Serveur** : `./server/server` (ou `server\Release\server.exe` sur Windows)
- **Client** : `./client/client` (ou `client\Release\client.exe` sur Windows)

Lancez d'abord le serveur, puis connectez un ou plusieurs clients.

## 4. Troubleshooting

### Compilation échoue

- Vérifiez que vous êtes dans le bon répertoire (racine du projet)
- Assurez-vous que le compilateur C++20 est installé (`g++-10` minimum ou `clang-11`)
- Nettoyez le build : `rm -rf build`
- Réessayez les commandes de compilation

### Conan ne trouve pas les dépendances

```bash
# Mettre à jour Conan (Ubuntu)
pipx upgrade conan

# Mettre à jour Conan (macOS/Windows)
pip install --upgrade conan

# Réinitialiser le profil
conan profile detect --force
```

## 📚 Pour plus d'informations

- Consultez l'[Architecture du projet](ARCHITECTURE.md) pour comprendre la structure
- Voir le [README principal](../README.md) pour un aperçu général
