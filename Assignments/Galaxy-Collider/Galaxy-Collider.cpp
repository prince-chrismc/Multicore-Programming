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

#include <limits>
#include "ObjectColors.h"
#include "Particle.h"
#include <map>
#include <random>

typedef Shader::Linked ShaderLinker;

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mode );


















class Blackhole : public Particle
{
public:
   Blackhole( float x, float  y ) : Particle( x, y, LDBL_MAX ) {}

   void Draw() const override { ShaderLinker::GetInstance()->SetUniformInt( "object_color", (GLint)ObjectColors::YELLOW ); Particle::Draw(); }
};


#include <tuple>
struct GlmVec2Comparator
{
   bool operator()( const glm::vec2& l, const glm::vec2& r ) const {
      return std::tie( l.x, l.y ) < std::tie( r.x, r.y );
   }
};

class Galaxy /*: public Drawable*/
{
public:
   Galaxy( ObjectColors col, float x, float y, float radius, size_t particles ) : m_Blackhole( x, y ), m_Color( col )
   {
      static constexpr const long double PI = 3.141592653589793238462643383279502884L;

      static std::random_device rd;
      static std::mt19937 gen( rd() );
      static std::lognormal_distribution<float> numGen( -0.5f, 1.25f );

      for( size_t i = 0; i < particles; i++ )
      {
         float a = numGen( gen ) * 2.0f * PI;
         float r = radius * sqrt( numGen( gen ) );

         // If you need it in Cartesian coordinates
         float rel_x = r * cos( a );
         float rel_y = r * sin( a );

         m_Stars.insert( std::make_pair( glm::vec2{ rel_x + x, rel_y + y }, Particle( rel_x + x, rel_y + y, gen() ) ) );
      }
   }

   void Draw() const
   {
      m_Blackhole.Draw();
      ShaderLinker::GetInstance()->SetUniformInt( "object_color", (GLint)m_Color );
      for( auto& Particle : m_Stars ) Particle.second.Draw();
   }

private:
   Blackhole m_Blackhole;
   ObjectColors m_Color;
   std::map<glm::vec2, Particle, GlmVec2Comparator> m_Stars;
};
















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
      ShaderLinker::GetInstance()->Init( &vertexShader, &fragmentShader );
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
   auto shaderProgram = ShaderLinker::GetInstance();


   Particle particle( 0.0, 0.0, 1.0 );

   Galaxy galaxy( ObjectColors::TEAL, -5.0f, 5.0f, 2.5f, 10000 );


   while( !window->ShouldClose() )
   {
      window->TriggerCallbacks();

      // Clear the colorbuffer
      glClearColor( 0.05f, 0.075f, 0.075f, 1.0f ); // near black teal
      glClear( GL_COLOR_BUFFER_BIT );

      shaderProgram->SetUniformMat4( "view_matrix", camera->GetViewMatrix() );
      shaderProgram->SetUniformMat4( "projection_matrix", window->GetProjectionMatrix() );

      // Draw Loop
      shaderProgram->SetUniformInt( "object_color", (GLint)ObjectColors::RED );
      particle.Draw();
      galaxy.Draw();

      window->NextBuffer();
   }


   return 0;
}

//
// Interfaces
//
class Drawable abstract
{
public:
   virtual ~Drawable() = default;
   virtual void Draw() const = 0;
};

//
// Models
//
class Quadrant : public Drawable
{
public:
   enum District { NE, SE, SW, NW };

   Quadrant( District disc, long double x, long double y ) : m_District( disc ), m_Pos( x, y ) {}

   void Draw() const override { /* TBA */ }

   District m_District;
   glm::vec<2, long double> m_Pos;
};


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
