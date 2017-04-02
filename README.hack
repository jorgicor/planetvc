How to work with the package
============================

To work with the game source I usually do it from two different folders: in one
folder I have the package with the source, in another a make the builds.

For example, I have the package in a pkg folder:

****
pkg/
****

And then I create a src.build folder to make the builds:

****
pkg/
src.build/
****

Then I use two terminals: one is usually inside pkg/src to modify the source,
the other is inside src.build.

Inside src.build, I first run

    > ../pkg/src/configure --enable-x11 --enable-sdl-datadir
    > make

so the generated build objects are in this folder and not in the original
pkg/src folder.

Each time I change the source files and I want to compile, I do it inside
src.build.

I run the game as well from inside src.build, but first we need the game data
in that folder. For that, we have two options: either we do a symbolic link
named 'data' to point to ../pkg/data; or we use ../pkg/mkdatatest to build copy
the data to our folder. mkdatatest can be used to copy the data for specific
builds, for example to test the data that goes into a mobile version or a demo
version.

Android
=======

To make the android builds I do something similar. We have a folder with

****
pkg/
src.build/
****

Inside this folder we can run 'pkg/mkandroid demo (or full) version_code
version path_to_sdl'.  That will create a android.build.demo or and
android.build.full folder and inside those folders the android project will be
copied, patched, and compilation will happen.

For example:

****
pkg/mkandroid full 34 1.23 ../SDL2.0.5
****

Will be used to make the full version for Android, with code 34, version number 1.23 and assuming that the SDL 2.0.5 source code is at ../SDL2.0.5 .

----

April 2017
Jorge Giner
