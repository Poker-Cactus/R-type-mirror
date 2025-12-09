#!/bin/bash

# Colors
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
BLUE='\033[0;34m'
MAGENTA='\033[0;35m'
CYAN='\033[0;36m'
WHITE='\033[1;37m'
GRAY='\033[0;90m'
BOLD='\033[1m'
RESET='\033[0m'

# Emojis
ROCKET="🚀"
GEAR="⚙️"
PACKAGE="📦"
SERVER="🖥️"
CLIENT="💻"
ENGINE="🎮"
COMMON="🔧"
CHECK="✅"
CROSS="❌"
CLEAN="🧹"
BUILD="🔨"
SPARKLES="✨"

# Banner
echo -e "${CYAN}${BOLD}"
echo "╔═══════════════════════════════════════════════════════════╗"
echo "║                                                           ║"
echo "║                   ${ROCKET}  R-TYPE BUILD  ${ROCKET}                    ║"
echo "║                                                           ║"
echo "╚═══════════════════════════════════════════════════════════╝"
echo -e "${RESET}\n"

# Function to print colored messages
print_step() {
    echo -e "${BOLD}${BLUE}${1}${RESET} ${2}"
}

print_success() {
    echo -e "${GREEN}${CHECK} ${1}${RESET}"
}

print_error() {
    echo -e "${RED}${CROSS} ${1}${RESET}"
}

print_building() {
    echo -e "${YELLOW}${BUILD} Building ${1}...${RESET}"
}

# Check if clean is requested
if [ "$1" == "clean" ]; then
    print_step "${CLEAN}" "Cleaning build directory..."
    rm -rf build CMakeUserPresets.json
    print_success "Clean complete!"
    echo ""
    exit 0
fi

# Check if only rebuild is requested
if [ "$1" == "rebuild" ]; then
    print_step "${BUILD}" "Rebuilding project..."
    cmake --build build --config Release 2>&1 | while IFS= read -r line; do
        if [[ $line == *"engine_core"* ]]; then
            echo -e "${MAGENTA}${ENGINE} $line${RESET}"
        elif [[ $line == *"common"* ]]; then
            echo -e "${CYAN}${COMMON} $line${RESET}"
        elif [[ $line == *"server"* ]]; then
            echo -e "${GREEN}${SERVER} $line${RESET}"
        elif [[ $line == *"client"* ]]; then
            echo -e "${BLUE}${CLIENT} $line${RESET}"
        elif [[ $line == *"Built target"* ]]; then
            echo -e "${GREEN}${CHECK} $line${RESET}"
        elif [[ $line == *"error"* ]] || [[ $line == *"Error"* ]]; then
            echo -e "${RED}${CROSS} $line${RESET}"
        else
            echo -e "${GRAY}$line${RESET}"
        fi
    done
    
    if [ $? -eq 0 ]; then
        echo ""
        print_success "Build successful!"
        echo -e "\n${SPARKLES}${GREEN}${BOLD} Ready to launch! ${SPARKLES}${RESET}\n"
    else
        echo ""
        print_error "Build failed!"
        exit 1
    fi
    exit 0
fi

# Check if coverage is requested
if [ "$1" == "coverage" ]; then
    print_step "${CHECK}" "Running coverage..."

    # Ensure we are in Debug mode
    print_step "${GEAR}" "Configuring CMake in Debug mode..."
    conan install . --output-folder=build --build=missing --profile=conan_profile -s build_type=Debug

    # Source the build environment to make kcov available
    if [ -f "build/conanbuild.sh" ]; then
        source build/conanbuild.sh
    fi

    cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake

    print_building "tests"
    cmake --build build --target unit_tests

    print_step "${ROCKET}" "Generating coverage report..."
    cmake --build build --target coverage

    if [ $? -eq 0 ]; then
        print_success "Coverage report generated at build/coverage/index.html"
        # Try to open the report
        if command -v xdg-open &> /dev/null; then
            xdg-open build/coverage/index.html
        elif command -v open &> /dev/null; then
            open build/coverage/index.html
        else
            echo -e "${YELLOW}Open build/coverage/index.html to view the report${RESET}"
        fi
    else
        print_error "Coverage generation failed!"
        exit 1
    fi
    exit 0
