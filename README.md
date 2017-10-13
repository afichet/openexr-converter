OpenEXR converter
=================

These tools are simple utilities to convert images files from OpenEXR to PNG 8bit sRGB files and TIFF to OpenEXR.

Compilation
===========

Dependencies
------------
You need to install OpenEXR to compile the tool:
* libopenexr
* libtiff

### Ubuntu 16.04 LTS

```
sudo apt install libopenexr-dev libtiff5-dev
```

### Arch Linux

```
sudo pacman -S openexr libtiff
```

### Compilation
In order to compile in a folder build the tool, do:

```
mkdir build
cd build
cmake ..
make
```

After compilation,
* `exr-png` will be created in `src/exr-png` folder.
* `tiff-exr` will be created in `src/tiff-exr` folder.

To install, do:

```
sudo make install
```

Usage
=====

OpenEXR to PNG
--------------

```
exr-png  [-g <real>] [-a] -o <string> -i <string> [--] [--version]
		 [-h]
```

Where: 

```
   -g <real>,  --gamma <real>
     Set the gamma correction (default sRGB)

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


TIFF to OpenEXR
---------------

```
tiff-exr <input_tiff_file> <output_exr_file>
```