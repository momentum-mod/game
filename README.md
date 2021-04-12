![Momentum Mod](https://i.imgur.com/iR7p55N.png)

> Momentum Mod is a free, standalone game built on the Source Engine centralizing the major movement gametypes found in games like ~~Counter-Strike, Half-Life, Team Fortress, and Titanfall~~ only Quake 3 Defrag matters.

## Defrag best mode??

This is my modification of Momentum Mod 0.8.7 to include more accurate Quake 3 / CPMA movement. It uses a completely rewritten player movement code based on the original Quake 3 GPL source. These are its features:

* Accurate Q3 ground movement
* Quake-like jump inputs (buffered)
* VQ3 and CPMA airacceleration/aircontrol
* VQ3 and CPMA airstepping
* VQ3 swimming
* CPMA trimping
* Wallclipping
* CPMA double jumps
* Acceleration penalty for holding jump too long
* Exposed variables in cvars so the movement can be tweaked:
    * sv_maxairspeed: developer intended maximum airspeed
    * sv_cpm_physics: enables CPMA physics
    * sv_airstrafeaccelerate: acceleration when strafing with A/D (sv_cpm_physics 1)
    * sv_maxairstrafespeed: max speed when strafing with A/D (sv_cpm_physics 1)
    * sv_aircontrol: amount you can control yourself with W/S (sv_cpm_physics 1)
    * sv_aircontrolpower: aircontrol formula exponenet (sv_cpm_physics 1)
    * sv_wallcliptime: window of time after landing to wallclip
    * sv_airdecelerate: how much acceleration to lose when holding jump too long
    * sv_airdeceleretetime: how long jump must be held to decelerate
    * sv_swimscale: how much slower to move in water
    * sv_duckscale: how much slower to move while ducking

There are currently many missing features, like slick surfaces, water movement, velocity snapping, and others, and no Quake 3 weapons have been implemented yet. After a complete implementation is finished, I will gladly implement non-VQ3/CPMA mechanics such as Quake 2 additive jumps.

Run faster!!!
![run faster!!](https://cdn.donmai.us/original/34/0b/__rumia_and_sin_sack_touhou_drawn_by_okahi__340b2f7081c34a459fea5c95464e990f.jpg)