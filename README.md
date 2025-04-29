# Glint

WIP repo until everything works.

Based on [Hakkun](https://github.com/fruityloops1/LibHakkun).

### Features

- F3 menu reimplementation (currently always enabled- sorry)

### Building (WIP)

Follow build instructions in [Hakkun](https://github.com/fruityloops1/LibHakkun).

- `make` to build
- `make clean` to clean build folder

- After building, the atmosphere folder in `build/sd` contains the mod

### Symbols

- `tools/ida/export_names_as_syms.py` can be used to create `main.sym`. I recommend pulling the decomp symbols into your database with the script in the decomp before re-exporting here.

### TODO

- Get SkyGridLevelSource working. Right now, it's just a renamed NetherFlatLevelSource. I have code to pull in for the generation but initing it instead of a OverworldLevelSource causes the thread to loop infinitely.

- Get custom blocks working - right now it's saving and loading the data but the game doesn't do anything with it.
