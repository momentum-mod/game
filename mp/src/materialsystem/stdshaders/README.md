# HOW TO SET UP PERL
* Install Strawberry Perl and add it to your system path variable
* Open CMD and type `perl -MCPAN -e shell`
* In the cpan shell type `install String::CRC32`

# HOW TO ADD NEW SHADERS
*  Add the .cpp files to `game_shader_dx9_momentum.vpc`
*  Add .fxc files to the file lists `momentum_dx9_30.txt` and `momentum_dx9_20b.txt`

# HOW TO COMPILE SHADERS
*  Change the `SDKBINDIR` variable in `buildmomentumshaders.bat` to the absolute path of your bin folder (where `shadercompile.exe` is located)
*  Run `buildmomentumshaders.bat`
