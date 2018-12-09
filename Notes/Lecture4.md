## Lecture Four Summary
- GPGPU: General-Purpose computing on a Graphics Processesing Unit
- CUDA: Compute Unified Device Architecture

GPU's offer more GFLOPS/s because they have more ALUs

Graphics pipeline
- Geometric Processing
- Rasterization

CUDA device is a coprocessor in the system, has its own DRAM and driver. Kernels ( typically data parallel functions ) are loaded
ontop the device, these are execuded SIMD in the device. Threads are single/indexed kernel executions similar to process ID.
Threads are grouped into blocks, there's typically map to GPU lanes, very minimal synchronization is possible ( ie barriers ).
Grids are groups of blocks within the device, used for schedualing purposes.
