## Lecture One Material
Basic overview of computer architecture... Why did Multicores arise... What type of multicores exist...

##### Evolution of Preliminary Designs
Performance gains from increases of hardware capabilities and manufacturing techniques...
- Doubling of transistor on a chip every two years _Moore's Law_
- Increasing speed with pipelining

Limitations...
1. Power wall
   - scaling CPU speed increased heat output

##### Instruction Level Parrellism
Multiple instructions executed per completed every clock cycle. The processor can...
- re-order instructions
- pipeline instructions
- split instructions into microinstructions
- do aggressive branch prediction
- Very Large instruction word size
- SIMD and vector processing
- Hardware multithreading

Limitations...
1. ILP Wall / Hidden parrelellism
   - could not find any more parrellelism in sequential code _Ahmdal's Law_
2. Memory Wall
   - Growing disparity between memory access times and CPU speed, diminishing returns from increased cache sizes

## Multicore Architecture
To increase the number of processors (cores), decrease the cache size, and decrease the clock rate.
- Each core is simpler
- Using more processors with small caches
   - achieves higher performance than enlarging caches and keeping the same number of processors.
   
Advantages...
- Signals (data) between processors (cores) travel shorter distances (multiple cores on a chip)
   - high-quality signals
   - allow higher bandwidth (for cache snooping circuitry, e.g.)
- Smaller size (PCB space) than multi-chip SMP designs
- Less power than multi-chip SMP designs
   - have to drive signals off the chip less often
- Multiple cores can share resources like L2 cache

Disatvanges...
- New OS and software support needed to optimally utilize multiple cores
- Actual performance improvement may not be proportional to the number of cores

##### Flynn's Taxonomy
Various multicore architectures that are possible.
![diagram](https://www.google.com/url?sa=i&rct=j&q=&esrc=s&source=images&cd=&cad=rja&uact=8&ved=2ahUKEwiwyffsj4neAhXGVN8KHWRxB_gQjRx6BAgBEAU&url=https%3A%2F%2Fwww.researchgate.net%2Ffigure%2FFlynns-Taxonomy-of-Computer-Architectures-13_fig1_268011284&psig=AOvVaw1K1frSKBtM7_QyYBY7d2UB&ust=1539716155421053)

##### Processor Structures
- Single core
- Multi-Processor
- Hyperthreading
- Milticore
- Milticore w/ shared cache
- Milticore w/ hyperthreading

### Multicore Processor
- All processors are on the same chip
   - CMP - Chip Multi Processor
- Follow a MIMD architecture
- All cores share the same memory

##### Homogeneous Multicores
- All cores have the exact same architecture and instruction set
   - Easy design/fabrication/course-grain computations
   - Complex syncronization
   - may not be efficient
   - memory latency is a bottleneck
   
##### Heterogeneous Multicore
- provide different levels of compatibility between the processors   
- Decreased Power Consumption
   - Low power processors are usually more efficient.
   - Heterogeneous cores can provide balance
   - between performance and power consumption
- Application Specific Instruction Sets
   - Higher efficiency
   - High performance cores
   - Specialized Instruction Set for each core.
   - Tailored for a specific application.
   - High flexibility through software programmability.
   - High performance at low power consumption
