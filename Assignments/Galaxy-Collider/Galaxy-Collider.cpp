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

#include <iostream>
#include <GL/glew.h>
#include "Singleton.h"
#include "Linked.h"
#include "Shaders.h"
#include "Camera.h"

typedef Shader::Linked ShaderLinker;

int main()
{
   std::cout << "Welcome to the Galaxy Collider Simulator!" << std::endl;

   // Create a GLFWwindow
   auto window = GlfwWindow::CreateInstance( "Galaxy Collider Simulator by Christopher McArthur" );
   if( ! window->IsValid() ) // Make sure it exists
   {
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
   Shader::Vertex vertexShader( "shaders/vertex.shader" );
   Shader::Fragment fragmentShader( "shaders/fragment.shader" );
   if( !vertexShader() || !fragmentShader() ) // make sure they are ready to use
   {
      return -1;
   }

   auto shaderProgram = ShaderLinker::GetInstance();
   if( !shaderProgram->Init( &vertexShader, &fragmentShader ) )
   {
      return -1;
   }

   // Constant vectors
   const glm::vec3 center( 0.0f, 0.0f, 0.0f );
   const glm::vec3 up( 0.0f, 1.0f, 0.0f );
   const glm::vec3 eye( 0.0f, 35.0f, 35.0f );

   // Setup global camera
   auto camera = Camera::GetInstance();


}
