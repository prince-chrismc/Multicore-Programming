## Lecture Four Summary
- GPGPU: General-Purpose computing on a Graphics Processesing Unit
- CUDA: Compute Unified Device Architecture

GPU's offer more GFLOPS/s because they have more ALUs

Graphics pipeline
- Geometric Processing
- Rasterization

CUDA device is a coprocessor in the system, has its own DRAM and driver. Kernels ( typically data parallel functions ) are loaded
onto the device, these are execuded SIMD in the device. Threads are single/indexed kernel executions similar to process ID.
Threads are grouped into blocks, there's typically map to GPU lanes, very minimal synchronization is possible ( ie barriers ).
Grids are groups of blocks within the device, used for schedualing purposes. Executing threads are grouped into wraps which all threads who are ready to execute the same next instruction.

Kernals map to grids, all blocks within a grid have a unique memory space, all threads within a block can share data and synchronize due to the shared memory space

Memory Models:
- Registers, on chip, limited number, 32-bit, fast access, per thread
- Local Memory, DRAM, slow, no chaching per thread, decent size
- Shared Memory, on chip, per block, synchronized, moderate size
- Gloabl Memory, DRAM, no chaching, slow, per grid
- Constant Memory, DRAM, chached, read-only
- Texture Memory, DRAM, chached, read-only

{ Global, Constant, Texture } managed by host code
