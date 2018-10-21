# Multicore-Programming [![Build Status](https://travis-ci.com/prince-chrismc/Multicore-Programming.svg?branch=master)](https://travis-ci.com/prince-chrismc/Multicore-Programming)
This is a C++ repository to contain my work for Multicore Programming (COMP426) from Concordia Universirty during Fall 2018.

### Table of Contents
- [Modules](#Modules)
   - [Assignments](#Assignments)
   - [External Libraries](#Libraries)
- [Building](#Building)
- [Sample](#Sample)
  
## Modules
#### [Assignments](https://github.com/prince-chrismc/Multicore-Programming/tree/master/Assignments)
- [Galaxy Collider](https://github.com/prince-chrismc/Multicore-Programming/tree/master/Assignments/Galaxy-Collider)

#### Libraries
- [Computer Graphics Library](https://github.com/prince-chrismc/Computer-Graphics-Libraries)
   - GLEW
   - GLFW
   - GLM
- [Intel Thread Building Blocks](https://github.com/prince-chrismc/tbb)

## Building
This is a very simple library, it is very easy to build and configure and the 'third-party' dependencies are self contained. Make sure to recursively clone the repository to obtain a copy of the submodules. To build and install, use CMake to generate the files required for your platform and execute the appropriate build command or procedure. No cmake options/settings should be changed or configured.

Unix Systems: The command is make which produce a release and debug version depending on the CMAKE_BUILD_TYPE specified.
Windows Systems: The usual MSVC files can be build through the IDE or via command line interface.

There is no installation of any kind.

## Sample
For a quick preview of what the rendering looks like, here's a GIF thats been speed up...

![preview](https://github.com/prince-chrismc/Multicore-Programming/blob/master/Docs/20181021_131943_1_1.gif?raw=true)
