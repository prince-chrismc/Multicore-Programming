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
#include "tbb/queuing_mutex.h"
#include <array>
#include <memory>

class Quadrant
{
public:
   enum District { ROOT = -1, NE, SE, SW, NW };

   Quadrant( float x_min, float y_min, float x_max, float y_max );
   Quadrant( Quadrant* parent, District disc, float x_min, float y_min, float x_max, float y_max );

   void Draw() const;

   void insert( Particle* particle );

   glm::vec2 calcForce( Particle* particle ) const;

   static constexpr const float THETA = 0.6f;
   static constexpr const float GAMMA = 0.00000001f;


private:
   Quadrant* m_Parent;

   class Contains
   {
   public:
      Contains() : m_Type( NOTHING ) {}

      enum Type { NOTHING, PARTICLE, QUADRANT } m_Type;
      std::array<std::unique_ptr<Quadrant>, 4> m_Quadrants;
      Particle* m_Particle;
   } m_Contains;
   tbb::queuing_mutex m_ContainerMutex;

   unsigned long long m_TotalParticles;
   glm::vec2 m_CenterOfMass;
   long double m_Mass;

   void calcMassDistribution();

   static glm::vec2 calcAcceleration( const Particle& particle_one, const Particle& particle_two );

   class Spacial
   {
   public:
      Spacial( District disc, float x_min, float y_min, float x_max, float y_max );

      bool outsideOfRegion( const Particle& particle ) const;
      std::array<std::unique_ptr<Quadrant>, 4> makeChildDistricts( Quadrant* quad ) const;
      District determineChildDistrict( const glm::vec2& pos ) const;
      float getHeight() const;

      District m_District;
      float m_MinX;
      float m_MinY;
      float m_MaxX;
      float m_MaxY;
      glm::vec2 m_Center;
   } m_Space;

   class Model
   {
   public:
      Model( float x_min, float y_min, float x_max, float y_max );
      Model( const Model& ) = delete;
      Model( const Model&& ) = delete;
      ~Model();

      void operator=( const Model& ) = delete;
      void operator=( const Model&& ) = delete;

      void Draw() const;
   private:
      GLuint m_VAO{};
      GLuint m_Vertices{};

      GLsizei m_NumVertices;
   };
};
