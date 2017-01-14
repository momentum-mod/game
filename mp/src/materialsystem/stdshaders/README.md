# HOW TO COMPILE SHADERS FOR MOMENTUM
*  Follow the [instructions](https://developer.valvesoftware.com/wiki/Shader_Authoring) in this section of the VDC on shader authoring, installing Perl, changing %PATH%, etc
*  Stop after setting Path ENV variable.
*  Change the `SDKBINDIR` var in `buildmomentumshaders.bat` to match your Source SDK installation
*  run `buildmomentumshaders.bat`

# HOW TO ADD NEW SHADERS
*  Add the .cpp files to `game_shader_dx9_momentum.vpc`
*  Add .fxc files to the file lists `momentum_dx9_30.txt` and `momentum_dx9_20b.txt`
*  run `buildmomentumshaders.bat`, re-run VPC for game projects, compile shader DLL.