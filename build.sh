#!/bin/bash

# ╔═══════════════════════════════════════════════════════════════════════════╗
# ║                           R-TYPE BUILD SYSTEM                             ║
# ╚═══════════════════════════════════════════════════════════════════════════╝

set -e

# ─────────────────────────────────────────────────────────────────────────────
# Colors & Styles
# ─────────────────────────────────────────────────────────────────────────────
readonly RED='\033[0;31m'
readonly GREEN='\033[0;32m'
readonly YELLOW='\033[1;33m'
readonly BLUE='\033[0;34m'
readonly MAGENTA='\033[0;35m'
readonly CYAN='\033[0;36m'
readonly WHITE='\033[1;37m'
readonly GRAY='\033[0;90m'
readonly BOLD='\033[1m'
readonly RESET='\033[0m'

# ─────────────────────────────────────────────────────────────────────────────
# Configuration
# ─────────────────────────────────────────────────────────────────────────────
readonly BUILD_DIR="build"
readonly BUILD_TYPE="Release"
readonly CONAN_PROFILE="conan_profile"

# ─────────────────────────────────────────────────────────────────────────────
# Helper Functions
# ─────────────────────────────────────────────────────────────────────────────
print_banner() {
    echo -e "${CYAN}${BOLD}"
    echo "╔═══════════════════════════════════════════════════════════╗"
    echo "║                                                           ║"
    echo "║                   🚀  R-TYPE BUILD  🚀                    ║"
    echo "║                                                           ║"
    echo "╚═══════════════════════════════════════════════════════════╝"
    echo -e "${RESET}"
}

print_step() {
    echo -e "${BOLD}${BLUE}▶${RESET} ${1}"
}

print_success() {
    echo -e "${GREEN}✓${RESET} ${1}"
}

print_error() {
    echo -e "${RED}✗${RESET} ${1}"
}

print_warning() {
    echo -e "${YELLOW}!${RESET} ${1}"
}

print_info() {
    echo -e "${GRAY}  ${1}${RESET}"
}

print_section() {
    echo ""
    echo -e "${MAGENTA}${BOLD}━━━ ${1} ━━━${RESET}"
    echo ""
}

print_result_banner() {
    local status=$1
    echo ""
    if [ "$status" = "success" ]; then
        echo -e "${GREEN}${BOLD}"
        echo "╔═══════════════════════════════════════════════════════════╗"
        echo "║              ✨  BUILD SUCCESSFUL!  ✨                    ║"
        echo "╚═══════════════════════════════════════════════════════════╝"
        echo -e "${RESET}"
        echo -e "\033[1;33m🍝 Mamacita, les pâtes au Crous 🍝\033[0m"
        echo ""
        echo -e "${WHITE}${BOLD}Run:${RESET}"
        echo -e "  ${GREEN}./build.sh server${RESET}  │  Server"
        echo -e "  ${BLUE}./build.sh client${RESET}  │  Client"
        echo -e "  ${MAGENTA}./build.sh editor${RESET}  │  Asset Editor"
        echo -e "  ${CYAN}./build.sh engine${RESET}  │  Engine Only"
        echo ""
    else
        echo -e "${RED}${BOLD}"
        echo "╔═══════════════════════════════════════════════════════════╗"
        echo "║                ❌  BUILD FAILED!  ❌                      ║"
        echo "╚═══════════════════════════════════════════════════════════╝"
        echo -e "${RESET}"
    fi
}

# ─────────────────────────────────────────────────────────────────────────────
# Build Output Formatter
# ─────────────────────────────────────────────────────────────────────────────
format_build_output() {
    while IFS= read -r line; do
        case "$line" in
            *"error:"*|*"Error:"*|*"ERROR"*)
                echo -e "${RED}${line}${RESET}" ;;
            *"warning:"*|*"Warning:"*)
                echo -e "${YELLOW}${line}${RESET}" ;;
            *"Built target"*)
                echo -e "${GREEN}✓ ${line}${RESET}" ;;
            *"Building"*"common"*|*"Linking"*"common"*)
                echo -e "${CYAN}🔧 ${line}${RESET}" ;;
            *"Building"*"network"*|*"Linking"*"network"*)
                echo -e "${MAGENTA}🌐 ${line}${RESET}" ;;
            *"Building"*"server"*|*"Linking"*"server"*)
                echo -e "${GREEN}🖥️  ${line}${RESET}" ;;
            *"Building"*"client"*|*"Linking"*"client"*)
                echo -e "${BLUE}💻 ${line}${RESET}" ;;
            *"Building"*"engineCore"*|*"Linking"*"engineCore"*)
                echo -e "${MAGENTA}🎮 ${line}${RESET}" ;;
            *"%]"*)
                echo -e "${WHITE}${line}${RESET}" ;;
            *)
                echo -e "${GRAY}${line}${RESET}" ;;
        esac
    done
}

