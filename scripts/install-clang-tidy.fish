#!/usr/bin/env fish

# Script pour installer clang-tidy et fixer automatiquement les warnings

echo "🔧 Installation de clang-tidy..."
brew install llvm

# Ajouter au PATH
set -gx PATH /opt/homebrew/opt/llvm/bin $PATH

echo "✅ clang-tidy installé!"
echo ""
echo "📝 Pour fixer tous les warnings dans vos tests:"
echo "  cd /Users/neauneau/Documents/tek3/rtype"
echo "  run-clang-tidy -p build -fix engineCore/tests/"
echo ""
echo "📝 Pour un seul fichier:"
echo "  clang-tidy --fix -p build engineCore/tests/ComponentStorageTests.cpp"
