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

#include "Particle.h"
#include "Linked.h"
#include <vector>

std::once_flag Particle::Model::s_Flag;
std::unique_ptr<Particle::Model> Particle::Model::s_Instance;

void Particle::Draw() const
{
   auto shaderProgram = Shader::Linked::GetInstance();

   glm::mat4 model_matrix(1.0f);
   model_matrix = glm::translate(model_matrix, {m_Pos.x, m_Pos.y, 0.0f});
   shaderProgram->SetUniformMat4("model_matrix", model_matrix);

   Model::GetInstance().Draw();
}

Particle::Model::Model()
{
   const GLuint PositonIndex = Shader::Linked::GetInstance()->GetAttributeLocation( "position" );

   std::vector<glm::vec3> vertices( { { 0.0f, 0.0f, 0.0f } } );

   glGenVertexArrays( 1, &m_VAO );
   glBindVertexArray( m_VAO );

   glGenBuffers( 1, &m_Vertices );
   glBindBuffer( GL_ARRAY_BUFFER, m_Vertices );
   glBufferData( GL_ARRAY_BUFFER, vertices.size() * sizeof( glm::vec3 ), &vertices.front(), GL_STATIC_DRAW );
   glVertexAttribPointer( PositonIndex, 3, GL_FLOAT, GL_FALSE, 3 * sizeof( GLfloat ), (GLvoid*)0 );
   glEnableVertexAttribArray( PositonIndex );
   glBindBuffer( GL_ARRAY_BUFFER, 0 );

   glBindVertexArray( 0 );

   m_NumVertices = static_cast<GLsizei>(vertices.size());
}

Particle::Model::~Model()
{
   glDeleteBuffers(1, &m_Vertices);
   glDeleteVertexArrays(1, &m_VAO);
}

const Particle::Model& Particle::Model::GetInstance()
{
   std::call_once(s_Flag, []() { s_Instance.reset(new Model()); });
   return *s_Instance;
}

void Particle::Model::Draw() const
{
   glBindVertexArray( m_VAO );
   glDrawArrays( GL_POINTS, 0, m_NumVertices );
   glBindVertexArray( 0 );
}