# ─────────────────────────────────────────────────────────────────────────────
# Core Build Functions
# ─────────────────────────────────────────────────────────────────────────────
install_dependencies() {
    print_step "Installing dependencies with Conan..."
    
    if ! conan install . --output-folder="$BUILD_DIR" --build=missing --profile="$CONAN_PROFILE" 2>&1 | \
        grep -E "(Installing|Already installed|Install finished)" | \
        while read -r line; do print_info "$line"; done; then
        print_error "Conan installation failed!"
        return 1
    fi
    
    # Fix toolchain path to be absolute
    if [ -f "$BUILD_DIR/CMakePresets.json" ]; then
        local toolchain_path="$(pwd)/$BUILD_DIR/conan_toolchain.cmake"
        if [[ "$OSTYPE" == "darwin"* ]]; then
            sed -i '' "s|\"toolchainFile\": \"conan_toolchain.cmake\"|\"toolchainFile\": \"$toolchain_path\"|g" "$BUILD_DIR/CMakePresets.json"
        else
            sed -i "s|\"toolchainFile\": \"conan_toolchain.cmake\"|\"toolchainFile\": \"$toolchain_path\"|g" "$BUILD_DIR/CMakePresets.json"
        fi
    fi
    
    print_success "Dependencies installed"
}

configure_cmake() {
    print_step "Configuring CMake..."
    
    local log_file="/tmp/cmake_config_$$.log"
    local cmake_args=("-S" "." "-B" "$BUILD_DIR" "-DCMAKE_BUILD_TYPE=$BUILD_TYPE")
    
    # Add toolchain file if it exists (from Conan)
    if [ -f "$BUILD_DIR/conan_toolchain.cmake" ]; then
        cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/conan_toolchain.cmake")
    fi
    
    if ! cmake "${cmake_args[@]}" > "$log_file" 2>&1; then
        # Check for generator mismatch
        if grep -q "Does not match the generator used previously" "$log_file"; then
            print_warning "CMake cache mismatch, cleaning..."
            rm -f "$BUILD_DIR/CMakeCache.txt"
            rm -rf "$BUILD_DIR/CMakeFiles"
            
            if ! cmake "${cmake_args[@]}" > "$log_file" 2>&1; then
                print_error "CMake configuration failed!"
                cat "$log_file"
                rm -f "$log_file"
                return 1
            fi
        else
            print_error "CMake configuration failed!"
            cat "$log_file"
            rm -f "$log_file"
            return 1
        fi
    fi
    
    rm -f "$log_file"
    print_success "CMake configured"
}

compile_project() {
    print_step "Compiling project..."
    echo ""
    
    cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j8 2>&1 | format_build_output
    return ${PIPESTATUS[0]}
}

compile_editor() {
    # Setup ImGui if not present
    if [ ! -d "assetEditor/vendor/imgui" ]; then
        print_step "Downloading ImGui..."
        ./assetEditor/setup_imgui.sh
        print_success "ImGui downloaded"
        echo ""
    fi
    
    print_step "Compiling Asset Editor..."
    
    # Configure with editor flag
    local cmake_args=("-S" "." "-B" "$BUILD_DIR" "-DCMAKE_BUILD_TYPE=$BUILD_TYPE" "-DBUILD_ASSET_EDITOR=ON")
    [ -f "$BUILD_DIR/conan_toolchain.cmake" ] && cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/conan_toolchain.cmake")
    
    cmake "${cmake_args[@]}" > /dev/null 2>&1 || {
        print_error "CMake configuration failed!"
        return 1
    }
    
    # Build just the editor
    cmake --build "$BUILD_DIR" --target assetEditor -j8 2>&1 | format_build_output
    return ${PIPESTATUS[0]}
}

