# DungeonMinder
The game about being a small invisible fairy tasked with keeping an adventurer alive. Accompany the heroic '@' as he explores the dungeon! Watch as he stupidly picks fights with monsters far stronger than him! Subtly influence the resulting brawl to keep him alive long enough to reach the stairs! Don't ever expect to be thanked for your good plans and hard work.

#### :large_blue_circle: Development note
I rushed out this game in 7 days back in 2009 when I knew almost nothing about C++ and so this was originally some of the worst code I have written. In 2024 I have much more experience under my belt and so I am using this codebase as a refactoring exercise to uplift it to a reasonable level of quality. 

## Features
- An AI controls the hero, fighting monsters, finding treasure and enjoying victories
- Indirect gameplay, with many ways to help the hero win/avoid fights
- 10 levels of increasingly hard monsters
- 3-element mana system and spell specialisation system, giving a total of 27 spells to cast
- Boss fight, chosen randomly from three possibilities

DungeonMinder was originally a 7DRL that was successfully completed in the 7DRL challenge 2009. With the challenge is over some extra development continued in regards to bug fixing and expanding and refining the game in some areas. The original 7DRL version can still be found on the Downloads page of the game's original [Google Code](https://code.google.com/archive/p/dungeonminder) page.

Find out about the 7DRL challenge [here](https://7drl.com/).

## Changelog
Version 0.8, updated from Version 7DRL
- Specialisation system added, where the player is given the opportunity to replace old spells with brand new ones (check the readme). This brings the number of spells up to 27.
- 4 new items have been added
- Prompt and screen effect used to clearly indicate when the hero is dead, and instructs the player to restart or quit
- Added keyboards commands to restart the game and to toggle fullscreen
- Bar at the bottom shows the effects of TAB, F5, F8 and m
- The "Earthquake" scroll will no longer block the hero from reaching his goal
- Hero will now respond to enemies attacking him from behind when he is running after a distant target
- Increased the number of levels to 10, with a shallower difficulty increase between levels
- Increased the size of the screen to 60 lines
- Increased the message log to 8 lines by default, total message log to 28 lines
- Added new "Important" message type, which is shown in purple
- Added new speeches for when the hero picks a fight - Weakened the stats of several monsters