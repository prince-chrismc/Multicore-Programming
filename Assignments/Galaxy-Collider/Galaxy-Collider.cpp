/*
 *
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

#include <GL/glew.h>
#include "Singleton.h"
#include "AppController.h"

#include "Galaxy.h"
#include "Quadrant.h"

#include "tbb/parallel_for_each.h"
#include "tbb/task_scheduler_init.h"

#include <iostream>


int main( int argc, char** argv )
{
   AppController oController;

   try
   {
      oController.InitOpenGL();
   }
   catch( const std::exception& e )
   {
      std::cout << "Failed: " << e.what() << std::endl;
      getchar();
      return -1;
   }

   Universe universe;
   const auto blackholePrime = Galaxy::Build( universe, ObjectColors::RED, 5.0f, -4.0f, 0.75f, 3500 );
   const auto blackholeSmall = Galaxy::Build( universe, ObjectColors::GREEN, -4.0f, 3.0f, 0.35f, 800 );
   const size_t NUM_PARTICLES = universe.size() - 1;

   const auto rotateAroundBlackholeFilter = [ blackholePrime, blackholeSmall ]( Particle* particle )
   {
      const auto calcForceAroundPrime = Galaxy::GenerateRotationAlgorithm( blackholePrime, false );
      const auto clacForceAroundSmall = Galaxy::GenerateRotationAlgorithm( blackholeSmall, true );

      // TO DO : Instead of deciding by color pick the closest one!
      switch( particle->m_Color )
      {
      case ObjectColors::RED:
         calcForceAroundPrime( particle );
         break;
      case ObjectColors::GREEN:
         clacForceAroundSmall( particle );
         break;
      default:
         break; // Skips blackholes!
      }
   };

   const auto applyFilterOnUniverse = [ &universe, NUM_PARTICLES ]( const Galaxy::ParticleManipulator& effect )
   {
      tbb::parallel_for(
         tbb::blocked_range<size_t>( 0, NUM_PARTICLES ),
         [ effect, &universe ]( const tbb::blocked_range<size_t>& range )
         {
            for( size_t i = range.begin(); i < range.end(); i++ )
               effect( &universe.at( i ) );
         }
      );
   };

   //
   // Render Loop
   //
   oController.Start();
   while( oController.IsRunning() )
   {
      oController.ClearFrame();

      Quadrant root( Quadrant::ROOT, -42.0f, -42.0f, 42.0f, 42.0f );
      applyFilterOnUniverse( [ &root ]( Particle* particle ) { root.insert( particle ); } );

      root.Draw();

      root.calcMassDistribution();
      applyFilterOnUniverse( rotateAroundBlackholeFilter );
      applyFilterOnUniverse( [ &root ]( Particle* particle ) { particle->m_Pos += root.calcForce( *particle ); } );

      if( oController++ )
         root.print();
   }

   return 0;
}
