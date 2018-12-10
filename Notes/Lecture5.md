## Lecture Five Material
OpenCL arise with the intersection of parallelism between CPUs and GPUs. Leverage any
device to accelerate computation. Portabilitiy across devices/architectures.

Provides low-level abstract for "expert developers", with hig-performance and device
portability. 


#### OpenCL
OpenCL is a ISO C99 with languange extensions. Static or dynamic compilation of
kernels. Biult-in extenstions are includes.

- Platform API { hardware abstraction, interact with devices, create context and work queue }
- Runtime API { Execute kernels, schedualing, memory resources }

###### Models
- Platofrm Model
   - Host alias to node
   - One or more devices, each is one or more units further divided into in processing elements
   - CPUs are seen as one device
   - GPUs are deperate devices
- Memory Model
   - Shared memory { implementation specififc }
   - Distinct address spaces { implementation specififc }
   - Adress Spaces
      - Private - to work item
      - Local   - to computational unit
      - Global/Constant - to the device
- Execution Model
   - Kernels work on _NDRange_ N-Dimensional Range from { 1D, 2D, 3D }
   - Sincel instance at a given index is a work-item, grouped into work-groups
   - kernels execute within contexts thusly constratied to { device, kernel, program objects, memory object }
   - queue coordinate kernel execution, interlaces with { kernels, memory, synchronization } commandes
   
###### Runtime
- Command Queues
   - operation to perform in a context, guaranteed execution at sync point
   - signal on completion of a element
   - blocking an async modes
   - fences and barriers within a work-group, no schedualing amoung groups
- Events
   - communicatate status of commands

###### Programming
