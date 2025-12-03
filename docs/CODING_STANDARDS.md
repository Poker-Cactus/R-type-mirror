# Normes de Codage et Outils de Formatage

Ce document explique comment utiliser les outils de formatage et de vérification de code pour le projet R-Type.

## 📋 Table des matières

- [Outils utilisés](#outils-utilisés)
- [Configuration](#configuration)
- [Utilisation](#utilisation)
- [Règles de formatage](#règles-de-formatage)
- [Règles de linting](#règles-de-linting)

## 🛠️ Outils utilisés

### clang-format
Outil de formatage automatique du code C++ pour garantir un style cohérent.

### clang-tidy
Outil d'analyse statique qui détecte les bugs potentiels, les mauvaises pratiques et les problèmes de performance.

## ⚙️ Configuration

### Fichiers de configuration

- **`.clang-format`** : Configuration du style de code
- **`.clang-tidy`** : Configuration des règles de linting
- **`scripts/format.sh`** : Script de formatage automatique

### Installation des outils

```bash
# Sur Ubuntu/Debian
sudo apt install clang-format clang-tidy

# Sur macOS
brew install clang-format llvm

# Sur Arch Linux
sudo pacman -S clang
```

## 🚀 Utilisation

### Formatage automatique du code

Pour formater tout le code du projet :

```bash
./scripts/format.sh
```

Ce script :
- Recherche tous les fichiers `.hpp`, `.h` et `.cpp`
- Applique automatiquement les règles de formatage
- Affiche un message de confirmation

### Formatage manuel d'un fichier

```bash
clang-format -i path/to/file.cpp
```

### Vérification sans modification

Pour voir les modifications sans les appliquer :

```bash
clang-format path/to/file.cpp
```

### Analyse avec clang-tidy

Pour analyser un fichier spécifique :

```bash
clang-tidy path/to/file.cpp -- -I./engineCore/include -I./common/include -std=c++20
```

Pour analyser tout le projet avec CMake :

```bash
cd build
cmake --build . --target clang-tidy
```

## 📐 Règles de formatage

### Indentation
- **Largeur** : 4 espaces
- **Tabulations** : Jamais (toujours des espaces)

### Accolades
- **Style** : Linux (accolade ouvrante sur la même ligne, sauf pour les fonctions)

```cpp
// ✅ Correct
void myFunction()
{
    if (condition) {
        // code
    }
}

// ❌ Incorrect
void myFunction() {
    if (condition)
    {
        // code
    }
}
```

### Longueur de ligne
- **Limite** : 120 caractères maximum

### Espacement
- Espaces avant les parenthèses des structures de contrôle (`if`, `for`, `while`)
- Pas d'espaces dans les parenthèses

```cpp
// ✅ Correct
if (condition) {
    for (int i = 0; i < 10; i++) {
        // code
    }
}

// ❌ Incorrect
if( condition ){
    for( int i = 0; i < 10; i++ ){
        // code
    }
}
```

### Fonctions courtes
- Les fonctions inline peuvent rester sur une seule ligne
- Les déclarations de paramètres doivent être compactes

## 🔍 Règles de linting

### Catégories activées

#### 🐛 Détection de bugs (`bugprone-*`)
- Détecte les erreurs courantes et les bugs potentiels

#### 🔬 Analyse statique (`clang-analyzer-*`)
- Analyse approfondie du code pour détecter les problèmes de logique

#### 🚀 Modernisation (`modernize-*`)
- Encourage l'utilisation des fonctionnalités C++ modernes
  - `nullptr` au lieu de `NULL`
  - `auto` quand approprié
  - Smart pointers (`std::unique_ptr`, `std::shared_ptr`)
  - `override` pour les méthodes virtuelles

#### ⚡ Performance (`performance-*`)
- Détecte les problèmes de performance potentiels

#### 📖 Lisibilité (`readability-*`)
- Vérifie la clarté et la maintenabilité du code

#### 🛡️ Core Guidelines (`cppcoreguidelines-*`)
- Application sélective des C++ Core Guidelines
  - Éviter les `goto`
  - Éviter les tableaux C (préférer `std::array` ou `std::vector`)
  - Gestion RAII de la mémoire

### Exemples de bonnes pratiques

#### Smart pointers
```cpp
// ✅ Correct - Utilisation de smart pointers
auto player = std::make_unique<Player>();
auto resource = std::make_shared<Resource>();

// ❌ Incorrect - Gestion manuelle de la mémoire
Player* player = new Player();
// ...
delete player;
```

#### Nullptr
```cpp
// ✅ Correct
int* ptr = nullptr;

// ❌ Incorrect
int* ptr = NULL;
```

#### Auto
```cpp
// ✅ Correct - auto pour les types évidents
auto it = myMap.find(key);
auto value = std::make_unique<MyClass>();

// ✅ Acceptable - type explicite si nécessaire pour la clarté
std::vector<int>::iterator it = myVector.begin();
```

#### Override
```cpp
class Base {
public:
    virtual void update() = 0;
};

class Derived : public Base {
public:
    // ✅ Correct
    void update() override;
    
    // ❌ Incorrect - manque override
    void update();
};
```

## 🔄 Intégration dans le workflow

### Avant chaque commit

1. **Formater le code** :
   ```bash
   ./scripts/format.sh
   ```

2. **Compiler et vérifier** :
   ```bash
   ./build.sh
   ```

3. **Commiter les changements** :
   ```bash
   git add .
   git commit -m "Your message"
   ```

### Configuration IDE

#### VS Code
L'extension C/C++ applique automatiquement le formatage avec `.clang-format` lors de la sauvegarde si configuré :

```json
{
    "editor.formatOnSave": true,
    "C_Cpp.clang_format_style": "file"
}
```

## 📝 Notes importantes

- **Toujours** exécuter `format.sh` avant de commiter
- Les règles de formatage sont **non négociables** pour maintenir la cohérence
- clang-tidy donne des **avertissements**, pas des erreurs bloquantes
- En cas de désaccord avec une règle de linting, discuter avec l'équipe avant de la désactiver

## 🤝 Contribution

Si vous pensez qu'une règle devrait être ajoutée ou modifiée :
1. Ouvrez une issue pour en discuter
2. Proposez la modification dans `.clang-format` ou `.clang-tidy`
3. Soumettez une pull request avec la justification

---

**Maintenez le code propre, moderne et cohérent ! ✨**
