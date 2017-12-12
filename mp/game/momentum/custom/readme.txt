This folder is automatically scanned when the game boots for VPK files or
subfolders.  Each subfolder or VPK is added as a search custom, so the files
inside those VPK's or subfolders will overide the default game files.

See gameinfo.txt for more details.

For example, you might have the following file structure:

	momentum/custom/my_custom_stuff/   <<< This subfolder will be added as a search custom
	momentum/custom/my_custom_stuff/models/custom_model.mdl
	momentum/custom/my_custom_stuff/materials/custom_material.vmt
	momentum/custom/my_custom_stuff/materials/vgui/custom_ui_thing.res
	momentum/custom/some_mod.vpk       <<< This VPK will be added as a search custom
	momentum/custom/another_mod.vpk    <<< This VPK will be added as a search custom


Mounting a VPK to the filesystem is more efficient that adding a subfolder,
because each time the engine neds to open a file, it will need to make a call to the
operating system call to search the folder.  VPKs can be searched by the engine much
more efficiently.  Each subfolder is a new search custom that must be checked each
time the engine tries to open a file.  So for optimal load times, always use VPK files
and don't make any subfolders in this folder!


Note that the following directory structure is NOT correct:

	momentum/custom/models/my_model.mdl

That will add the directory "momentum/custom/models" as a search path, in which case the
file my_model.mdl actually exists at the root of the game's virtual filesystem.
