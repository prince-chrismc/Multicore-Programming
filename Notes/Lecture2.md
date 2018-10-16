## Lecture One Material
Multithreading with native threads in C/C++

##### Processor Architecture
- Interconnectors
   - Bus, Ring, Mesh, Crossbar ( todays multicore design )
- Scalar vs SIMD
   - Usual
- Vector Processing
- Shared Memory
   - single copy
   - sync memory access (reads/writed)
   - not very scalable
- Distributed Memory
   - local memory access ( no sync )
   - explicit data exchange between cores ( costly )
   - data and communication is bottleneck

##### Programming Shared-Memory Multicore
- Shared access --> communication
- racing conditions + data integrity

Multicore programming is multithreading taking place in parrellel.

- Many compuation threads, local data access minimal to no syncronization
- One control thread which creates, scheduals and syncronizes the computation threads

- Data Parallelism
   - each thread does the same thing over a different segment of data set, join at end concat results
   
- Task Parallelism
   - each thread gets is own job might be syncronization
   
### OpenMP
Preprocessor data parallelization. Instructs compiler where to parrellelize. Everything is up to the programmer to ensure thread sately.

### Cilk
C dynamic multithread application. 

###### CilkPlus
Language extension to provide **fork-join** parallelism. Featuring an "efficient work-stealing schedular".

```c
cilk int fib (int n) {
   if (n<2) return (n);
   else {
      int x,y;
      x = spawn fib(n-1);
      y = spawn fib(n-2);
      sync;
      return (x+y);
   }
}
```

Strict new keywords but no data structures are offerd by this extension.

Loops must be made recursive to apply the keywords in any meaningful manner.

Each processor maintains a w ork deque of ready threads, and it manipulates the bottom of the deque like a stack
- When a processor runs out of work, it steals a thread from the top of a random victim’s deque. 

Provides a spin lock --> unlikely to use with extension symatics ... can not call `spawn` or `sync`

### Programming Distributed-Memory Architecture
- Communication takes place in the form of _Message Passing_ which requires massive syncronization by both the Tx and Rx

### Analysing Performance
- Coverage  -   _Ahmdol's Law_
- Granularity   -  work / thread 
- Locality of computation anf communication

##### Limits to scalabilty
_Ahmdol's Law_
- Programs have sequential and parrellel portions.. infers a max `Speed Up`
- Granularity is a qualitative measure of the ratio of computation to communication
   - Seperating computation and Communication with syncronization events
   - fine grain : little compuation to lots of communication, little enhancement range + lots of overhead
   - coarse grain : lots of computation with little communication, easier to gain in performance. requires load balancing!
      - threads which finish earliest end up idling reducing the efficiency of the parrellelization

Static Load Balancing
- homogeneous multicores: each core gets equal work each does equal amount of work
- heterogenous cores: some cores will be faster and compelete and fall idle

Dynamic Load Balancing
- Idle cores as assigned work
- work seperation is unven or core performance is uneven

### Granularity and Performance trade offs
- pipelining communication and work phases
- memory locatilty - each thread gets is own block vs each reading across blocks
- memory latency - Uniform ( one main bank ) Non-Uniform ( multi level cache and DIMMS )
