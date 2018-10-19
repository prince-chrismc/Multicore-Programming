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

#pragma once

#include <mutex>
#include <memory>
#include <GL/glew.h>
#include "glm/vec2.hpp"

class Particle
{
public:
   Particle( float x, float  y, long double m ) : m_Pos( x, y ), m_Mass( m ) {}
   Particle( const Particle& ) = default;
   Particle( const Particle&& ) = delete;
   virtual ~Particle() = default;

   void operator=( const Particle& ) = delete;
   void operator=( const Particle&& ) = delete;

   virtual void Draw() const;

   glm::vec2 m_Pos;
   const long double m_Mass;

private:
   class Model;
};

class Particle::Model final
{
public:
   Model( const Model& ) = delete;
   Model( const Model&& ) = delete;
   ~Model();

   void operator=( const Model& ) = delete;
   void operator=( const Model&& ) = delete;

   static const Model& GetInstance();

   void Draw() const;

private:
   Model();

   GLuint m_VAO{};
   GLuint m_Vertices{};

   GLsizei m_NumVertices;

   static std::once_flag s_Flag;
   static std::unique_ptr<Model> s_Instance;
};
