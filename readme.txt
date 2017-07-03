Welcome to my simple Asteroids clone.

This was not intended to correctly replicate the gameplay in the original Asteroids. Instead I used this as a tool to practice development.

If you just want to try the game, there is a build for Windows in the release section on the github (https://github.com/LucidFrost/asteroids/releases).

If you want to tour the source code and build the project yourself, you will find it is very simple. The entire project is built using a single .bat file with minimal configuration. The structure of the source is also that of a "unity" build which may be unusual to some people.

I don't use any header files and just #include the actual .cpp files in various locations. This is why I only build the single source file from the build.bat file.

After the build, it auto starts the game. If you want to turn this off just get rid of the last line in the build.bat file.

Ryan