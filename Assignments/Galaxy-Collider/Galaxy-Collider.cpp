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
#include "Shaders.h"
#include "Camera.h"
#include <iostream>
#include <chrono>

#include "Galaxy.h"
#include "Quadrant.h"

#include "tbb/parallel_for_each.h"
#include "tbb/pipeline.h"
#include "tbb/task_scheduler_init.h"

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mode );

int main( int argc, char** argv )
{
   std::cout << argv[ 0 ] << std::endl;
   std::cout << "Welcome to the Galaxy Collider Simulator!" << std::endl;

   // Create a GLFWwindow
   try
   {
      auto window = GlfwWindow::CreateInstance( "Galaxy Collider Simulator by Christopher McArthur" );
      window->SetKeyCallback( key_callback );
   }
   catch( const std::exception& e )
   {
      std::cout << "Window Creation Failed: " << e.what() << std::endl;
      getchar();
      return -1;
   }

   // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
   glewExperimental = GL_TRUE;
   // Initialize GLEW to setup the OpenGL Function pointers
   if( glewInit() != GLEW_OK )
   {
      std::cout << "Failed to initialize GLEW" << std::endl;
      return -1;
   }

   // Build and compile our shader program
   try
   {
      Shader::Vertex vertexShader( "../Galaxy-Collider/shaders/vertex.shader" );
      Shader::Fragment fragmentShader( "../Galaxy-Collider/shaders/fragment.shader" );
      Shader::Linked::GetInstance()->Init( &vertexShader, &fragmentShader );
   }
   catch( const std::exception& e )
   {
      std::cout << "Shader Setup Failed: " << e.what() << std::endl;
      getchar();
      return -1;
   }

   // Setup global camera
   auto camera = Camera::GetInstance();
   auto window = GlfwWindow::GetInstance();
   auto shaderProgram = Shader::Linked::GetInstance();

   tbb::concurrent_vector<Particle> universe;
   auto blackholePrime = Galaxy::Build( universe, ObjectColors::RED, 5.0f, -4.0f, 0.25f, 4000 );
   auto blackholeSmall = Galaxy::Build( universe, ObjectColors::GREEN, -3.0f, 2.0f, 0.125f, 2500 );

   const auto calcForOnStarRange = []( Particle* blackhole ) {
      return [ blackhole ]( Particle* star )
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
         star->m_Pos.x += ( r[ 1 ] / dist ) * v;
         star->m_Pos.y += ( -r[ 0 ] / dist ) * v;
      };
   };

   const size_t NUM_PARTICLES = universe.size() - 1;

   const auto calcForceAroundPrime = calcForOnStarRange( blackholePrime );
   const auto clacForceAroundSmall = calcForOnStarRange( blackholeSmall );

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

      Quadrant root( Quadrant::ROOT, -8.0f, -8.0f, 8.0f, 8.0f );

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
         std::cout << "FPS: " << frameCounter / static_cast<float>(elapsed.count()) << std::endl;
         frameCounter = 0;
         start = std::chrono::high_resolution_clock::now();
      }
   }

   return 0;
}

//
// CALLBACK FUNCTIONS
//
void key_callback( GLFWwindow* window, int key, int, int action, int )
{
   // we are only concerned about key presses not releases
   if( action != GLFW_PRESS )
      return;

   switch( key )
   {
   case GLFW_KEY_ESCAPE:
      // Window close
      glfwSetWindowShouldClose( window, GLFW_TRUE );
      break;

   default:
      // Do Nothing
      break;
   };
}
