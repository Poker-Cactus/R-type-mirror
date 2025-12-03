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
    echo -e "\033[1;34m🧹 MrPropre au charbon 🧹\033[0m"
    rm -rf build CMakeUserPresets.json
    echo -e "${GRAY}\n  ======== Cleaning ========${RESET}"
    echo -e "${GRAY}  Delete folder build/${RESET}"
    echo -e "${GRAY}  Delete file CMakeUserPresets.json\n${RESET}"
    echo -e "\033[1;33m🪄  Tutty Propo 🪄\033[0m"
    exit 0
fi

# Check if only rebuild is requested
if [ "$1" == "rebuild" ] || [ "$1" == "re" ]; then
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

# Check if test is requested
if [ "$1" == "test" ]; then
    TEST_FILTER="$2"

    if [ -n "$TEST_FILTER" ]; then
        print_step "🧪" "Running tests matching: ${TEST_FILTER}..."
    else
        print_step "🧪" "Running all tests..."
    fi
    echo ""

    # Check if build directory exists
    if [ ! -d "build" ]; then
        print_error "Build directory not found! Run './build.sh' first."
        exit 1
    fi

    # Build test command
    if [ -n "$TEST_FILTER" ]; then
        # Run filtered tests directly with doctest
        TEST_CMD="./build/tests/system_manager_tests --test-case=\"*${TEST_FILTER}*\""
        echo -e "${CYAN}Command: ${TEST_CMD}${RESET}\n"
        eval $TEST_CMD 2>&1 | while IFS= read -r line; do
            if [[ $line == *"passed"* ]] || [[ $line == *"SUCCESS"* ]]; then
                echo -e "${GREEN}$line${RESET}"
            elif [[ $line == *"failed"* ]] || [[ $line == *"FAILURE"* ]] || [[ $line == *"ERROR"* ]]; then
                echo -e "${RED}$line${RESET}"
            elif [[ $line == *"TEST CASE"* ]] || [[ $line == *"doctest"* ]]; then
                echo -e "${CYAN}$line${RESET}"
            else
                echo -e "${GRAY}$line${RESET}"
            fi
        done
        TEST_RESULT=${PIPESTATUS[0]}
    else
        # Run all tests with CTest
        cd build
        ctest --test-dir engine_core/tests --output-on-failure --verbose 2>&1 | while IFS= read -r line; do
            if [[ $line == *"PASSED"* ]] || [[ $line == *"passed"* ]] || [[ $line == *"SUCCESS"* ]]; then
                echo -e "${GREEN}$line${RESET}"
            elif [[ $line == *"FAILED"* ]] || [[ $line == *"failed"* ]] || [[ $line == *"Error"* ]]; then
                echo -e "${RED}$line${RESET}"
            elif [[ $line == *"Test"* ]] || [[ $line == *"Start"* ]]; then
                echo -e "${CYAN}$line${RESET}"
            else
                echo -e "${GRAY}$line${RESET}"
            fi
        done
        TEST_RESULT=$?
        cd ..
    fi

    if [ $TEST_RESULT -eq 0 ]; then
        echo ""
        echo -e "${GREEN}${BOLD}"
        echo "╔═══════════════════════════════════════════════════════════╗"
        echo "║                                                           ║"
        echo "║              ✅  ALL TESTS PASSED!  ✅                    ║"
        echo "║                                                           ║"
        echo "╚═══════════════════════════════════════════════════════════╝"
        echo -e "${RESET}\n"
    else
        echo ""
        print_error "Some tests failed!"
        exit 1
    fi
    exit 0
fi

