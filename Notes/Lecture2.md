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
- Communication takes place in the form of _Message Passing_


