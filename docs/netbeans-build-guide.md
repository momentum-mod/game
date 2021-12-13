# NetBeans Build Guide

---
**NOTE**

This guide is for the `0.8.7` public preview build since the game is now closed-source.

---

This guide will offer a high-level overview of compiling the mod on Linux using the [NetBeans IDE](https://netbeans.apache.org/).

## Initial Setup

### With sudo

If project already has been previously compiled with `sudo`:
1. `cd` to `game/mp/`
2. run `sudo chown -R $(whoami):$(whoami) .` where `$(whoami)` _should_ be your local user (not `root`)

---
**NOTE**

These steps can also be combined into a single command like so:
```sh
sudo chown -R $(whoami):$(whoami) game/mp/
```

---

### Without sudo

1. Replace `groups=sudo` with `groups=<username>` in `/etc/schroot/chroot.d/steamrt_scout_i386.conf` where `<username>` is your local user (not `root`)
2. `cd` to `game/mp/src/`
3. Execute `./creategameprojects` (`chmod +x` if it cannot be run for some reason)
4. Run `schroot --chroot steamrt_scout_i386 -- make -f games.mak` to compile

---
**NOTE**

Step 1 can also be completed with something like:
```sh
sed -i "s/groups=sudo/groups=$(whoami)/g" /etc/schroot/chroot.d/steamrt_scout_i386.conf
```

---

## NetBeans Configuration

---
**NOTE**

The following steps require the configuration steps from above ([Initial Setup](#initial-setup)) except for compiling

---

1. Install NetBeans (netbeans package)
2. Navigate to `Tools` -> `Plugins`  
	- Navigate to `Settings` -> `Check NetBeans 8.2 Plugin Portal`  
	- Select `Available Plugins` -> `Install C/C++`  
3. Navigate to `Tools` -> `Options`  
	- Navigate to `Editor` -> `Formatting`  
		- `Language`: `All Languages`  
		- `Category`: `Tabs and Indents`  
		- `Expand Tabs to Spaces`: `Check`  
		- `Number of Spaces per Indent`: `4`  
		- `Tab Size`: `4`  
		- `Right Margin`: `120`  
	- Navigate to `Editor` -> `On Save`  
		- `Language`: `All Languages`  
		- `Reformat`: `None`  
		- `Remove Trailing Whitespace From`: `Modified Lines Only`  
4. Navigate to `File` -> `Open Project`  
	- Select `game/mp/src`  
5. Navigate to `File` -> Select `Project Properties`  
	- Under `General` -> Select `Add Source Folder` -> Select `game/mp/src`  
	- Under `Run` -> Configure `Run Directory`: `<path to MomentumDev>` (Server and Client configuration)  
	- Under `Run` -> Configure `Run Command` : `<change run parameters to liking>` (Server and Client configuration)  
6. Select `Run` -> `Build Project (F11)`  
7. Select `Debug` -> `Debug Project (F5)` (`Run` -> `Run Project (F6)` does not work for some reason)  