# Check if example is requested
if [ "$1" == "example" ]; then
    EXAMPLE_NAME="$2"

    if [ -z "$EXAMPLE_NAME" ]; then
        print_error "Usage: ./build.sh example <example_name>"
        echo -e "${CYAN}Available examples in build/examples/:${RESET}"
        if [ -d "build/examples" ]; then
            for example in build/examples/*; do
                if [ -x "$example" ] && [ -f "$example" ]; then
                    basename "$example" | sed 's/Example$//' | sed 's/_/ /g' | awk '{print "  - " $0}'
                fi
            done
        else
            echo -e "${GRAY}  (No examples built yet)${RESET}"
        fi
        exit 1
    fi

    # Check if build directory exists
    if [ ! -d "build" ]; then
        print_step "${ROCKET}" "Build directory not found. Building project..."
        ./build.sh
        if [ $? -ne 0 ]; then
            print_error "Build failed!"
            exit 1
        fi
    fi

    # Ensure build/examples exists
    if [ ! -d "build/examples" ]; then
        print_error "Examples directory not found. Building project..."
        cmake --build build 2>&1 | tail -5
        if [ $? -ne 0 ]; then
            print_error "Build failed!"
            exit 1
        fi
    fi

    # Search for matching example (case-insensitive, supports CamelCase, snake_case, etc.)
    EXAMPLE_PATH=""
    EXAMPLE_FOUND=""

    # Normalize search: convert to lowercase and remove separators
    SEARCH_NORMALIZED=$(echo "$EXAMPLE_NAME" | tr '[:upper:]' '[:lower:]' | tr -d '_-')

    for example_file in build/examples/*; do
        if [ -f "$example_file" ]; then
            # Get basename and remove "Example" suffix if present
            example_base=$(basename "$example_file")
            example_normalized=$(echo "$example_base" | sed 's/Example$//' | tr '[:upper:]' '[:lower:]' | tr -d '_-')

            if [ "$example_normalized" == "$SEARCH_NORMALIZED" ]; then
                EXAMPLE_PATH="$example_file"
                EXAMPLE_FOUND="$example_base"
                break
            fi
        fi
    done

    if [ -z "$EXAMPLE_PATH" ]; then
        print_error "Example not found: ${EXAMPLE_NAME}"
        echo -e "${CYAN}Available examples:${RESET}"
        for example in build/examples/*; do
            if [ -x "$example" ] && [ -f "$example" ]; then
                basename "$example" | sed 's/Example$//' | sed 's/_/ /g' | awk '{print "  - " $0}'
            fi
        done
        exit 1
    fi

    # Check if example executable exists and is executable
    if [ ! -x "$EXAMPLE_PATH" ]; then
        print_step "${BUILD}" "Example not executable or needs rebuild. Building..."

        # Extract target name from filename
        TARGET_NAME=$(basename "$EXAMPLE_PATH")

        cmake --build build --target "$TARGET_NAME" 2>&1 | while IFS= read -r line; do
            if [[ $line == *"Built target"* ]]; then
                echo -e "${GREEN}${CHECK} $line${RESET}"
            elif [[ $line == *"error"* ]] || [[ $line == *"Error"* ]]; then
                echo -e "${RED}${CROSS} $line${RESET}"
            else
                echo -e "${GRAY}$line${RESET}"
            fi
        done

        if [ $? -ne 0 ]; then
            print_error "Build failed!"
            exit 1
        fi
        echo ""
    fi

    # Run the example
    print_step "${SPARKLES}" "Running example: ${EXAMPLE_FOUND}"
    echo ""
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
    echo ""

    "$EXAMPLE_PATH"
    EXAMPLE_RESULT=$?

    echo ""
    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"

    if [ $EXAMPLE_RESULT -eq 0 ]; then
        echo ""
        print_success "Example completed successfully!"
    else
        echo ""
        print_error "Example exited with code: $EXAMPLE_RESULT"
        exit $EXAMPLE_RESULT
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
cmake --preset conan-release > /dev/null 2>&1
if [ $? -ne 0 ]; then
    print_error "CMake configuration failed!"
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
    echo "║              ${SPARKLES}  BUILD SUCCESSFUL!  ${SPARKLES}                    ║"
    echo "║                                                           ║"
    echo "╚═══════════════════════════════════════════════════════════╝"
    echo -e "${RESET}\n"

    echo -e "${WHITE}${BOLD}To run the application:${RESET}"
    echo -e "  ${SERVER}  Server: ${GREEN}./build/server/server${RESET}"
    echo -e "  ${CLIENT}  Client: ${BLUE}./build/client/client${RESET}"
    echo ""
    echo -e "\033[1;33m🍝 Mamacita, les pâtes au Crous 🍝\033[0m"
else
    echo ""
    print_error "Build failed!"
    exit 1
fi
