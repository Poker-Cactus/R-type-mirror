# Guide de Tests R-Type

## Vue d'ensemble

Ce projet utilise **doctest** pour les tests unitaires du moteur ECS. doctest est un framework de test C++ moderne, léger et extrêmement rapide, largement utilisé dans l'industrie.

**Pourquoi doctest ?**
- ⚡ Ultra-rapide : 10-20x plus rapide que Google Test en compilation
- 🪶 Header-only : Un seul fichier d'en-tête à inclure
- 🎯 Syntaxe simple et intuitive
- 🔧 Support natif de BDD (Behavior-Driven Development)

## 🚀 Lancer tous les tests

### Méthode 1 : Via le script build.sh (recommandé)
```bash
./build.sh test
```

### Méthode 2 : Directement avec l'exécutable
```bash
./build/tests/system_manager_tests
```

### Méthode 3 : Via CTest
```bash
cd build
ctest --test-dir engineCore/tests --output-on-failure
```

## 🎯 Commandes utiles

### Lancer un test spécifique

```bash
# Par nom de test
./build/tests/system_manager_tests --test-case="*RegisterSystem*"

# Par pattern (tous les tests SystemManager)
./build/tests/system_manager_tests --test-case="SystemManager*"

# Par pattern (tous les tests contenant "Update")
./build/tests/system_manager_tests --test-case="*Update*"
```

### Lancer une suite de tests

```bash
# Tous les tests SystemManager
./build/tests/system_manager_tests --test-case="SystemManager*"

# Tous les tests WorldIntegration
./build/tests/system_manager_tests --test-case="WorldIntegration*"

# Tous les tests EdgeCases
./build/tests/system_manager_tests --test-case="EdgeCases*"
```

### Options utiles de doctest

```bash
# Liste tous les tests disponibles
./build/tests/system_manager_tests --list-test-cases

# Liste avec plus de détails
./build/tests/system_manager_tests --list-test-cases --list-test-names-only

# Mode succès (affiche les tests réussis)
./build/tests/system_manager_tests --success

# Mode minimal (seulement les erreurs)
./build/tests/system_manager_tests --minimal

# Arrêter au premier échec
./build/tests/system_manager_tests --abort-after=1

# Exécuter N fois
./build/tests/system_manager_tests --count=100

# Ordre aléatoire
./build/tests/system_manager_tests --order-by=rand

# Durée des tests
./build/tests/system_manager_tests --duration=true

# Exclure certains tests
./build/tests/system_manager_tests --test-case-exclude="*Stress*"
```

### Avec CTest

```bash
cd build

# Mode verbose
ctest --test-dir engineCore/tests --verbose

# Lancer seulement les tests qui ont échoué
ctest --test-dir engineCore/tests --rerun-failed

# Lancer en parallèle (4 jobs)
ctest --test-dir engineCore/tests -j4

# Afficher la sortie seulement en cas d'échec
ctest --test-dir engineCore/tests --output-on-failure
```

## 📊 Structure des tests

### Fichiers

```
engineCore/
├── tests/
│   ├── CMakeLists.txt              # Configuration CMake pour les tests
│   └── SystemManagerTests.cpp      # Tests du SystemManager (20 tests)
└── include/ecs/
    └── SystemManager.hpp           # Code testé
```

### Organisation des tests

1. **SystemManager** (10 tests)
   - Tests des fonctionnalités de base du SystemManager
   - Enregistrement, récupération, suppression de systèmes
   - Mécanisme d'update

2. **WorldIntegration** (5 tests)
   - Tests d'intégration avec la classe World
   - Interaction entre World et SystemManager

3. **EdgeCases** (5 tests)
   - Tests de cas limites et de robustesse
   - Tests de stress

## 🧪 Exemples concrets

### Tester uniquement l'enregistrement de systèmes

```bash
./build/tests/system_manager_tests --test-case="*Register*"
```

