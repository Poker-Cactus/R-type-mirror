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
    rm -rf build CMakeUserPresets.json CMakePresets.json
    echo -e "${GRAY}\n  ======== Cleaning ========${RESET}"
    echo -e "${GRAY}  Delete folder build/${RESET}"
    echo -e "${GRAY}  Delete file CMakeUserPresets.json${RESET}"
    echo -e "${GRAY}  Delete file CMakePresets.json\n${RESET}"
    echo -e "\033[1;33m🪄  Tutty Propo 🪄\033[0m"
    exit 0
fi

# Check if only rebuild is requested
if [ "$1" == "rebuild" ] || [ "$1" == "re" ]; then
    print_step "${BUILD}" "Rebuilding project..."
    cmake --build build --config Release 2>&1 | while IFS= read -r line; do
        if [[ $line == *"engineCore"* ]]; then
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
    COVERAGE_FLAG="$3"

    # Check if coverage is requested (either as 2nd or 3rd argument)
    if [ "$TEST_FILTER" == "--coverage" ]; then
        COVERAGE_FLAG="--coverage"
        TEST_FILTER=""
    fi

    if [ -n "$TEST_FILTER" ]; then
        print_step "🧪" "Running tests matching: ${TEST_FILTER}..."
    else
        if [ "$COVERAGE_FLAG" == "--coverage" ]; then
            print_step "🧪" "Running all tests with coverage..."
        else
            print_step "🧪" "Running all tests..."
        fi
    fi
    echo ""

    # Check if build directory exists
    if [ ! -d "build" ]; then
        print_error "Build directory not found! Run './build.sh' first."
        exit 1
    fi

    # Clean coverage_report before building tests
    if [ -d "build/coverage_report" ]; then
        print_step "🧹" "Cleaning coverage_report directory..."
        rm -rf build/coverage_report
    fi

    # If coverage is requested, rebuild with coverage flags
    if [ "$COVERAGE_FLAG" == "--coverage" ]; then
        print_step "🔧" "Configuring build with coverage and tests enabled..."
        cd build
        cmake .. -DENABLE_COVERAGE=ON -DBUILD_TESTS=ON > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            print_error "CMake configuration with coverage failed!"
            cd ..
            exit 1
        fi
        print_success "Coverage configuration applied!"
        echo ""

        print_step "🔨" "Rebuilding tests with coverage..."
        cmake --build . --target component_signature_tests component_manager_tests entity_tests entity_manager_tests component_storage_tests system_manager_tests world_tests 2>&1 | tail -10
        if [ $? -ne 0 ]; then
            print_error "Build with coverage failed!"
            cd ..
            exit 1
        fi
        print_success "Tests rebuilt with coverage!"
        echo ""
        cd ..
    else
        # Build tests without coverage
        print_step "🔧" "Configuring build with tests enabled..."
        cd build
        cmake .. -DBUILD_TESTS=ON > /dev/null 2>&1
        if [ $? -ne 0 ]; then
            print_error "CMake configuration failed!"
            cd ..
            exit 1
        fi
        print_success "Configuration applied!"
        echo ""

        print_step "🔨" "Building tests..."
        cmake --build . --target component_signature_tests component_manager_tests entity_tests entity_manager_tests component_storage_tests system_manager_tests world_tests 2>&1 | tail -10
        if [ $? -ne 0 ]; then
            print_error "Build failed!"
            cd ..
            exit 1
        fi
        print_success "Tests rebuilt!"
        echo ""
        cd ..
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
        ctest --test-dir engineCore/tests --output-on-failure --verbose 2>&1 | while IFS= read -r line; do
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

        # Generate coverage report if requested
        if [ "$COVERAGE_FLAG" == "--coverage" ]; then
            print_step "📊" "Generating coverage report..."
            echo ""

            cd build

            # Find all .gcda files (coverage data from executed code)
            GCDA_FILES=$(find ./engineCore/tests -name '*.gcda' 2>/dev/null)

            if [ -z "$GCDA_FILES" ]; then
                print_error "No coverage data found. Make sure tests are built with coverage flags."
                echo -e "${YELLOW}Hint: You may need to rebuild with coverage enabled:${RESET}"
                echo -e "  ${CYAN}cmake -DENABLE_COVERAGE=ON ..${RESET}"
                cd ..
                exit 1
            fi

            # Create coverage directory
            mkdir -p coverage_report
            cd coverage_report

            # Run gcov on test files
            echo -e "${CYAN}Generating coverage reports...${RESET}"

            # Process each test's gcda file directly
            for gcda_file in $GCDA_FILES; do
                gcov "../${gcda_file}" > /dev/null 2>&1 || true
            done

            # Generate summary
            echo ""
            echo -e "${CYAN}${BOLD}Code Coverage Summary:${RESET}"
            echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"

            total_files=0
            global_total_lines=0
            global_exec_lines=0

            # Create temporary files for data aggregation
            temp_data=$(mktemp)
            temp_folders=$(mktemp)

            # Parse .gcov files for source coverage
            for gcov_file in *.gcov; do
                if [ -f "$gcov_file" ]; then
                    # Extract the full path from the gcov file (format: "Source:/path/to/file")
                    full_path=$(head -n 1 "$gcov_file" | grep "Source:" | sed "s/.*Source://" | tr -d ' ')

                    # Skip system headers, conan dependencies, and doctest
                    if [[ "$full_path" == *"/Applications/"* ]] || \
                       [[ "$full_path" == *"/usr/"* ]] || \
                       [[ "$full_path" == *"/.conan"* ]] || \
                       [[ "$full_path" == *"doctest"* ]] || \
                       [[ -z "$full_path" ]]; then
                        continue
                    fi

                    # Check if it's a project source file
                    if [[ "$full_path" == *"/engineCore/"* ]] || \
                       [[ "$full_path" == *"/client/"* ]] || \
                       [[ "$full_path" == *"/server/"* ]] || \
                       [[ "$full_path" == *"/common/"* ]] || \
                       [[ "$full_path" == *"/network/"* ]]; then

                        # Skip test files
                        if [[ "$full_path" == *"/tests/"* ]]; then
                            continue
                        fi

                        # Extract folder name (simple approach)
                        if [[ "$full_path" == *"/engineCore/"* ]]; then
                            folder_name="engineCore"
                        elif [[ "$full_path" == *"/client/"* ]]; then
                            folder_name="client"
                        elif [[ "$full_path" == *"/server/"* ]]; then
                            folder_name="server"
                        elif [[ "$full_path" == *"/common/"* ]]; then
                            folder_name="common"
                        elif [[ "$full_path" == *"/network/"* ]]; then
                            folder_name="network"
                        else
                            continue
                        fi

                        # Extract filename
                        filename=$(basename "$full_path")

                        # Calculate coverage
                        lines_total=$(grep -E "^[[:space:]]*[0-9#-]+:[[:space:]]*[0-9]+" "$gcov_file" | grep -v "^[[:space:]]*-:" | wc -l | tr -d ' ')
                        lines_exec=$(grep -E "^[[:space:]]*[1-9][0-9]*:[[:space:]]*[0-9]+" "$gcov_file" | wc -l | tr -d ' ')

                        if [ "$lines_total" -gt 0 ]; then
                            # Store file data: folder|filename|exec|total
                            echo "${folder_name}|${filename}|${lines_exec}|${lines_total}" >> "$temp_data"

                            # Track unique folders
                            echo "$folder_name" >> "$temp_folders"

                            # Accumulate global totals
                            global_total_lines=$((global_total_lines + lines_total))
                            global_exec_lines=$((global_exec_lines + lines_exec))

                            total_files=$((total_files + 1))
                        fi
                    fi
                fi
            done

            if [ $total_files -eq 0 ]; then
                echo -e "  ${YELLOW}No source coverage data found.${RESET}"
                rm -f "$temp_data" "$temp_folders"
            else
                # First, display uncovered lines summary
                echo ""
                echo -e "${YELLOW}${BOLD}⚠️  Uncovered Lines Report:${RESET}"
                echo -e "${GRAY}   ────────────────────────────────────────────────────${RESET}"

                has_uncovered=0
                for gcov_file in *.gcov; do
                    if [ -f "$gcov_file" ]; then
                        # Extract the full path
                        full_path=$(head -n 1 "$gcov_file" | grep "Source:" | sed "s/.*Source://" | tr -d ' ')

                        # Check if it's a project source file
                        if [[ "$full_path" == *"/engineCore/"* ]] || \
                           [[ "$full_path" == *"/client/"* ]] || \
                           [[ "$full_path" == *"/server/"* ]] || \
                           [[ "$full_path" == *"/common/"* ]] || \
                           [[ "$full_path" == *"/network/"* ]]; then

                            # Skip test files
                            if [[ "$full_path" == *"/tests/"* ]]; then
                                continue
                            fi

                            filename=$(basename "$full_path")

                            # Find uncovered lines (marked with #####)
                            uncovered_lines=$(grep -n "^[[:space:]]*#####:" "$gcov_file" | grep -v "}" | head -10)

                            if [ -n "$uncovered_lines" ]; then
                                has_uncovered=1
                                echo ""
                                echo -e "   ${RED}📄 ${filename}${RESET}"

                                # Display up to 10 uncovered lines with their content
                                echo "$uncovered_lines" | while IFS=: read -r line_num line_content; do
                                    # Extract just the line number from gcov format
                                    actual_line=$(echo "$line_content" | sed 's/^[[:space:]]*#####:[[:space:]]*//' | awk '{print $1}')
                                    # Extract the code content
                                    code_content=$(echo "$line_content" | sed 's/^[[:space:]]*#####:[[:space:]]*[0-9]*://')

                                    if [ -n "$code_content" ] && [ "$code_content" != "}" ]; then
                                        printf "      ${GRAY}Line %-4s${RESET} %s\n" "$actual_line" "$code_content"
                                    fi
                                done | head -10

                                # Count total uncovered
                                total_uncovered=$(echo "$uncovered_lines" | wc -l | tr -d ' ')
                                if [ "$total_uncovered" -gt 10 ]; then
                                    echo -e "      ${GRAY}... and $((total_uncovered - 10)) more uncovered lines${RESET}"
                                fi
                            fi
                        fi
                    fi
                done

                if [ $has_uncovered -eq 0 ]; then
                    echo -e "   ${GREEN}✨ All lines are covered!${RESET}"
                fi

                echo ""
                echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"

                # Get unique folders
                folders=$(sort -u "$temp_folders")

                # Display coverage by folder
                for folder in $folders; do
                    # Calculate folder totals
                    folder_total=0
                    folder_exec=0

                    while IFS='|' read -r fld fname fexec ftotal; do
                        if [ "$fld" = "$folder" ]; then
                            folder_total=$((folder_total + ftotal))
                            folder_exec=$((folder_exec + fexec))
                        fi
                    done < "$temp_data"

                    if [ $folder_total -gt 0 ]; then
                        folder_coverage=$(awk "BEGIN {printf \"%.1f\", ($folder_exec/$folder_total)*100}")

                        # Color based on folder coverage
                        if (( $(echo "$folder_coverage >= 90" | bc -l) )); then
                            folder_color="$GREEN"
                        elif (( $(echo "$folder_coverage >= 70" | bc -l) )); then
                            folder_color="$YELLOW"
                        else
                            folder_color="$RED"
                        fi

                        echo ""
                        echo -e "${MAGENTA}${BOLD}📦 ${folder}${RESET} - ${folder_color}${folder_coverage}%${RESET} (${folder_exec}/${folder_total} lines)"
                        echo -e "${GRAY}   ────────────────────────────────────────────────────${RESET}"

                        # Display individual files for this folder
                        while IFS='|' read -r fld fname fexec ftotal; do
                            if [ "$fld" = "$folder" ]; then
                                file_coverage=$(awk "BEGIN {printf \"%.1f\", ($fexec/$ftotal)*100}")

                                # Color based on file coverage
                                if (( $(echo "$file_coverage >= 90" | bc -l) )); then
                                    file_color="$GREEN"
                                elif (( $(echo "$file_coverage >= 70" | bc -l) )); then
                                    file_color="$YELLOW"
                                else
                                    file_color="$RED"
                                fi

                                printf "   ${file_color}%5.1f%%${RESET} - %-40s ${GRAY}(%s/%s)${RESET}\n" "$file_coverage" "$fname" "$fexec" "$ftotal"
                            fi
                        done < "$temp_data"
                    fi
                done

                # Display global coverage
                if [ $global_total_lines -gt 0 ]; then
                    global_coverage=$(awk "BEGIN {printf \"%.1f\", ($global_exec_lines/$global_total_lines)*100}")

                    # Color based on global coverage
                    if (( $(echo "$global_coverage >= 90" | bc -l) )); then
                        global_color="$GREEN"
                    elif (( $(echo "$global_coverage >= 70" | bc -l) )); then
                        global_color="$YELLOW"
                    else
                        global_color="$RED"
                    fi

                    echo ""
                    echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
                    echo -e "${WHITE}${BOLD}🌍 TOTAL COVERAGE:${RESET} ${global_color}${BOLD}${global_coverage}%${RESET} (${global_exec_lines}/${global_total_lines} lines)"
                fi

                # Cleanup temp files
                rm -f "$temp_data" "$temp_folders"
            fi

            echo -e "${CYAN}━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━${RESET}"
            echo ""
            print_success "Coverage report generated!"
            echo -e "${GRAY}Detailed coverage files: build/coverage_report/*.gcov${RESET}"
            echo ""

            cd ../..
        fi
    else
        echo ""
        print_error "Some tests failed!"
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

# Fix the toolchain file path in build/CMakePresets.json to be absolute
if [ -f "build/CMakePresets.json" ]; then
    if command -v sed &> /dev/null; then
        if [[ "$OSTYPE" == "darwin"* ]]; then
            # macOS sed - use absolute path for toolchain file
            sed -i '' 's|"toolchainFile": "conan_toolchain.cmake"|"toolchainFile": "'$(pwd)'/build/conan_toolchain.cmake"|g' build/CMakePresets.json
        else
            # Linux sed
            sed -i 's|"toolchainFile": "conan_toolchain.cmake"|"toolchainFile": "'$(pwd)'/build/conan_toolchain.cmake"|g' build/CMakePresets.json
        fi
    fi
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
    if [[ $line == *"engineCore"* ]]; then
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
