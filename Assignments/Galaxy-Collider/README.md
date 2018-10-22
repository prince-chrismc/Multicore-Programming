# Gallaxy Collider
A Banrs-Hut / N-Body simulation of two galaxies colliding.

>*To view this [document](https://github.com/prince-chrismc/Multicore-Programming/blob/master/Assignments/Galaxy-Collider/README.md) properly, click on the link provided.* There are also samples which can be seen of beautiful galaxy collisions [here](https://github.com/prince-chrismc/Multicore-Programming/tree/master/Docs) for reference.

### Table Of Contents
1. [Contributors](#contributors)
2. [Design](#design)
3. [Physics Engine](#physics-engine)
4. [Parallelization](#parallelization)

## Design
There are several models to this project which build up the under lying data structures as well as a controller for the application life cycle and some general computation.

At the core of an N-Body simulation are the bodies of in this case `Particle`s which are recursively devided into `Quadrants`. There is a special type of `Particle` which are treated differently by the _physics engine_; these are `Blackholes` which are the center of a cluster of particles and the point of rotation for that cluster.

The concepts of galaxies and a universe are present but are not data structures. The `Universe` is a vector of `Partcile`s and is filled with galaxies; galaxies are the parallel generation of clusters of particles centered around a black hole at a certian postion. A Universe may contain any number of elements.

Each `Particle` from the universe is pumped into a root `Quadrant` which recursively divides when a second particle is added within its space. Any `Particle`s out of the root are ignored for the purpose of this model, however they will be processed by the _physics engine_ and will be pulled towards the center of mass.

## Physics Engine
In order to have enough computation to perform for the parrallelization of this simulation to have any meaningfuly addition to the program, there is an extra layer of _physics_ which are applied to the simulation.

1. rotational force around a blackhole. when a particle is created it is associated to a blackhole to which it will spend it's existance trying to rotate at a velocity proportional to its distance.
2. Maximum force: it is possible for the N-Body force to launch particles far and wide, as such a particle has a maximum acceleration, this phenomenon also applies to blackholes as such is maximum acceleration is much smaller ( proportional to weight )
2. Collision handling:
   - Particle and Particle: When two particles collide ( or pass extremely close together ) one of the particles absorbs/consumes a portion of the weight ( based on distance ) and launches it the inverse of that proportion multiplied by the maximum force. In less ambigous words, a portion of the energy from  the collision results is the transfer of mass and the remain energy is translation move one of the particles.
   - Particle and Blackhole: All the weight of the particle is absorbed by the blackhole and the particle experiences the singularity! ( its actually just places extremely far from the quad-tree ).

## Parrallelaization
~~The main computational work for the _physics engine_ as well as the quad-tree and N-Body simulation are done within a TBB pipeline consisting of 5 stages.~~

The pipeline utterly failed! The biggest limitation to design of this is the Quad-tree insertion which needs to be done for every single frame in order to be able to do the force calculation. If time permits, for my third assignment I would like to try and move particles within the quad tree in order to removethe overhead of inserting. The insertion is the most costly because is requires syncronization during the early life cycle of the quadrant. That means when a quadrant has 0 or 1 particles ( ie not yet been recursively divided ) it will block further insertion however when there are two or more then there is no syncronization since the particle will always be forwarded to a child quadrant.

The second limitation to this application is the lack of performance gained from multithreaded OpenGL operations. For this reason I have opted to keep all the generation of models ( OpenGL buffers ) and rendering operations ( ie passing variables and buffers to the shaders ) in the main control thread. There is no reason to have this done in other threads.

The main computation work is done in a series of `parallel_for` loops which apply different `ParticleManipulator`s. The sequesne of this pseudo pipeline are as follows
1. `parallel_for` insertion into quad tree
2. Sequential draw of the quad tree. This also inclues the generation of the models for the lines if enabled.
3. Recursively calculate the mass distribution ( done with `task_group`s )
4. `parallel_for` rotation
5. `parallel_for` N-Bosy force application

The last signification parallelazation is with the generation of each galaxy which utilizes the `concurrent_vector`'s thread safe growth to fill it with a `parallel_for` loop.
