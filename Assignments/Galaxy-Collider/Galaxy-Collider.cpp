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

#include "Galaxy.h"

typedef Shader::Linked ShaderLinker;

void key_callback( GLFWwindow* window, int key, int scancode, int action, int mode );






















#include <variant>
#include <array>
#include <memory>

class Quadrant
{
public:
   enum District { NE, SE, SW, NW };

   Quadrant( District disc, float x_min, float y_min, float x_max, float y_max ) : m_District( disc ),
      m_MinX( x_min ),
      m_MinY( y_min ),
      m_MaxX( x_max ),
      m_MaxY( y_max ),
      m_Center(
         m_MinX + ( m_MaxX - m_MinX ) / 2.0f,
         m_MinY + ( m_MaxY - m_MinY ) / 2.0f
      )
   {
      const GLuint PositonIndex = Shader::Linked::GetInstance()->GetAttributeLocation( "position" );

      std::vector<glm::vec3> vertices( { { m_MinX, m_MinY, 0.0f }, { m_MinX, m_MaxY, 0.0f },
                                         { m_MinX, m_MaxY, 0.0f }, { m_MaxX, m_MaxY, 0.0f },
                                         { m_MaxX, m_MaxY, 0.0f }, { m_MaxX, m_MinY, 0.0f },
                                         { m_MaxX, m_MinY, 0.0f }, { m_MinX, m_MinY, 0.0f } } );

      glGenVertexArrays( 1, &m_VAO );
      glBindVertexArray( m_VAO );

      glGenBuffers( 1, &m_Vertices );
      glBindBuffer( GL_ARRAY_BUFFER, m_Vertices );
      glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec3 ), &vertices.front(), GL_STATIC_DRAW );
      glVertexAttribPointer( PositonIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ), (GLvoid*)0 );
      glEnableVertexAttribArray( PositonIndex );
      glBindBuffer( GL_ARRAY_BUFFER, 0 );

      glBindVertexArray( 0 );

      m_NumVertices = (GLsizei)vertices.size();
   }

   ~Quadrant()
   {
      glDeleteBuffers( 1, &m_Vertices );
      glDeleteVertexArrays( 1, &m_VAO );
   }

   void Draw() const
   {
      auto shaderProgram = Shader::Linked::GetInstance();
      shaderProgram->SetUniformInt( "object_color", (GLint)ObjectColors::GREY );
      shaderProgram->SetUniformMat4( "model_matrix", glm::mat4(1.0f) );

      glBindVertexArray( m_VAO );
      glDrawArrays( GL_LINES, 0, m_NumVertices );
      glBindVertexArray( 0 );

      shaderProgram->SetUniformInt( "object_color", (GLint)ObjectColors::RED );

      if( auto pval = std::get_if<Particle>( &m_Contains ) )
         pval->Draw();
      else if( auto pval = std::get_if<std::array<std::unique_ptr<Quadrant>, 4>>( &m_Contains ) )
         for( auto& quad : *pval ) quad->Draw();
   }

   District getDistrict( const glm::vec2 & pos ) const
   {
      if( pos.x <= m_Center.x && pos.y <= m_Center.y ) return SW;
      if( pos.x <= m_Center.x && pos.y >= m_Center.y ) return NW;
      if( pos.x >= m_Center.x && pos.y >= m_Center.y ) return NE;
      if( pos.x >= m_Center.x && pos.y <= m_Center.y ) return SE;

      throw std::runtime_error( "Can't determine quadrant!" );
   }

   void insert( const Particle& particle )
   {
      if( auto pval = std::get_if<Particle>( &m_Contains ) )
      {
         Particle existingParticles = *pval;

         std::array<std::unique_ptr<Quadrant>, 4> oChildQuads =
         {
            std::make_unique<Quadrant>( NE, m_Center.x, m_Center.y, m_MaxX, m_MaxY ),
            std::make_unique<Quadrant>( SE, m_Center.x, m_MinY, m_MaxX, m_Center.y ),
            std::make_unique<Quadrant>( SW, m_MinX, m_MinY, m_Center.x, m_Center.y ),
            std::make_unique<Quadrant>( NW, m_MinX, m_Center.y, m_Center.x, m_MaxY )
         };

         oChildQuads[ getDistrict( existingParticles.m_Pos ) ]->insert( existingParticles );
         oChildQuads[ getDistrict( particle.m_Pos ) ]->insert( particle );

         m_Contains.emplace<std::array<std::unique_ptr<Quadrant>, 4>>( std::move( oChildQuads ) );
      }
      else if( auto pval = std::get_if<std::array<std::unique_ptr<Quadrant>, 4>>( &m_Contains ) )
      {
         ( *pval )[ getDistrict( particle.m_Pos ) ]->insert( particle );
      }
      else
      {
         m_Contains.emplace<Particle>( particle );
      }
   }

   std::variant<int, Particle, std::array<std::unique_ptr<Quadrant>, 4>> m_Contains;


   District m_District;
   float m_MinX;
   float m_MinY;
   float m_MaxX;
   float m_MaxY;
   glm::vec2 m_Center{};

   GLuint m_VAO{};
   GLuint m_Vertices{};

   GLsizei m_NumVertices;
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


   Particle particle_one( 1.0, 1.0, 1.0 );
   Particle particle_two( 2.0, -3.0, 1.0 );

   Galaxy galaxy( ObjectColors::TEAL, -5.0f, 5.0f, 7.5f, 10000 );

   Quadrant root( Quadrant::NE, -5.0f, -5.0f, 5.0f, 5.0f );

   root.insert( particle_one );
   root.insert( particle_two );


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

      galaxy.Draw();

      root.Draw();

      window->NextBuffer();
   }


   return 0;
}

//
// Models
//




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
