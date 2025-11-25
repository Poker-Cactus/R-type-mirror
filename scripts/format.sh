#!/bin/bash
find . -name "*.hpp" -o -name "*.h" -o -name "*.cpp" | xargs clang-format -i
echo "Code formatted ✨"