Sortie :
```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases: 3 | 3 passed | 0 failed | 0 skipped
[doctest] assertions: 6 | 6 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Tester uniquement la suppression

```bash
./build/tests/system_manager_tests --test-case="*Remove*"
```

### Tester tous les cas limites

```bash
./build/tests/system_manager_tests --test-case="EdgeCases*"
```

### Débugger un test qui échoue

```bash
# Lancer le test avec plus de détails
./build/tests/system_manager_tests \
  --test-case="*MonTest*" \
  --success \
  --abort-after=1

# Ou avec un debugger
lldb ./build/tests/system_manager_tests
> run --test-case="*MonTest*"
```

## 📈 Interpréter les résultats

### Sortie réussie

```
[doctest] doctest version is "2.4.11"
[doctest] run with "--help" for options
===============================================================================
[doctest] test cases: 20 | 20 passed | 0 failed | 0 skipped
[doctest] assertions: 45 | 45 passed | 0 failed |
[doctest] Status: SUCCESS!
```

### Sortie en cas d'échec

```
TEST CASE:  SystemManager - GetSystemReturnsValidPointer

/path/to/test.cpp:110: ERROR: REQUIRE( system != nullptr ) is NOT correct!
  values: REQUIRE( 0x0 != 0x0 )
```

L'erreur indique :
- Le fichier et la ligne : `/path/to/test.cpp:110`
- L'assertion qui a échoué : `REQUIRE( system != nullptr )`
- Les valeurs obtenues : `REQUIRE( 0x0 != 0x0 )`

## 🔧 Ajouter de nouveaux tests

### Template de base

```cpp
TEST_CASE("SystemManager - MonNouveauTest") {
    // Arrange - Préparer l'environnement
    ecs::SystemManager manager;
    ecs::World world;
    manager.registerSystem<TestSystem>();

    // Act - Exécuter l'action à tester
    auto* system = manager.getSystem<TestSystem>();

    // Assert - Vérifier le résultat
    REQUIRE(system != nullptr);
    CHECK(system->getUpdateCallCount() == 0);
}
```

### Avec SUBCASE (équivalent de fixtures)

```cpp
TEST_CASE("SystemManager - TestsAvecSetup") {
    // Setup commun
    ecs::SystemManager manager;
    ecs::World world;
    manager.registerSystem<TestSystem>();

    SUBCASE("Premier sous-test") {
        auto* sys = manager.getSystem<TestSystem>();
        CHECK(sys != nullptr);
    }

    SUBCASE("Deuxième sous-test") {
        manager.update(world, 0.016f);
        auto* sys = manager.getSystem<TestSystem>();
        CHECK(sys->getUpdateCallCount() == 1);
    }
}
```

### Assertions disponibles

```cpp
// Assertions basiques
CHECK(expression);          // Continue si échoue
REQUIRE(expression);        // Arrête le test si échoue

// Égalité
CHECK(a == b);
CHECK(a != b);

// Comparaison
CHECK(a < b);
CHECK(a <= b);
CHECK(a > b);
CHECK(a >= b);

// Booléens
CHECK(condition);
CHECK_FALSE(condition);

// Pointeurs
CHECK(ptr != nullptr);
CHECK(ptr == nullptr);

// Exceptions
CHECK_THROWS(expression);
CHECK_THROWS_AS(expression, exception_type);
CHECK_NOTHROW(expression);

// Messages personnalisés
CHECK_MESSAGE(condition, "Mon message d'erreur");
REQUIRE_MESSAGE(ptr != nullptr, "Le pointeur ne devrait pas être null");

// Approximation (nombres flottants)
CHECK(value == doctest::Approx(3.14).epsilon(0.01));
```

### Différence CHECK vs REQUIRE

```cpp
TEST_CASE("Différence CHECK vs REQUIRE") {
    int* ptr = nullptr;

    // CHECK continue même si échoue
    CHECK(ptr != nullptr);
    CHECK(ptr->value == 42);  // ⚠️ Crash potentiel si ptr est null

    // REQUIRE arrête le test immédiatement si échoue
    REQUIRE(ptr != nullptr);
    CHECK(ptr->value == 42);  // ✅ Sûr, exécuté seulement si ptr != null
}
```

## 🐛 Debugging

### Avec lldb (macOS/Linux)

```bash
lldb ./build/tests/system_manager_tests
(lldb) run --test-case="*MonTest*"
(lldb) bt  # Backtrace en cas de crash
```

### Avec gdb (Linux)

```bash
gdb ./build/tests/system_manager_tests
(gdb) run --test-case="*MonTest*"
(gdb) backtrace  # Backtrace en cas de crash
```

### Avec Valgrind (détection de fuites mémoire)

```bash
valgrind --leak-check=full ./build/tests/system_manager_tests
```

## 📝 Bonnes pratiques

### Nommage des tests

```cpp
// ❌ Mauvais
TEST_CASE("Test1")

