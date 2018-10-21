/*
MIT License

Copyright (c) 2018 Chris McArthur, prince.chrismc(at)gmail(dot)com

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
*/

#include "Galaxy.h"
#include <random>
#include "tbb/parallel_for.h"

Blackhole::Blackhole(float x, float y) : Particle( ObjectColors::YELLOW, x, y, 1453.485L )
{
}

Particle* Galaxy::Build( tbb::concurrent_vector<Particle>& out_particles, ObjectColors col, float x, float y, float radius, size_t particles )
{
   auto blackhole = out_particles.emplace_back( ObjectColors::YELLOW, x, y, 1453.485L ); // Blackhole
   static constexpr const long double PI = 3.141592653589793238462643383279502884L;

   std::random_device rd;
   std::mt19937 gen( rd() );
   const std::lognormal_distribution<long double> numGenPos( 0.0L, 1.8645L );
   const std::lognormal_distribution<float> numGenMass( 0.0f, 1.0f );

   const auto ParticleGenerator = [ =, &out_particles, &gen ]( const tbb::blocked_range<size_t>& range )
   {
      for( size_t i = range.begin(); i < range.end(); i++ )
      {
         const auto a = static_cast<float>( numGenPos( gen ) * 2.0L * PI );
         const auto r = static_cast<float>( sqrt( numGenPos( gen ) * radius ) );

         // in Cartesian coordinates
         const float rel_x = r * cos( a );
         const float rel_y = r * sin( a );


         // distance in parsec by pythag
         const float dist = sqrt( rel_x * rel_x + rel_y * rel_y );

         if( dist < radius * 4.8746f )
            out_particles.emplace_back( col, rel_x + x, rel_y + y, 0.76f + numGenMass( gen ) / 100.0f );
         else
            i -= 1;
      }
   };

   tbb::parallel_for( tbb::blocked_range<size_t>( 0, particles ), ParticleGenerator );

   return &*blackhole;
}
