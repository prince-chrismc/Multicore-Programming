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

#include "Particle.h"
#include <variant>
#include <array>
#include <memory>

class Quadrant
{
public:
   enum District { NE, SE, SW, NW };

   Quadrant(District disc, float x_min, float y_min, float x_max, float y_max);

   void Draw() const;

   District getDistrict(const glm::vec2& pos) const;

   void insert(const Particle& particle);

private:
   std::variant<int, Particle, std::array<std::unique_ptr<Quadrant>, 4>> m_Contains;

   District m_District;
   float m_MinX;
   float m_MinY;
   float m_MaxX;
   float m_MaxY;
   glm::vec2 m_Center{};

   class Model
   {
   public:
      Model( float x_min, float y_min, float x_max, float y_max );
      ~Model();

      void Draw() const;
   private:
      GLuint m_VAO{};
      GLuint m_Vertices{};

      GLsizei m_NumVertices;
   } m_oModel;
};
