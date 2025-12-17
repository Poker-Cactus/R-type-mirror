FROM ubuntu:22.04

# Éviter les prompts interactifs
ENV DEBIAN_FRONTEND=noninteractive

# Installer les dépendances système
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    git \
    python3 \
    python3-pip \
    pkg-config \
    libx11-dev \
    libxext-dev \
    libxrandr-dev \
    libxcursor-dev \
    libxi-dev \
    libxinerama-dev \
    libxxf86vm-dev \
    libxss-dev \
    libgl1-mesa-dev \
    libglu1-mesa-dev \
    libasound2-dev \
    libpulse-dev \
    libwayland-dev \
    libxkbcommon-dev \
    && rm -rf /var/lib/apt/lists/*

# Installer Conan
RUN pip3 install conan

# Configurer le profil Conan
RUN conan profile detect

# Copier le projet
WORKDIR /app
COPY . .

# Installer les dépendances Conan
RUN conan install . --output-folder=build --build=missing \
    -c tools.system.package_manager:mode=install \
    -c tools.system.package_manager:sudo=False

# Configurer CMake
WORKDIR /app/build
RUN cmake .. -DCMAKE_TOOLCHAIN_FILE=conan_toolchain.cmake -DCMAKE_BUILD_TYPE=Release

# Compiler le projet
RUN cmake --build . --config Release

# Revenir au répertoire principal
WORKDIR /app
