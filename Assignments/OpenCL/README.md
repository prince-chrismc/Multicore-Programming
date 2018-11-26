# OpenCL N-Body Simulation
Your typical N-Body simulation.

![preview](https://github.com/prince-chrismc/Multicore-Programming/blob/master/Docs/Screencast%202018-11-26%2013:35:15.mp4?raw=true)

One exception to this project is that most of the work was done on my laptop which AMD does not expose the integrated GPU as a seperate compute device ( aka there is only one device ). This limitation strongly affect my implementation. I did however do some work on my workstation ( which has 2 CPUs and a GPU ) and modified the implementation slightly to meet the project requirements as much as possible in my limited time frame.

This was built from the AMD APP SDK v3.0 Sample but is heavily modified from the orignal.

## Operational Modes
My simulation can run in three modes, CPU, GPU or ALL. When running with CPU/GPU, the kernel is executed on the given device and the host application performs the rendering.

### Work Division
When running under the ALL flag the work is dived amoung all devices as follows. Otherwise it is performed on the selected device.

###### CPU
Performs the N-Body calculation. Reading from a current position buffer and calculates the new position which is saved into the next frame buffer.

###### GPU
Reads from the current position buffer to render the particles based on the positions and calculates the color.

###### Synchronization
The host program waits for both kernels to execute. There's no data dependency between the kernels.

### Kernel Workload
###### CPU
The number of particles are divided into work groups of 64 elements and are passed to the CPU which is unrolled into sets of 8 for processesing.

###### GPU
The particles are passed as a single group and processed in groups of 256 to calculate the pixel color for each pixel. Once the particles are proccessed the image in rendered with OpenGL.

### Host Control
The host application parses the CLI args, generates the two galaxies, and setups OpenCL.

The host has a limited amount of work in the main loop since there is no dependency between the Kernels. It has two buffers for the current and next frame and alternates the roles between them. The Current buffer is passed to the GPU and splits the buffer into work grous ( by shifting the pointer ) which are passed to the CPU.
