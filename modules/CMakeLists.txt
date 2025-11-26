# Dossier où mettre tous les .so/.dll
set(MODULE_OUTPUT_DIR "${CMAKE_SOURCE_DIR}/libs")

# Crée le dossier si il n'existe pas
file(MAKE_DIRECTORY ${MODULE_OUTPUT_DIR})

# Liste tous les sous-dossiers
file(GLOB MODULE_DIRS RELATIVE ${CMAKE_CURRENT_SOURCE_DIR} */)

foreach(mod_dir ${MODULE_DIRS})
    file(GLOB MODULE_SOURCES "${mod_dir}/*.cpp")
    if(MODULE_SOURCES)
        # Nom de la lib = nom du dossier du module
        string(TOUPPER ${mod_dir} MOD_NAME)

        add_library(${MOD_NAME} SHARED ${MODULE_SOURCES})
        target_include_directories(${MOD_NAME} PRIVATE
            ${CMAKE_SOURCE_DIR}/client/interfaces
        )

        # Exemple de dépendances : SDLBackend → SDL2
        if(${mod_dir} STREQUAL "SDLBackend")
            find_package(SDL2 REQUIRED)
            target_include_directories(${MOD_NAME} PRIVATE ${SDL2_INCLUDE_DIRS})
            target_link_libraries(${MOD_NAME} ${SDL2_LIBRARIES})
        endif()

        # Spécifie le dossier de sortie du .so/.dll
        set_target_properties(${MOD_NAME} PROPERTIES
            LIBRARY_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR}
            RUNTIME_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR} # Windows
            ARCHIVE_OUTPUT_DIRECTORY ${MODULE_OUTPUT_DIR} # Static lib si besoin
        )
    endif()
endforeach()
