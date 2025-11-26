#!/bin/bash

echo "==> Running clang-tidy..."

if [ ! -f build/compile_commands.json ]; then
    echo "compile_commands.json not found. Did the build step run?"
    exit 1
fi

run-clang-tidy -p build -quiet
STATUS=$?

if [ $STATUS -ne 0 ]; then
    echo "❌ clang-tidy failed."
    exit 1
fi

echo "✅ clang-tidy OK."
exit 0