compile_editor() {
    # Setup ImGui if not present
    if [ ! -d "assetEditor/vendor/imgui" ]; then
        print_step "Downloading ImGui..."
        ./assetEditor/setup_imgui.sh
        print_success "ImGui downloaded"
        echo ""
    fi
    
    print_step "Compiling Asset Editor..."
    
    # Configure with editor flag
    local cmake_args=("-S" "." "-B" "$BUILD_DIR" "-DCMAKE_BUILD_TYPE=$BUILD_TYPE" "-DBUILD_ASSET_EDITOR=ON")
    [ -f "$BUILD_DIR/conan_toolchain.cmake" ] && cmake_args+=("-DCMAKE_TOOLCHAIN_FILE=$BUILD_DIR/conan_toolchain.cmake")
    
    cmake "${cmake_args[@]}" > /dev/null 2>&1 || {
        print_error "CMake configuration failed!"
        return 1
    }
    
    # Build just the editor
    cmake --build "$BUILD_DIR" --target assetEditor 2>&1 | format_build_output
    return ${PIPESTATUS[0]}
}

# ─────────────────────────────────────────────────────────────────────────────
# Main Commands
# ─────────────────────────────────────────────────────────────────────────────
cmd_build() {
    print_banner
    
    # Check if incremental build is possible
    if [ -d "$BUILD_DIR" ] && [ -f "$BUILD_DIR/CMakeCache.txt" ]; then
        print_section "Incremental Build"
        if ! compile_project; then
            print_result_banner "failure"
            exit 1
        fi
        echo ""
        if ! compile_editor; then
            print_result_banner "failure"
            exit 1
        fi
    else
        print_section "Full Build"
        install_dependencies || exit 1
        echo ""
        configure_cmake || exit 1
        echo ""
        if ! compile_project; then
            print_result_banner "failure"
            exit 1
        fi
        echo ""
        if ! compile_editor; then
            print_result_banner "failure"
            exit 1
        fi
    fi
    
    print_result_banner "success"
}

cmd_clean() {
    print_banner
    print_section "Clean"
    
    print_step "Removing build artifacts..."
    
    if [ -d "$BUILD_DIR" ]; then
        # Remove only compiled objects, keep conan files
        find "$BUILD_DIR" -name "CMakeFiles" -type d -exec rm -rf {} + 2>/dev/null || true
        find "$BUILD_DIR" -name "*.a" -type f -delete 2>/dev/null || true
        find "$BUILD_DIR" -name "*.o" -type f -delete 2>/dev/null || true
        rm -f "$BUILD_DIR/server/server" 2>/dev/null || true
        rm -f "$BUILD_DIR/client/client" 2>/dev/null || true
        rm -f "$BUILD_DIR/libs/"*.dylib 2>/dev/null || true
        rm -f "$BUILD_DIR/libs/"*.so 2>/dev/null || true
        print_success "Build artifacts cleaned"
    else
        print_info "Nothing to clean"
    fi
    
    # Remove imgui.ini if it exists
    if [ -f "imgui.ini" ]; then
        rm -f "imgui.ini"
        print_info "Removed imgui.ini"
    fi
}

cmd_fclean() {
    print_banner
    print_section "Full Clean"
    
    print_step "Removing all generated files..."
    
    local removed=0
    
    if [ -d "$BUILD_DIR" ]; then
        rm -rf "$BUILD_DIR"
        print_info "Removed $BUILD_DIR/"
        removed=1
    fi
    
    if [ -f "CMakeUserPresets.json" ]; then
        rm -f "CMakeUserPresets.json"
        print_info "Removed CMakeUserPresets.json"
        removed=1
    fi
    
    if [ -f "CMakePresets.json" ]; then
        rm -f "CMakePresets.json"
        print_info "Removed CMakePresets.json"
        removed=1
    fi
    
    if [ $removed -eq 1 ]; then
        print_success "Full clean completed"
    else
        print_info "Nothing to clean"
    fi
}

cmd_re() {
    cmd_fclean
    echo ""
    cmd_build
}

