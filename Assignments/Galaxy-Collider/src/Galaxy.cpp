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

#ifdef _CLANG
   #define TBB_USE_GLIBCXX_VERSION 60000 // Know TBBB Issue for linux && clang
#endif

#include "Galaxy.h"
#include "tbb/parallel_for.h"
#include <random>

Blackhole::Blackhole(float x, float y) : Particle( ObjectColors::YELLOW, x, y, 1453.485L )
{
}

Particle* Galaxy::Build( Universe& out_particles, ObjectColors col, float x, float y, float radius, size_t particles )
{
   const auto blackhole = out_particles.emplace_back( ObjectColors::YELLOW, x, y, 1453.485L ); // Blackhole
   static constexpr const long double PI = 3.141592653589793238462643383279502884L;

   std::random_device rd;
   std::mt19937 gen( rd() );
   std::lognormal_distribution<double> numGenPos( 0.0, 1.8645 );
   std::lognormal_distribution<float> numGenMass( 0.0f, 1.0f );

   const auto ParticleGenerator = [ & ]( const tbb::blocked_range<size_t>& range )
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

Galaxy::ParticleManipulator Galaxy::GenerateRotationAlgorithm(Particle* blackhole, bool clockwise)
{
   return [ = ]( Particle* star )
   {
      const float &x1( blackhole->m_Pos.x ), &y1( blackhole->m_Pos.y );
      const long double &m1( blackhole->m_Mass );

      const float &x2( star->m_Pos.x ), &y2( star->m_Pos.y );

      // Calculate distance from the planet with index idx_main
      float r[ 2 ];
      r[ 0 ] = x1 - x2;
      r[ 1 ] = y1 - y2;

      // distance in parsec by pythag
      const float dist = sqrt( r[ 0 ] * r[ 0 ] + r[ 1 ] * r[ 1 ] );

      // Based on the distance from the sun calculate the velocity needed to maintain a circular orbit
      const float v = sqrt( Galaxy::GAMMA * m1 / dist );

      // Calculate a suitable vector perpendicular to r for the velocity of the tracer
      if( clockwise )
      {
         star->m_Pos.x += ( r[ 1 ] / dist ) * v;
         star->m_Pos.y += ( -r[ 0 ] / dist ) * v;
      }
      else
      {
         star->m_Pos.x -= ( r[ 1 ] / dist ) * v;
         star->m_Pos.y -= ( -r[ 0 ] / dist ) * v;
      }
   };
}
