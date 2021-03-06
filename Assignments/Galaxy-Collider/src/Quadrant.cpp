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

#include "Quadrant.h"
#include "Linked.h"
#include "ObjectColors.h"
#include <vector>
#include "tbb/task_group.h"
#include <random>

Quadrant::Quadrant( District disc, float x_min, float y_min, float x_max, float y_max ) :
   m_TotalParticles( 0 ), m_CenterOfMass( 0.0f ), m_Mass( 0.0L ),
   m_Space( disc, x_min, y_min, x_max, y_max )
{
}

void Quadrant::Draw()
{
   auto shaderProgram = Shader::Linked::GetInstance();
   shaderProgram->SetUniformInt( "object_color", (GLint)ObjectColors::GREY );
   shaderProgram->SetUniformMat4( "model_matrix", glm::mat4( 1.0f ) );

   //if( m_Space.getHeight() > 0.05f )
   //{
   //   m_oModel.emplace( m_Space.m_MinX, m_Space.m_MinY, m_Space.m_MaxX, m_Space.m_MaxY );
   //   m_oModel->Draw();
   //}

   if( auto pval = std::get_if<Particle*>( &m_Contains ) )
      ( *pval )->Draw();
   else if( auto pval = std::get_if<std::array<std::unique_ptr<Quadrant>, 4>>( &m_Contains ) )
      for( auto& quad : *pval ) quad->Draw();
}

void Quadrant::insert( Particle* particle )
{
   if( m_Space.outsideOfRegion( *particle ) )
      return; // Don't even bother =)

   m_InsertLock.lock();
   if( auto pval = std::get_if<Particle*>( &m_Contains ) )
   {
      float r = sqrt( ( particle->m_Pos.x - ( *pval )->m_Pos.x ) * ( particle->m_Pos.x - ( *pval )->m_Pos.x ) +
         ( particle->m_Pos.y - ( *pval )->m_Pos.y ) * ( particle->m_Pos.y - ( *pval )->m_Pos.y ) );

      if( r < TOO_CLOSE )
      {
         if( particle->m_Color != ObjectColors::YELLOW )
         {

            static constexpr const long double PI = 3.141592653589793238462643383279502884L;

            std::random_device rd;
            std::mt19937 gen( rd() );
            std::lognormal_distribution<float> numGenPos( 0.0f, 1.8645f );

            const auto angle = static_cast<float>( numGenPos( gen ) * 2.0L * PI );
            const auto travel = sqrt( numGenPos( gen ) * 1.8987654f );

            // in Cartesian coordinates
            const float rel_x = travel * cos( angle );
            const float rel_y = travel * sin( angle );


            const float distance = sqrt( rel_x * rel_x + rel_y * rel_y );

            auto myParticle = const_cast<Particle*>( *pval );
            if( distance <= 1.8987654f )
            {
               particle->m_Pos.x += rel_x;
               particle->m_Pos.y += rel_y;

               const float force_ratio = distance / 1.8987654f;
               myParticle->m_Mass += ( particle->m_Mass * force_ratio );
               particle->m_Mass = particle->m_Mass * ( 1.0f - force_ratio );
            }
            else
            {
               particle->m_Pos = { -1000.0f, -1000.0f };
               myParticle->m_Mass += particle->m_Mass / 2.0f;
            }
         }
         m_InsertLock.unlock();
         return; // The particle is too close just drop it...
      }

      std::array<std::unique_ptr<Quadrant>, 4> oChildQuads = m_Space.makeChildDistricts();
      oChildQuads[ m_Space.determineChildDistrict( ( *pval )->m_Pos ) ]->insert( *pval );
      oChildQuads[ m_Space.determineChildDistrict( particle->m_Pos ) ]->insert( particle );

      m_Contains.emplace<std::array<std::unique_ptr<Quadrant>, 4>>( std::move( oChildQuads ) );

      m_CenterOfMass = glm::vec2{ 0.0f,0.0f };
      m_Mass = 0.0f;
   }
   else if( auto pval = std::get_if<std::array<std::unique_ptr<Quadrant>, 4>>( &m_Contains ) )
   {
      m_TotalParticles++;
      m_InsertLock.unlock();
      return ( *pval )[ m_Space.determineChildDistrict( particle->m_Pos ) ]->insert( particle );
   }
   else
   {
      m_Contains.emplace<Particle*>( particle );
      m_CenterOfMass = particle->m_Pos;
      m_Mass = particle->m_Mass;
   }

   m_TotalParticles++;
   m_InsertLock.unlock();
}

