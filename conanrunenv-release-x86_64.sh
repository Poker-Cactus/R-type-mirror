script_folder="/home/seila/R-type-mirror"
echo "echo Restoring environment" > "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
for v in LD_LIBRARY_PATH DYLD_LIBRARY_PATH ACLOCAL_PATH AUTOMAKE_CONAN_INCLUDES PATH M4 ALSA_CONFIG_DIR OPENSSL_MODULES
do
   is_defined="true"
   value=$(printenv $v) || is_defined="" || true
   if [ -n "$value" ] || [ -n "$is_defined" ]
   then
       echo export "$v='$value'" >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
   else
       echo unset $v >> "$script_folder/deactivate_conanrunenv-release-x86_64.sh"
   fi
done

export LD_LIBRARY_PATH="/home/seila/.conan2/p/b/sdl_mee1b8a41ebf3a/p/lib:$LD_LIBRARY_PATH"
export DYLD_LIBRARY_PATH="/home/seila/.conan2/p/b/sdl_mee1b8a41ebf3a/p/lib:$DYLD_LIBRARY_PATH"
export ACLOCAL_PATH="$ACLOCAL_PATH:/home/seila/.conan2/p/b/libtod47084472f51f/p/res/aclocal"
export AUTOMAKE_CONAN_INCLUDES="$AUTOMAKE_CONAN_INCLUDES:/home/seila/.conan2/p/b/libtod47084472f51f/p/res/aclocal"
export PATH="/home/seila/.conan2/p/automf2e79eeca022d/p/bin:/home/seila/.conan2/p/autoc958b1a03a355a/p/bin:/home/seila/.conan2/p/m43fe61932e2887/p/bin:$PATH"
export M4="/home/seila/.conan2/p/m43fe61932e2887/p/bin/m4"
export ALSA_CONFIG_DIR="/home/seila/.conan2/p/b/libal35d65e201b94e/p/res/alsa"
export OPENSSL_MODULES="/home/seila/.conan2/p/b/opensa3679badf7682/p/lib/ossl-modules"