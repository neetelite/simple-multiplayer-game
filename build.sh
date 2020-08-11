#compiler="-Wall"
compiler="-g -O0 -DBUILD_DEBUG -DOS_LINUX"
linker="-lm -lX11 -lGL"

gcc $compiler $linker main.c -o ../run/gtd_linux_x64
gcc $compiler -lm netcode/server.c -o ../run/server
