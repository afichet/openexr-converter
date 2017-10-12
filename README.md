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

This will generate into a directory `bin`, the executable, `openexr-converter`.

Usage
=====

```
   openexr-converter  [-a] -o <string> -i <string> [--] [--version]
                      [-h]
```

Where: 

```
   -a,  --ignore-alpha
     Ignore alpha channel

   -o <string>,  --output <string>
     (required)  Set file to save to

   -i <string>,  --input <string>
     (required)  Set file to convert

   --,  --ignore_rest
     Ignores the rest of the labeled arguments following this flag.

   --version
     Displays version information and exits.

   -h,  --help
     Displays usage information and exits.
```
