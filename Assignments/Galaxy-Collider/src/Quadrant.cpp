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

Quadrant::Quadrant( District disc, float x_min, float y_min, float x_max, float y_max ) :
   m_District( disc ),
   m_MinX( x_min ),
   m_MinY( y_min ),
   m_MaxX( x_max ),
   m_MaxY( y_max ),
   m_Center( m_MinX + ( m_MaxX - m_MinX ) / 2.0f,
             m_MinY + ( m_MaxY - m_MinY ) / 2.0f ),
   m_oModel( m_MinX, m_MinY, m_MaxX, m_MaxY )
{

}

void Quadrant::Draw() const
{
   auto shaderProgram = Shader::Linked::GetInstance();
   shaderProgram->SetUniformInt( "object_color", (GLint)ObjectColors::GREY );
   shaderProgram->SetUniformMat4( "model_matrix", glm::mat4( 1.0f ) );

   m_oModel.Draw();

   shaderProgram->SetUniformInt( "object_color", (GLint)ObjectColors::RED );

   if( auto pval = std::get_if<Particle>( &m_Contains ) )
      pval->Draw();
   else if( auto pval = std::get_if<std::array<std::unique_ptr<Quadrant>, 4>>( &m_Contains ) )
      for( auto& quad : *pval ) quad->Draw();
}

Quadrant::District Quadrant::getDistrict( const glm::vec2& pos ) const
{
   if( pos.x >= m_Center.x && pos.y >= m_Center.y ) return NE;
   if( pos.x >= m_Center.x && pos.y <= m_Center.y ) return SE;
   if( pos.x <= m_Center.x && pos.y <= m_Center.y ) return SW;
   if( pos.x <= m_Center.x && pos.y >= m_Center.y ) return NW;

   throw std::runtime_error( "Can't determine quadrant!" );
}

void Quadrant::insert( const Particle& particle )
{
   if( outsideOfRegion( particle ) )
      return; // Don't even bother =)

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

bool Quadrant::outsideOfRegion(const Particle& particle)
{
   // https://stackoverflow.com/a/42396910/8480874
   auto calcVector = []( glm::vec2 p1, glm::vec2 p2 )->glm::vec2 {
      return{ p2.x - p1.x , -1 * ( p2.y - p1.y ) };
   };

   auto isPointWithinFourCornersOfRectangle = [ calcVector ]( glm::vec2 A, glm::vec2 B, glm::vec2 C, glm::vec2 D, glm::vec2 m )->bool {
      glm::vec2 AB = calcVector( A, B );  float C1 = -1 * ( AB.y*A.x + AB.x*A.y ); float D1 = ( AB.y*m.x + AB.x*m.y ) + C1;
      glm::vec2 AD = calcVector( A, D );  float C2 = -1 * ( AD.y*A.x + AD.x*A.y ); float D2 = ( AD.y*m.x + AD.x*m.y ) + C2;
      glm::vec2 BC = calcVector( B, C );  float C3 = -1 * ( BC.y*B.x + BC.x*B.y ); float D3 = ( BC.y*m.x + BC.x*m.y ) + C3;
      glm::vec2 CD = calcVector( C, D );  float C4 = -1 * ( CD.y*C.x + CD.x*C.y ); float D4 = ( CD.y*m.x + CD.x*m.y ) + C4;

      return 0 >= D1 && 0 >= D4 && 0 <= D2 && 0 >= D3;
   };

   return ! isPointWithinFourCornersOfRectangle( glm::vec2{ m_MaxX, m_MaxY }, glm::vec2{ m_MaxX, m_MinY },
                                                 glm::vec2{ m_MinX, m_MinY }, glm::vec2{ m_MinX, m_MaxY },
                                                 particle.m_Pos
   );
}

Quadrant::Model::Model( float x_min, float y_min, float x_max, float y_max )
{
   const GLuint PositonIndex = Shader::Linked::GetInstance()->GetAttributeLocation( "position" );

   std::vector<glm::vec3> vertices(
      {
         {x_min, y_min, 0.0f}, {x_min, y_max, 0.0f},
         {x_min, y_max, 0.0f}, {x_max, y_max, 0.0f},
         {x_max, y_max, 0.0f}, {x_max, y_min, 0.0f},
         {x_max, y_min, 0.0f}, {x_min, y_min, 0.0f}
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