glm::vec2 Quadrant::calcForce( const Particle& particle ) const
{
   glm::vec2 acc{ 0.0f, 0.0f };

   if( auto pval = std::get_if<Particle*>( &m_Contains ) )
   {
      acc = calcAcceleration( particle, **pval );
   }
   else if( auto pval = std::get_if<std::array<std::unique_ptr<Quadrant>, 4>>( &m_Contains ) )
   {
      float d = m_Space.getHeight();
      float r = sqrt( ( particle.m_Pos.x - m_CenterOfMass.x ) * ( particle.m_Pos.x - m_CenterOfMass.x ) +
         ( particle.m_Pos.y - m_CenterOfMass.y ) * ( particle.m_Pos.y - m_CenterOfMass.y ) );

      if( d / r < THETA )
      {
         const float k = GAMMA * m_Mass / ( r*r*r );
         acc.x = k * ( m_CenterOfMass.x - particle.m_Pos.x );
         acc.y = k * ( m_CenterOfMass.y - particle.m_Pos.y );
      }
      else
      {
         for( auto& quad : *pval )
            acc += quad->calcForce( particle );
      }
   }

   const float MAX_FORCE = ( particle.m_Color == ObjectColors::YELLOW ) ? 0.0856745f : 1.8987654f;
   return glm::vec2
   {
      std::abs( acc.x ) < MAX_FORCE ? acc.x : acc.x > 0 ? MAX_FORCE : 0.0f - MAX_FORCE,
      std::abs( acc.y ) < MAX_FORCE ? acc.y : acc.y > 0 ? MAX_FORCE : 0.0f - MAX_FORCE
   };
}

void Quadrant::print() const
{
   printf( "%llu particles with a mass of %f centered at { %f, %f }\r\n", m_TotalParticles, m_Mass, m_CenterOfMass.x, m_CenterOfMass.y );
}

void Quadrant::calcMassDistribution()
{
   if( auto pval = std::get_if<std::array<std::unique_ptr<Quadrant>, 4>>( &m_Contains ) )
   {
      tbb::task_group g;
      for( auto& quad : *pval )
      {
         if( quad->m_TotalParticles == 0 ) continue;

         g.run( [ & ] { quad->calcMassDistribution(); } );
      }
      g.wait();

      for( auto& quad : *pval )
      {
         m_Mass += quad->m_Mass;
         m_CenterOfMass += quad->m_Mass * quad->m_CenterOfMass;
      }
      m_CenterOfMass /= m_Mass;
   }
}

glm::vec2 Quadrant::calcAcceleration( const Particle& particle_one, const Particle& particle_two )
{
   glm::vec2 acc{ 0.0f, 0.0f };

   if( &particle_one == &particle_two )
      return acc;

    // assign references to the variables in a readable form
   const float &x1( particle_one.m_Pos.x ), &y1( particle_one.m_Pos.y );
   const float &x2( particle_two.m_Pos.x ), &y2( particle_two.m_Pos.y );
   const float &m2( particle_two.m_Mass );

   const float r = sqrt( ( x1 - x2 ) * ( x1 - x2 ) + ( y1 - y2 ) * ( y1 - y2 ) );

   if( r > 0 ) // if distance is greater zero
   {
      const float k = GAMMA * m2 / ( r*r*r );

      acc.x += k * ( x2 - x1 );
      acc.y += k * ( y2 - y1 );
   }

   return acc;
}

//
// Spacial
//
Quadrant::Spacial::Spacial( District disc, float x_min, float y_min, float x_max, float y_max ) :
   m_District( disc ),
   m_MinX( x_min ),
   m_MinY( y_min ),
   m_MaxX( x_max ),
   m_MaxY( y_max ),
   m_Center( m_MinX + ( m_MaxX - m_MinX ) / 2.0f,
             m_MinY + ( m_MaxY - m_MinY ) / 2.0f )
{
}

