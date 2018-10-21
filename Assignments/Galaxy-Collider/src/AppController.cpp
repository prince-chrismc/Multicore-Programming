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

#include <GL/glew.h>
#include "AppController.h"
#include "Singleton.h"
#include "Linked.h"
#include "Shaders.h"
#include <iostream>
#include "Camera.h"

AppController::AppController( int /*argc*/, char ** argv )
{
   std::cout << argv[ 0 ] << std::endl;
   std::cout << "Welcome to the Galaxy Collider Simulator!" << std::endl;
}

void AppController::InitOpenGL() const
{
   // Create a GLFW window
   const auto window = GlfwWindow::CreateInstance( "Galaxy Collider Simulator by Christopher McArthur" );
   if( window->SetKeyCallback( key_callback ) != nullptr ) throw std::runtime_error( "GLFW callback already set!" );

   // Setup GLEW
   glewExperimental = GL_TRUE; // Set this to true so GLEW knows to use a modern approach to retrieving function pointers and extensions
   // Initialize GLEW to setup the OpenGL Function pointers
   if( glewInit() != GLEW_OK ) throw std::runtime_error( "Failed to initialize GLEW" );

   // Initialize shaders
   Shader::Vertex vertexShader( "../Galaxy-Collider/shaders/vertex.shader" );
   Shader::Fragment fragmentShader( "../Galaxy-Collider/shaders/fragment.shader" );
   Shader::Linked::GetInstance()->Init( &vertexShader, &fragmentShader );
}

void AppController::Start()
{
   m_FrameCounter = 0;
   m_Start = std::chrono::steady_clock::now();
}

bool AppController::operator++(int)
{
   GlfwWindow::GetInstance()->NextBuffer();

   m_FrameCounter++;
   auto elapsed = std::chrono::duration_cast<std::chrono::seconds>( std::chrono::steady_clock::now() - m_Start );

   if( elapsed.count() > 5.0 )
   {
      std::cout << "FPS: " << m_FrameCounter / static_cast<float>( elapsed.count() ) << std::endl;
      Start();
      return true;
   }

   return false;
}

void AppController::ClearFrame() const
{
   auto window = GlfwWindow::GetInstance();
   window->TriggerCallbacks();

   // Clear the colorbuffer
   glClearColor( 0.05f, 0.075f, 0.075f, 1.0f ); // near black teal
   glClear( GL_COLOR_BUFFER_BIT );

   const auto shaderProgram = Shader::Linked::GetInstance();
   shaderProgram->SetUniformMat4( "view_matrix", Camera::GetInstance()->GetViewMatrix() );
   shaderProgram->SetUniformMat4( "projection_matrix", window->GetProjectionMatrix() );
}

//
// CALLBACK FUNCTIONS
//
void AppController::key_callback( GLFWwindow* window, int key, int /*scancode*/, int action, int /*mode*/ )
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
   }
}
