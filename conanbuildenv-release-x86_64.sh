script_folder="/home/seila/R-type-mirror"
echo "echo Restoring environment" > "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
for v in ACLOCAL_PATH AUTOMAKE_CONAN_INCLUDES AUTOCONF AUTORECONF AUTOHEADER AUTOM4TE M4
do
   is_defined="true"
   value=$(printenv $v) || is_defined="" || true
   if [ -n "$value" ] || [ -n "$is_defined" ]
   then
       echo export "$v='$value'" >> "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
   else
       echo unset $v >> "$script_folder/deactivate_conanbuildenv-release-x86_64.sh"
   fi
done

export ACLOCAL_PATH="$ACLOCAL_PATH:/home/seila/.conan2/p/b/libtod47084472f51f/p/res/aclocal"
export AUTOMAKE_CONAN_INCLUDES="$AUTOMAKE_CONAN_INCLUDES:/home/seila/.conan2/p/b/libtod47084472f51f/p/res/aclocal"
export AUTOCONF="/home/seila/.conan2/p/autoc958b1a03a355a/p/bin/autoconf"
export AUTORECONF="/home/seila/.conan2/p/autoc958b1a03a355a/p/bin/autoreconf"
export AUTOHEADER="/home/seila/.conan2/p/autoc958b1a03a355a/p/bin/autoheader"
export AUTOM4TE="/home/seila/.conan2/p/autoc958b1a03a355a/p/bin/autom4te"
export M4="/home/seila/.conan2/p/m43fe61932e2887/p/bin/m4"