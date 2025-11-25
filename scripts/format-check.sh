#!/bin/bash

echo "==> Running clang-format check..."

FILES=$(git ls-files '*.cpp' '*.hpp' '*.h')

if [ -z "$FILES" ]; then
    echo "No C++ files found."
    exit 0
fi

clang-format --dry-run --Werror $FILES
STATUS=$?

if [ $STATUS -ne 0 ]; then
    echo "❌ clang-format failed."
    exit 1
fi

echo "✅ clang-format OK."
exit 0
