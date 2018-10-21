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

#include "Singleton.h"
#include "Linked.h"
#include "Camera.h"
#include "AppController.h"

#include "Galaxy.h"
#include "Quadrant.h"

#include "tbb/parallel_for_each.h"
#include "tbb/task_scheduler_init.h"

#include <iostream>
#include <chrono>


int main( int argc, char** argv )
{
   AppController( argc, argv );

   try
   {
      AppController::InitOpenGL();
   }
   catch( const std::exception& e )
   {
      std::cout << "Failed: " << e.what() << std::endl;
      getchar();
      return -1;
   }

   // Setup global camera
   auto camera = Camera::GetInstance();
   auto window = GlfwWindow::GetInstance();
   auto shaderProgram = Shader::Linked::GetInstance();

   tbb::concurrent_vector<Particle> universe;
   const auto blackholePrime = Galaxy::Build( universe, ObjectColors::RED, 5.0f, -4.0f, 0.75f, 3500 );
   const auto blackholeSmall = Galaxy::Build( universe, ObjectColors::GREEN, -4.0f, 3.0f, 0.35f, 800 );

   const auto calcForOnStarRange = []( Particle* blackhole, bool clockwise ) {
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
   };

   const size_t NUM_PARTICLES = universe.size() - 1;

   const auto calcForceAroundPrime = calcForOnStarRange( blackholePrime, false );
   const auto clacForceAroundSmall = calcForOnStarRange( blackholeSmall, true );

   const auto rotateAroundBlackholeFilter = [ calcForceAroundPrime, clacForceAroundSmall ]( Particle* particle )
   {
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


//
// Render Loop
//
   size_t frameCounter = 0;
   auto start = std::chrono::high_resolution_clock::now();

   while( !window->ShouldClose() )
   {
      window->TriggerCallbacks();

      // Clear the colorbuffer
      glClearColor( 0.05f, 0.075f, 0.075f, 1.0f ); // near black teal
      glClear( GL_COLOR_BUFFER_BIT );

      shaderProgram->SetUniformMat4( "view_matrix", camera->GetViewMatrix() );
      shaderProgram->SetUniformMat4( "projection_matrix", window->GetProjectionMatrix() );

      Quadrant root( Quadrant::ROOT, -42.0f, -42.0f, 42.0f, 42.0f );

      const auto applyForceFilter = [ &root ]( Particle* particle )
      {
         particle->m_Pos += root.calcForce( *particle );

         return particle;
      };

      const auto insertFilter = [ &root ]( Particle* particle )
      {
         root.insert( particle );
      };


      tbb::parallel_for(
         tbb::blocked_range<size_t>( 0, NUM_PARTICLES ),
         [ insertFilter, &universe ]( const tbb::blocked_range<size_t>& range )
         {
            for( size_t i = range.begin(); i < range.end(); i++ )
               insertFilter( &universe.at( i ) );
         }
      );

      // Draw Loop
      root.Draw();

      root.calcMassDistribution();

      tbb::parallel_for(
         tbb::blocked_range<size_t>( 0, NUM_PARTICLES ),
         [ rotateAroundBlackholeFilter, &universe ]( const tbb::blocked_range<size_t>& range )
         {
            for( size_t i = range.begin(); i < range.end(); i++ )
               rotateAroundBlackholeFilter( &universe.at( i ) );
         }
      );

      tbb::parallel_for(
         tbb::blocked_range<size_t>( 0, NUM_PARTICLES ),
         [ applyForceFilter, &universe ]( const tbb::blocked_range<size_t>& range )
         {
            for( size_t i = range.begin(); i < range.end(); i++ )
               applyForceFilter( &universe.at( i ) );
         }
      );

      window->NextBuffer();

      frameCounter++;
      auto elapsed = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::high_resolution_clock::now() - start );

      if( elapsed.count() > 5.0 )
      {
         root.print();
         std::cout << "FPS: " << frameCounter / static_cast<float>( elapsed.count() ) << std::endl;
         frameCounter = 0;
         start = std::chrono::high_resolution_clock::now();
      }
   }

   return 0;
}
