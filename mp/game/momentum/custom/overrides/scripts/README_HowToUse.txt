In order to override the momentum weapons with TF2 (or other custom) weapons/particles/etc,
you will need to rename the corresponding file of interest to remove the extra suffix.

For example, if you want the TF2 stock rocket launcher, you would rename the
`weapon_momentum_rocketlauncher_tf2_stock.txt`
file to:
`weapon_momentum_rocketlauncher.txt`
The weapon would be overridden for as long as this text file is in the `overrides/scripts/` folder.

If you want it removed, simply delete the txt file or rename it to something else, as the game is 
explicitly looking for "weapon_momentum_rocketlauncher.txt" when loading the weapon script.

This system is in place so that any updates we do to the official script inside the base momentum
folder will not affect your overrides, causing you to lose the override with every game update/steam
file verification.