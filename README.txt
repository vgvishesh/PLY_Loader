plyloader.cpp
plyloader.h

these are the header and implementation files offering functionality to read 3D data file specified in .ply format.

FEATURES:
1. Can read .ply files for meshes, and Point cloud data.
2. Can read files having any combination of vertex attributes like colors, normals, and positions.

NOT SUPPORTED:
Since .ply is a user defined format, and any attribute can be added according to the need of the users. So, instead of making a fixed reader supporting all types of .ply files and increasing the complexity of the code, we have provided a basic .ply reader. Functions for any additional functionality that may needed can be added easily.