cmd_test() {
    print_banner
    print_section "Running Tests"
    
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Run './build.sh' first."
        exit 1
    fi
    
    print_step "Configuring tests..."
    (
        cd "$BUILD_DIR"
        cmake .. -DBUILD_TESTS=ON > /dev/null 2>&1
    ) || { print_error "CMake configuration failed!"; exit 1; }
    
    print_step "Building tests..."
    cmake --build "$BUILD_DIR" --target component_signature_tests component_manager_tests entity_tests entity_manager_tests component_storage_tests system_manager_tests world_tests -j8 2>&1 | tail -5
    
    print_step "Executing tests..."
    echo ""
    
    (
        cd "$BUILD_DIR"
        ctest --test-dir engineCore/tests --output-on-failure 2>&1 | while IFS= read -r line; do
            case "$line" in
                *"PASSED"*|*"passed"*)
                    echo -e "${GREEN}${line}${RESET}" ;;
                *"FAILED"*|*"failed"*)
                    echo -e "${RED}${line}${RESET}" ;;
                *"Test"*|*"Start"*)
                    echo -e "${CYAN}${line}${RESET}" ;;
                *)
                    echo -e "${GRAY}${line}${RESET}" ;;
            esac
        done
    )
    
    local result=$?
    
    echo ""
    if [ $result -eq 0 ]; then
        echo -e "${GREEN}${BOLD}✓ All tests passed${RESET}"
    else
        echo -e "${RED}${BOLD}✗ Some tests failed${RESET}"
        exit 1
    fi
}

