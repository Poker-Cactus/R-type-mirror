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
sudo apt install python3 python3-pip python3-full
```

### macOS

Nécessite [Homebrew](https://brew.sh/).

```bash
# CMake
brew install cmake

# Python
brew install python
```

### Windows

1. **Visual Studio** : Installez Visual Studio 2019 ou 2022 avec le workload "Développement Desktop en C++".
2. **CMake** : Téléchargez et installez depuis [cmake.org/download](https://cmake.org/download/).
3. **Python** : Téléchargez et installez depuis [python.org](https://www.python.org/).

## 2. Configuration de l'Environnement Virtuel Python

Afin d'éviter les problèmes de permissions avec pip (notamment sur Debian/Ubuntu), un environnement virtuel Python est recommandé.

### Créer l'environnement virtuel

À la racine du projet :

```bash
python3 -m venv venv
```

### Activer l'environnement virtuel

**Linux/macOS** :

```bash
source venv/bin/activate
```

**Windows** (PowerShell) :

```powershell
venv\Scripts\Activate.ps1
```

**Windows** (CMD) :

```cmd
venv\Scripts\activate.bat
```

Une fois activé, vous verrez `(venv)` au début de votre terminal.

### Optionnel : Activation automatique à chaque terminal

Ajoutez cette ligne à votre `~/.bashrc` ou `~/.zshrc` :

```bash
if [[ "$PWD" == /chemin/vers/TypeMirror* ]]; then
    source /chemin/vers/TypeMirror/venv/bin/activate
fi
```

## 3. Lancer le Projet

Une fois les outils installés et l'environnement virtuel activé, suivez ces étapes pour compiler et lancer le jeu.

### Étape 1 : Installation de Conan et configuration

```bash
# S'assurer que l'env virtuel est activé
source venv/bin/activate  # Linux/macOS

# Installer Conan
pip install conan

# Détecter le profil Conan
conan profile detect --force
```

### Étape 2 : Création du dossier de build et compilation des dépendances

À la racine du projet, exécutez :

```bash
# Nettoyer et créer le dossier de build
rm -rf build ; mkdir build
cd build

# Télécharger et préparer les dépendances
conan install .. --output-folder=. --build=missing

# Configurer CMake
cmake .. \
  -DCMAKE_TOOLCHAIN_FILE=build/Release/generators/conan_toolchain.cmake \
  -DCMAKE_BUILD_TYPE=Release

# Compiler le projet
cmake --build .
```

### Étape 3 : Exécution

Les exécutables compilés se trouvent dans les dossiers respectifs :

- **Serveur** : `./server/server` (ou `server\Release\server.exe` sur Windows)
- **Client** : `./client/client` (ou `client\Release\client.exe` sur Windows)

Lancez d'abord le serveur, puis connectez un ou plusieurs clients.

## 4. Troubleshooting

### "externally-managed-environment" (Debian/Ubuntu)

Si vous avez cette erreur lors de `pip install`, activez simplement l'environnement virtuel :

```bash
source venv/bin/activate
```

### Compilation échoue

- Vérifiez que vous êtes dans le bon répertoire (racine du projet)
- Assurez-vous que le compilateur C++20 est installé (`g++-10` minimum ou `clang-11`)
- Nettoyez le build : `rm -rf build`
- Réessayez les commandes de l'étape 2

### Conan ne trouve pas les dépendances

```bash
# Mettre à jour Conan
pip install --upgrade conan

# Réinitialiser le profil
conan profile detect --force
```

## 📚 Pour plus d'informations

- Consultez l'[Architecture du projet](ARCHITECTURE.md) pour comprendre la structure
- Voir le [README principal](../README.md) pour un aperçu général
