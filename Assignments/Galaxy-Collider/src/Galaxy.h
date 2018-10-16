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
#include "ObjectColors.h"
#include <map>

class Blackhole final : public Particle
{
public:
   Blackhole(float x, float y);

   void Draw() const override;
};

struct GlmVec2Comparator
{
   bool operator()(const glm::vec2& l, const glm::vec2& r) const;
};

class Galaxy
{
public:
   Galaxy(ObjectColors col, float x, float y, float radius, size_t particles);

   void Draw() const;

   static constexpr const float GAMMA = 0.00000005f;

   Blackhole m_Blackhole;
   ObjectColors m_Color;
   std::map<glm::vec2, Particle, GlmVec2Comparator> m_Stars;
};