bool Quadrant::Spacial::outsideOfRegion( const Particle& particle ) const
{
   // https://stackoverflow.com/a/42396910/8480874
   auto calcVector = []( glm::vec2 p1, glm::vec2 p2 )->glm::vec2 {
      return{ p2.x - p1.x , -1 * ( p2.y - p1.y ) };
   };

   const auto isPointWithinFourCornersOfRectangle = [ calcVector ]( glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D, glm::vec2 m )->bool {
      glm::vec2 AB = calcVector( A, B );  float C1 = -1 * ( AB.y*A.x + AB.x*A.y ); float D1 = ( AB.y*m.x + AB.x*m.y ) + C1;
      glm::vec2 AD = calcVector( A, D );  float C2 = -1 * ( AD.y*A.x + AD.x*A.y ); float D2 = ( AD.y*m.x + AD.x*m.y ) + C2;
      glm::vec2 BC = calcVector( B, C );  float C3 = -1 * ( BC.y*B.x + BC.x*B.y ); float D3 = ( BC.y*m.x + BC.x*m.y ) + C3;
      glm::vec2 CD = calcVector( C, D );  float C4 = -1 * ( CD.y*C.x + CD.x*C.y ); float D4 = ( CD.y*m.x + CD.x*m.y ) + C4;

      return 0 >= D1 && 0 >= D4 && 0 <= D2 && 0 >= D3;
   };

   return !isPointWithinFourCornersOfRectangle( glm::vec2{ m_MaxX, m_MaxY }, glm::vec2{ m_MaxX, m_MinY },
                                                glm::vec2{ m_MinX, m_MinY }, glm::vec2{ m_MinX, m_MaxY },
                                                particle.m_Pos
   );
}

std::array<std::unique_ptr<Quadrant>, 4> Quadrant::Spacial::makeChildDistricts() const
{
   return
   {
         std::make_unique<Quadrant>( NE, m_Center.x, m_Center.y, m_MaxX, m_MaxY ),
         std::make_unique<Quadrant>( SE, m_Center.x, m_MinY, m_MaxX, m_Center.y ),
         std::make_unique<Quadrant>( SW, m_MinX, m_MinY, m_Center.x, m_Center.y ),
         std::make_unique<Quadrant>( NW, m_MinX, m_Center.y, m_Center.x, m_MaxY )
   };
}

Quadrant::District Quadrant::Spacial::determineChildDistrict( const glm::vec2& pos ) const
{
   if( pos.x >= m_Center.x && pos.y >= m_Center.y ) return NE;
   if( pos.x >= m_Center.x && pos.y <= m_Center.y ) return SE;
   if( pos.x <= m_Center.x && pos.y <= m_Center.y ) return SW;
   if( pos.x <= m_Center.x && pos.y >= m_Center.y ) return NW;

   throw std::runtime_error( "Can't determine quadrant!" );
}

float Quadrant::Spacial::getHeight() const
{
   return m_MaxX - m_MinX;
}

//
// Model
//
Quadrant::Model::Model( float x_min, float y_min, float x_max, float y_max )
{
   const GLuint PositonIndex = Shader::Linked::GetInstance()->GetAttributeLocation( "position" );

   std::vector<glm::vec3> vertices(
      {
         { x_min, y_min, 0.0f }, { x_min, y_max, 0.0f },
         { x_min, y_max, 0.0f }, { x_max, y_max, 0.0f },
         { x_max, y_max, 0.0f }, { x_max, y_min, 0.0f },
         { x_max, y_min, 0.0f }, { x_min, y_min, 0.0f }
      }
   );

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

Quadrant::Model::~Model()
{
   glDeleteBuffers( 1, &m_Vertices );
   glDeleteVertexArrays( 1, &m_VAO );
}

void Quadrant::Model::Draw() const
{
   glBindVertexArray( m_VAO );
   glDrawArrays( GL_LINES, 0, m_NumVertices );
   glBindVertexArray( 0 );
}
