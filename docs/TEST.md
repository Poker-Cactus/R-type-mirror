# Technologies et Fonctionnalités (Mocking, BDD...)

## Google Test (GTest) : La solution "Tout inclus"

- **Mocking** : C'est le seul à inclure son propre framework : gMock. C'est puissant, mais verbeux.
- **Fonctionnalités uniques** : "Death Tests" (vérifier qu'un programme crash ou abort correctement), tests paramétrés (générer des tests avec des données différentes), tests typés.
- **Style** : Plutôt orienté xUnit (Setup/Teardown).

## Catch2 : Le champion du BDD (Behavior Driven Development)

- **Mocking** : Aucun. Tu dois utiliser une lib externe comme Trompeloeil ou FakeIt.
- **Fonctionnalités uniques** : Le système de SECTION imbriquées est génial. Il permet de réutiliser le code de setup naturellement sans créer de classes. Il supporte nativement les tags pour n'exécuter que certains tests (`./tests [api]`).
- **Style** : BDD (SCENARIO, GIVEN, WHEN, THEN) ou classique.

## doctest : L'essentiel, vite fait

- **Mocking** : Aucun. Comme Catch2, besoin d'une lib externe.
- **Fonctionnalités** : Très similaire à Catch2 (macros, sections), mais avec un sous-ensemble de fonctionnalités pour garantir la vitesse.
- **Style** : Similaire à Catch2.

**🏆 Vainqueur Structure de test** : Catch2 (pour ses Sections).

---

# Temps de Compilation

C'est souvent le point de douleur en C++ (les templates de tests sont lourds).

## doctest : 🚀 Le plus rapide

Il a été conçu spécifiquement pour ça. Il est extrêmement léger. C'est le choix n°1 si tu as des milliers de tests et que tu ne veux pas attendre 10 minutes à chaque build.

## Catch2 (v3) : ⚖️ Moyen

La v2 était lente (tout dans le header). La v3 a séparé l'interface de l'implémentation (bibliothèque statique), ce qui a grandement amélioré les choses, mais reste plus lourd que doctest.

## Google Test : 🐢 Le plus lent

C'est une grosse librairie avec beaucoup d'héritage et de templates complexes. Sur des petits projets, c'est invisible, mais sur des projets géants, l'impact est réel.

**🏆 Vainqueur** : doctest (par KO).

---

# Syntaxe

## Google Test : 👴 Datée / Verbeuse

Tu dois choisir la bonne macro : `EXPECT_EQ(a, b)`, `EXPECT_STREQ(str1, str2)`, `ASSERT_GT(x, y)`. Si tu te trompes de macro, le message d'erreur est moins clair ou le test ne compile pas.

**Exemple** : `ASSERT_EQ(users.size(), 5);`

## Catch2 & doctest : ✨ Moderne / Magique

Ils utilisent les "expression templates". Une seule macro suffit, ils décomposent le code C++ pour comprendre la comparaison.

**Exemple** : `REQUIRE(users.size() == 5);` → Si ça rate, il t'affichera : "Expected 5, but got 3". C'est beaucoup plus naturel à écrire.

**🏆 Vainqueur** : Égalité Catch2 / doctest.

---

# Intégration dans le code

## doctest : 🎯 Unique en son genre

Il permet d'écrire les tests directement dans tes fichiers `.cpp` ou `.h` de production, juste à côté de la fonction que tu testes.

Au moment de compiler ton exécutable final (Release), tu ajoutes un flag (`-DDOCTEST_CONFIG_DISABLE`) et le code de test disparaît complètement du binaire. C'est génial pour garder la doc et le test proches du code.

## GTest & Catch2 : 📁 Classique

Ils imposent généralement d'avoir un dossier `tests/` séparé avec ses propres fichiers `.cpp` qui compilent un exécutable à part.

**🏆 Vainqueur** : doctest (pour la flexibilité), GTest (pour la séparation stricte standard).

---

# Intégration CMake et Conan

Les trois sont des standards industriels, donc l'intégration est excellente pour tous, mais avec des nuances.

## Avec Conan (gestionnaire de paquets)

Dans ton `conanfile.txt` ou `conanfile.py`, c'est trivial pour les trois :

- `gtest/1.14.0`
- `catch2/3.5.2`
- `doctest/2.4.11`

## Avec CMake

### Google Test : Très verbeux historiquement, mais standardisé maintenant

```cmake
find_package(GTest REQUIRED)
target_link_libraries(mon_test GTest::gtest_main)
# GTest nécessite souvent de gérer les threads manuellement sous Linux (-lpthread)
```

### Catch2 (v3) : Moderne et propre

```cmake
find_package(Catch2 3 REQUIRED)
target_link_libraries(mon_test Catch2::Catch2WithMain)
```

### doctest : Le plus simple

Car souvent utilisé en "Header Only" (pas de lib à linker si on veut, juste des includes).

```cmake
find_package(doctest REQUIRED)
target_link_libraries(mon_test doctest::doctest)
```

**🏆 Vainqueur** : Égalité parfaite. Conan et CMake gèrent les trois comme des citoyens de première classe.

---

# 🏁 Résumé Final : Lequel choisir ?

| Critère | Google Test | Catch2 (v3) | doctest |
|---------|-------------|-------------|---------|
| **Techno/Mock** | ✅ Le plus complet (gMock inclus) | ❌ Mock externe requis | ❌ Mock externe requis |
| **Compile Time** | 🔴 Lent | 🟡 Moyen | 🟢 Très Rapide |
| **Syntaxe** | 🔴 Macros spécifiques (ASSERT_EQ) | 🟢 Naturelle (REQUIRE) | 🟢 Naturelle (CHECK) |
| **Code Location** | 📁 Fichiers séparés | 📁 Fichiers séparés | 🎯 Dans le code source ou séparé |
| **CMake/Conan** | ✅ Excellent | ✅ Excellent | ✅ Excellent |

## Mon avis pour ton projet :

Doctest est là lib qui parait la plus adaptée pour ce projet, car en plus de permettre de documenter le code grâce au test directement écrit dans le code source.
Cette lib est aussi la plus efficace au niveau de la compilation et la syntaxe est plus naturelle.