cmd_run_server() {
    # Ensure build directory and configuration exist
    if [ ! -d "$BUILD_DIR" ] || [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
        install_dependencies || exit 1
        echo ""
        configure_cmake || exit 1
        echo ""
    fi

    print_step "Building Server..."
    cmake --build "$BUILD_DIR" --target server --config "$BUILD_TYPE" -j8 2>&1 | format_build_output
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        print_error "Server build failed"
        exit 1
    fi

    if [ ! -f "$BUILD_DIR/server/server" ]; then
        print_error "Server binary not found after build."
        exit 1
    fi

    exec "$BUILD_DIR/server/server" "$@"
}

cmd_run_client() {
    # Ensure build directory and configuration exist
    if [ ! -d "$BUILD_DIR" ] || [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
        install_dependencies || exit 1
        echo ""
        configure_cmake || exit 1
        echo ""
    fi

    print_step "Building Client..."
    cmake --build "$BUILD_DIR" --target client --config "$BUILD_TYPE" -j8 2>&1 | format_build_output
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        print_error "Client build failed"
        exit 1
    fi

    if [ ! -f "$BUILD_DIR/client/client" ]; then
        print_error "Client binary not found after build."
        exit 1
    fi

    exec "$BUILD_DIR/client/client" "$@"
}

cmd_run_editor() {
    # Ensure build directory and configuration exist
    if [ ! -d "$BUILD_DIR" ] || [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
        install_dependencies || exit 1
        echo ""
        configure_cmake || exit 1
        echo ""
    fi

    print_step "Building Asset Editor..."
    cmake --build "$BUILD_DIR" --target assetEditor --config "$BUILD_TYPE" -j8 2>&1 | format_build_output
    if [ ${PIPESTATUS[0]} -ne 0 ]; then
        print_error "Asset Editor build failed"
        exit 1
    fi

    if [ ! -f "$BUILD_DIR/assetEditor/assetEditor" ]; then
        print_error "Asset Editor binary not found after build."
        exit 1
    fi

    exec "$BUILD_DIR/assetEditor/assetEditor" "$@"
}

cmd_engine() {
    print_banner
    print_section "Building Engine"
    
    # Check if conan dependencies are installed
    if [ ! -d "$BUILD_DIR" ] || [ ! -f "$BUILD_DIR/CMakeCache.txt" ]; then
        install_dependencies || exit 1
        echo ""
        configure_cmake || exit 1
        echo ""
    fi
    
    print_step "Compiling engineCore..."
    echo ""
    
    # Build just the engine
    cmake --build "$BUILD_DIR" --target engineCore -j8 2>&1 | format_build_output
    
    if [ ${PIPESTATUS[0]} -eq 0 ]; then
        echo ""
        echo -e "${GREEN}${BOLD}"
        echo "╔═══════════════════════════════════════════════════════════╗"
        echo "║           ✨  ENGINE BUILD SUCCESSFUL!  ✨                ║"
        echo "╚═══════════════════════════════════════════════════════════╝"
        echo -e "${RESET}"
    else
        echo ""
        echo -e "${RED}${BOLD}"
        echo "╔═══════════════════════════════════════════════════════════╗"
        echo "║              ❌  ENGINE BUILD FAILED!  ❌                ║"
        echo "╚═══════════════════════════════════════════════════════════╝"
        echo -e "${RESET}"
        exit 1
    fi
}

cmd_launch() {
    print_banner
    print_section "Quick Launch"
    
    # Quick recompile
    if [ ! -d "$BUILD_DIR" ]; then
        print_error "Build directory not found. Run './build.sh' first."
        exit 1
    fi
    
    print_step "Recompiling..."
    cmake --build "$BUILD_DIR" --config "$BUILD_TYPE" -j8 > /dev/null 2>&1
    
    if [ $? -ne 0 ]; then
        print_error "Compilation failed!"
        exit 1
    fi
    
    print_success "Compilation done"
    
    if [ ! -f "$BUILD_DIR/server/server" ] || [ ! -f "$BUILD_DIR/client/client" ]; then
        print_error "Server or Client not found after compilation."
        exit 1
    fi
    
    print_step "Launching Server & Client..."
    echo ""
    
    # Create temp files for output
    local server_log="/tmp/rtype_server_$$.log"
    local client_log="/tmp/rtype_client_$$.log"
    
    # Launch server in background
    "$BUILD_DIR/server/server" > "$server_log" 2>&1 &
    local server_pid=$!
    
    # Small delay to let server start
    sleep 1
    
    # Launch client in background
    "$BUILD_DIR/client/client" > "$client_log" 2>&1 &
    local client_pid=$!
    
    # Function to cleanup on exit
    cleanup() {
        echo ""
        print_step "Shutting down..."
        kill $server_pid 2>/dev/null || true
        kill $client_pid 2>/dev/null || true
        rm -f "$server_log" "$client_log"
        exit 0
    }
    
    trap cleanup SIGINT SIGTERM
    
    # Tail both logs with colored prefixes
    tail -f "$server_log" | while IFS= read -r line; do
        echo -e "${GREEN}[SERVER]${RESET} $line"
    done &
    local tail_server=$!
    
    tail -f "$client_log" | while IFS= read -r line; do
        echo -e "${BLUE}[CLIENT]${RESET} $line"
    done &
    local tail_client=$!
    
    # Wait for processes
    wait $server_pid $client_pid 2>/dev/null
    
    # Cleanup
    kill $tail_server $tail_client 2>/dev/null || true
    rm -f "$server_log" "$client_log"
}

cmd_help() {
    echo -e "${CYAN}${BOLD}R-Type Build System${RESET}"
    echo ""
    echo -e "${WHITE}Usage:${RESET} ./build.sh [command]"
    echo ""
    echo -e "${WHITE}Commands:${RESET}"
    echo -e "  ${GREEN}(none)${RESET}      Build the project (incremental if possible)"
    echo -e "  ${GREEN}-l, --launch${RESET} Recompile and launch server & client"
    echo -e "  ${GREEN}re${RESET}          Full clean and rebuild"
    echo -e "  ${GREEN}clean${RESET}       Remove build artifacts"
    echo -e "  ${GREEN}fclean${RESET}      Remove all generated files"
    echo -e "  ${GREEN}test${RESET}        Build and run tests"
    echo -e "  ${GREEN}server${RESET}      Run the server"
    echo -e "  ${GREEN}client${RESET}      Run the client"
    echo -e "  ${GREEN}editor${RESET}      Run the Asset Editor"
    echo -e "  ${GREEN}engine${RESET}      Build engineCore only"
    echo -e "  ${GREEN}help${RESET}        Show this help message"
    echo ""
}

# ─────────────────────────────────────────────────────────────────────────────
# Entry Point
# ─────────────────────────────────────────────────────────────────────────────
main() {
    local command="${1:-build}"
    shift 2>/dev/null || true
    
    case "$command" in
        ""|"all"|"build")   cmd_build ;;
        "-l"|"--launch")    cmd_launch ;;
        "re"|"rebuild")     cmd_re ;;
        "clean")            cmd_clean ;;
        "fclean")           cmd_fclean ;;
        "test"|"tests")     cmd_test "$@" ;;
        "server")           cmd_run_server "$@" ;;
        "client")           cmd_run_client "$@" ;;
        "editor")           cmd_run_editor "$@" ;;
        "engine")           cmd_engine ;;
        "help"|"-h"|"--help") cmd_help ;;
        *)
            print_error "Unknown command: $command"
            echo ""
            cmd_help
            exit 1
            ;;
    esac
}

main "$@"
