# Dossier de sortie des modules dans build/libs/
set(MODULE_OUTPUT_DIR "${CMAKE_BINARY_DIR}/libs")

# Crée le dossier libs s'il n'existe pas
file(MAKE_DIRECTORY ${MODULE_OUTPUT_DIR})

# Liste tous les sous-dossiers (SDL2, etc.)
file(GLOB MODULE_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} *)

foreach(module_dir ${MODULE_DIRS})
    # Ignore CMakeLists.txt et autres fichiers
    if(IS_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/${module_dir}")
        # Récupère tous les fichiers .cpp du module
        file(GLOB_RECURSE MODULE_SOURCES "${module_dir}/*.cpp")
        
        if(MODULE_SOURCES)
            # Nom de la bibliothèque = nom du dossier en minuscule + "_module"
            string(TOLOWER ${module_dir} module_name)
            set(LIB_NAME "${module_name}_module")
            
            message(STATUS "Building dynamic module: ${LIB_NAME}")
            
            add_library(${LIB_NAME} SHARED ${MODULE_SOURCES})
            
            # Include le dossier du module lui-même + client/include
            target_include_directories(${LIB_NAME} PRIVATE
                ${CMAKE_CURRENT_SOURCE_DIR}/${module_dir}
                ${CMAKE_SOURCE_DIR}/client/include
            )
            
            # Dépendances spécifiques selon le module
            if(module_dir STREQUAL "SDL2")
                # Conan's sdl_ttf may export different target names depending on version/options.
                set(_ttf_target "")
                if(TARGET SDL2_ttf::SDL2_ttf)
                    set(_ttf_target SDL2_ttf::SDL2_ttf)
                elseif(TARGET SDL2_ttf::SDL2_ttf-static)
                    set(_ttf_target SDL2_ttf::SDL2_ttf-static)
                elseif(TARGET sdl_ttf::sdl_ttf)
                    set(_ttf_target sdl_ttf::sdl_ttf)
                endif()

                if(_ttf_target STREQUAL "")
                    message(FATAL_ERROR "No SDL2_ttf target found (expected SDL2_ttf::SDL2_ttf, SDL2_ttf::SDL2_ttf-static or sdl_ttf::sdl_ttf)")
                endif()

                # Conan's sdl_mixer may export different target names depending on version/options.
                set(_mixer_target "")
                if(TARGET SDL2_mixer::SDL2_mixer)
                    set(_mixer_target SDL2_mixer::SDL2_mixer)
                elseif(TARGET SDL2_mixer::SDL2_mixer-static)
                    set(_mixer_target SDL2_mixer::SDL2_mixer-static)
                elseif(TARGET sdl_mixer::sdl_mixer)
                    set(_mixer_target sdl_mixer::sdl_mixer)
                endif()

                if(_mixer_target STREQUAL "")
                    message(WARNING "No SDL2_mixer target found - audio features will be disabled")
                    target_link_libraries(${LIB_NAME} PRIVATE SDL2::SDL2 ${_ttf_target} SDL2_image::SDL2_image)
                else()
                    target_link_libraries(${LIB_NAME} PRIVATE SDL2::SDL2 ${_ttf_target} SDL2_image::SDL2_image ${_mixer_target})
                endif()
            elseif(module_dir STREQUAL "SFML")
                # SFML dependencies - try CMake config first, fallback to pkg-config
                find_package(SFML 2 QUIET COMPONENTS graphics window system audio)
                if(SFML_FOUND)
                    target_link_libraries(${LIB_NAME} PRIVATE sfml-graphics sfml-window sfml-system sfml-audio)
                else()
                    # Fallback to pkg-config for systems without CMake config files
                    find_package(PkgConfig REQUIRED)
                    pkg_check_modules(SFML REQUIRED sfml-all)
                    target_link_libraries(${LIB_NAME} PRIVATE ${SFML_LIBRARIES})
                    target_include_directories(${LIB_NAME} PRIVATE ${SFML_INCLUDE_DIRS})
                endif()
            endif()
            
            # Place les .so/.dll dans build/
            set_target_properties(${LIB_NAME} PROPERTIES
                LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
                RUNTIME_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
                ARCHIVE_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
                PREFIX ""  # Pas de préfixe "lib"
            )
        endif()
    endif()
endforeach()
