Welcome to my simple Asteroids clone.

This was not intended to correctly replicate the gameplay in the original Asteroids. Instead I used this as a tool to practice development.

If you just want to try the game, there is a build for Windows in the release section on the github at https://github.com/LucidFrost/asteroids/releases.

If you want to tour the source code and build the project yourself, you will find it is very simple. The entire project is built using a single 'build.bat' file with minimal configuration. The structure of the source is also that of a unity build, which may be unusual to some people.

I don't use any header files and just #include the actual .cpp files in various locations. This is why I only build the single source file from the 'build.bat' file.

The only external build dependency is the compiler. The build file calls 'vcvarsall.bat' to import the MSVC compiler variables if they haven't already been imported. You can download the standalone compiler without the entire Visual Studio installation at http://landinghub.visualstudio.com/visual-cpp-build-tools.

To run the game after a build, you must be in the 'data' directory. I have created a simple 'run.bat' file to do this for you.

The build defaults to a debug build but you can specify 'release' on the command line to get a release build. This will also package the needed files into a 'release' folder at the root of the project. This is what is zip'd on the github releases page.

I plan to port the game to a number of different platforms, but the development platform is Windows. Because of this, the support for other platforms will be only what is required to build the release executable for that platform.

Ryan