// ✅ Bon - Descriptif et clair
TEST_CASE("SystemManager - RegisterSystemCreatesNewSystem")
TEST_CASE("SystemManager - GetNonExistentSystemReturnsNullptr")
```

### Organisation avec SUBCASE

```cpp
TEST_CASE("SystemManager - CompleteWorkflow") {
    ecs::SystemManager manager;
    ecs::World world;

    SUBCASE("Enregistrement") {
        auto& sys = manager.registerSystem<TestSystem>();
        CHECK(sys.getUpdateCallCount() == 0);
    }

    SUBCASE("Utilisation") {
        manager.registerSystem<TestSystem>();
        manager.update(world, 0.016f);
        auto* sys = manager.getSystem<TestSystem>();
        CHECK(sys->getUpdateCallCount() == 1);
    }

    SUBCASE("Suppression") {
        manager.registerSystem<TestSystem>();
        manager.removeSystem<TestSystem>();
        CHECK(manager.getSystem<TestSystem>() == nullptr);
    }
}
```

### Organisation

- Un test = Une fonctionnalité
- Tests indépendants (pas de dépendances entre tests)
- Utiliser SUBCASE pour partager du code de setup
- Noms descriptifs

### Structure AAA (Arrange-Act-Assert)

```cpp
TEST_CASE("SystemManager - UpdateCallsSystems") {
    // Arrange - Préparer
    ecs::SystemManager manager;
    ecs::World world;
    manager.registerSystem<TestSystem>();

    // Act - Agir
    manager.update(world, 0.016f);

    // Assert - Vérifier
    auto* sys = manager.getSystem<TestSystem>();
    CHECK(sys->getUpdateCallCount() == 1);
}
```

## 🔗 Ressources

- [Documentation doctest](https://github.com/doctest/doctest)
- [Tutorial doctest](https://github.com/doctest/doctest/blob/master/doc/markdown/tutorial.md)
- [Exemples doctest](https://github.com/doctest/doctest/tree/master/examples)

## 💡 Tips

1. **Lancer les tests fréquemment** - Après chaque modification
2. **Un test échoue ?** - Utiliser `--test-case` pour l'isoler
3. **Tests lents ?** - Utiliser `--duration=true` pour identifier les lents
4. **CI/CD** - Intégrer `./build.sh test` dans votre pipeline
5. **Coverage** - Utiliser `gcov` ou `lcov` pour mesurer la couverture de code
6. **Compilation rapide** - doctest compile 10-20x plus vite que gtest

## 🎯 Checklist

- [ ] Tous les tests passent : `./build.sh test`
- [ ] Nouveau code = Nouveaux tests
- [ ] Tests nommés de façon descriptive
- [ ] Tests indépendants (pas d'ordre requis)
- [ ] Utiliser REQUIRE pour les conditions critiques
- [ ] Utiliser CHECK pour les vérifications non-critiques
- [ ] Pas de magic numbers (utiliser des constantes)
- [ ] Commentaires pour les tests complexes

## ⚡ Avantages de doctest

### Performance de compilation

```
Google Test: ~1.5s pour compiler 20 tests
doctest:     ~0.1s pour compiler 20 tests
```

### Taille du binaire

```
Google Test: ~2.5 MB
doctest:     ~800 KB
```

### Syntaxe simple

```cpp
// Google Test
TEST_F(MyFixture, MyTest) {
    ASSERT_NE(ptr, nullptr);
    EXPECT_EQ(value, 42);
}

// doctest
TEST_CASE("MyTest") {
    REQUIRE(ptr != nullptr);
    CHECK(value == 42);
}
```
