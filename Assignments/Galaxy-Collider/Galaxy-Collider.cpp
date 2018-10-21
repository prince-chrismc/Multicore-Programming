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
#include "Linked.h"
#include "AppController.h"
#include "Camera.h"

#include "Galaxy.h"
#include "Quadrant.h"

#include "tbb/parallel_for_each.h"
#include "tbb/task_scheduler_init.h"

#include <iostream>
#include <chrono>


int main( int argc, char** argv )
{
   AppController oController( argc, argv );

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

   Universe universe;
   const auto blackholePrime = Galaxy::Build( universe, ObjectColors::RED, 5.0f, -4.0f, 0.75f, 3500 );
   const auto blackholeSmall = Galaxy::Build( universe, ObjectColors::GREEN, -4.0f, 3.0f, 0.35f, 800 );
   const size_t NUM_PARTICLES = universe.size() - 1;

   const auto calcForceAroundPrime = Galaxy::GenerateRotationAlgorithm( blackholePrime, false );
   const auto clacForceAroundSmall = Galaxy::GenerateRotationAlgorithm( blackholeSmall, true );

   const auto rotateAroundBlackholeFilter = [ calcForceAroundPrime, clacForceAroundSmall ]( Particle* particle )
   {
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
   size_t frameCounter = 0;
   auto start = std::chrono::high_resolution_clock::now();

   auto window = GlfwWindow::GetInstance();
   const auto shaderProgram = Shader::Linked::GetInstance();

   while( !window->ShouldClose() )
   {
      window->TriggerCallbacks();

      // Clear the colorbuffer
      glClearColor( 0.05f, 0.075f, 0.075f, 1.0f ); // near black teal
      glClear( GL_COLOR_BUFFER_BIT );

      shaderProgram->SetUniformMat4( "view_matrix", Camera::GetInstance()->GetViewMatrix() );
      shaderProgram->SetUniformMat4( "projection_matrix", window->GetProjectionMatrix() );

      Quadrant root( Quadrant::ROOT, -42.0f, -42.0f, 42.0f, 42.0f );
      applyFilterOnUniverse( [ &root ]( Particle* particle ) { root.insert( particle ); } );

      root.Draw();
      window->NextBuffer();

      root.calcMassDistribution();
      applyFilterOnUniverse( rotateAroundBlackholeFilter );
      applyFilterOnUniverse( [ &root ]( Particle* particle ) { particle->m_Pos += root.calcForce( *particle ); } );

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
