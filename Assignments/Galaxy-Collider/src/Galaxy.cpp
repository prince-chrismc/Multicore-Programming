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

#include "Galaxy.h"
#include "Linked.h"
#include <tuple>
#include <random>
#include <limits>

Blackhole::Blackhole( float x, float y ) : Particle( x, y, 1453.485f )
{
}

void Blackhole::Draw() const
{
   Shader::Linked::GetInstance()->SetUniformInt( "object_color", (GLint)ObjectColors::YELLOW );
   Particle::Draw();
}

bool GlmVec2Comparator::operator()(const glm::vec2& l, const glm::vec2& r) const
{
   return std::tie(l.x, l.y) < std::tie(r.x, r.y);
}

Galaxy::Galaxy( ObjectColors col, float x, float y, float radius, size_t particles ) : m_Blackhole( x, y ), m_Color( col )
{
   static constexpr const long double PI = 3.141592653589793238462643383279502884L;

   std::random_device rd;
   std::mt19937 gen( rd() );
   std::lognormal_distribution<long double> numGenPos( 0.0L, 1.8645L );
   std::lognormal_distribution<long double> numGenMass( 2.0L, 125.873275L );
   
   for( size_t i = 0; i < particles; i++ )
   {
      float a = static_cast<float>( numGenPos( gen ) * 2.0L * PI );
      float r = static_cast<float>( sqrt( numGenPos( gen ) * radius ) );

      // in Cartesian coordinates
      float rel_x = r * cos( a );
      float rel_y = r * sin( a );

      m_Stars.insert( std::make_pair( glm::vec2{ rel_x + x, rel_y + y }, Particle( rel_x + x, rel_y + y, 0.76f + static_cast<float>( numGenMass( gen ) / std::numeric_limits<float>::max() ) ) ) );
   }
}

void Galaxy::Draw() const
{
   m_Blackhole.Draw();
   Shader::Linked::GetInstance()->SetUniformInt( "object_color", (GLint)m_Color );
   for( auto& Particle : m_Stars ) Particle.second.Draw();
}
