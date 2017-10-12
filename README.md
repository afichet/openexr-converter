OpenEXR converter
=================

This tools in a simple utility to convert images files from OpenEXR to PNG 8bit sRGB files.

Compilation
===========

Dependencies
------------
You need to install OpenEXR to compile the tool:
* libopenexr-dev

### Ubuntu 16.04 LTS

```
sudo apt install libopenexr-dev
```

### Arch Linux

```
sudo pacman -S openexr
```

### Compilation
In order to compile in a folder build the tool, do:

```
mkdir build
cd build
cmake ..
make
```

