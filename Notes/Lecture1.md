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