fi

# Full build process
print_step "${PACKAGE}" "Installing dependencies with Conan..."
conan install . --output-folder=build --build=missing --profile=conan_profile 2>&1 | grep -E "(Installing|Already installed|Install finished)" | while read -r line; do
    echo -e "${GRAY}  $line${RESET}"
done

if [ $? -ne 0 ]; then
    print_error "Conan installation failed!"
    exit 1
fi
print_success "Dependencies installed!"
echo ""

print_step "${GEAR}" "Configuring CMake..."
cmake --preset conan-release > /tmp/cmake_config.log 2>&1
CMAKE_STATUS=$?

# If CMake failed, check if it's due to generator mismatch and retry after cleanup
if [ $CMAKE_STATUS -ne 0 ]; then
    if grep -q "Does not match the generator used previously" /tmp/cmake_config.log; then
        print_step "${CLEAN}" "Cleaning incompatible CMake cache..."
        rm -f build/CMakeCache.txt
        rm -rf build/CMakeFiles
        print_step "${GEAR}" "Retrying CMake configuration..."
        cmake --preset conan-release > /tmp/cmake_config.log 2>&1
        CMAKE_STATUS=$?
    fi
fi

if [ $CMAKE_STATUS -ne 0 ]; then
    print_error "CMake configuration failed!"
    echo -e "${YELLOW}Error details:${RESET}"
    cat /tmp/cmake_config.log
    exit 1
fi
print_success "CMake configured!"
echo ""

print_step "${BUILD}" "Compiling project..."
echo ""

cmake --build build --config Release 2>&1 | while IFS= read -r line; do
    if [[ $line == *"engine_core"* ]]; then
        if [[ $line == *"Building"* ]]; then
            print_building "${MAGENTA}Engine Core${RESET}"
        else
            echo -e "${MAGENTA}${ENGINE} $line${RESET}"
        fi
    elif [[ $line == *"common"* ]]; then
        if [[ $line == *"Building"* ]]; then
            print_building "${CYAN}Common${RESET}"
        else
            echo -e "${CYAN}${COMMON} $line${RESET}"
        fi
    elif [[ $line == *"server"* ]]; then
        if [[ $line == *"Building"* ]]; then
            print_building "${GREEN}Server${RESET}"
        else
            echo -e "${GREEN}${SERVER} $line${RESET}"
        fi
    elif [[ $line == *"client"* ]]; then
        if [[ $line == *"Building"* ]]; then
            print_building "${BLUE}Client${RESET}"
        else
            echo -e "${BLUE}${CLIENT} $line${RESET}"
        fi
    elif [[ $line == *"Built target"* ]]; then
        echo -e "${GREEN}${BOLD}${CHECK} $line${RESET}"
    elif [[ $line == *"error"* ]] || [[ $line == *"Error"* ]]; then
        echo -e "${RED}${CROSS} $line${RESET}"
    elif [[ $line =~ ^[[:space:]]*\[.*%\] ]]; then
        echo -e "${WHITE}$line${RESET}"
    else
        echo -e "${GRAY}$line${RESET}"
    fi
done

if [ $? -eq 0 ]; then
    echo ""
    echo -e "${GREEN}${BOLD}"
    echo "╔═══════════════════════════════════════════════════════════╗"
    echo "║                                                           ║"
    echo "║              ${SPARKLES}  BUILD SUCCESSFUL!  ${SPARKLES}                 ║"
    echo "║                                                           ║"
    echo "╚═══════════════════════════════════════════════════════════╝"
    echo -e "${RESET}\n"
    
    echo -e "${WHITE}${BOLD}To run the application:${RESET}"
    echo -e "  ${SERVER}  Server: ${GREEN}./build/server/server${RESET}"
    echo -e "  ${CLIENT}  Client: ${BLUE}./build/client/client${RESET}"
    echo ""
else
    echo ""
    print_error "Build failed!"
    exit 1
fi
