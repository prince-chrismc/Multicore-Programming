/*
MIT License

Copyright (c) 2017 Chris McArthur, prince.chrismc(at)gmail(dot)com

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
#include "glm/vec3.hpp"
#include "glm/detail/type_mat4x4.hpp"
#include <mutex>
#include <memory>

class Camera
{
public:
   ~Camera() = default;
   Camera(const Camera&) = delete;
   Camera& operator=(const Camera&) = delete;

   static std::shared_ptr<Camera> GetInstance() { std::call_once(s_Flag, []() { s_Instance.reset(new Camera()); }); return s_Instance; }

   glm::mat4 GetViewMatrix() const;

   void IncrementRotationX() { m_rotx += ROTATION_STEP_SIZE; }
   void DecrementRotationX() { m_rotx -= ROTATION_STEP_SIZE; }
   void IncrementRotationY() { m_roty += ROTATION_STEP_SIZE; }
   void DecrementRotationY() { m_roty -= ROTATION_STEP_SIZE; }

   void AdjustPanX(const float& adj) { m_panx += adj; }
   void AdjustTiltY(const float& adj);
   void AdjustZoomZ(const float& adj);

   void Reset() { m_rotx = 0.0f; m_roty = 0.0f; m_panx = 0.0f; m_tilty = 0.0f; m_zoomz = 1.0f; }

private:
   Camera() : m_rotx(0.0f), m_roty(0.0f), m_panx(0.0f), m_tilty(0.0f), m_zoomz(1.0f) {}

   // Members
   float m_rotx;
   float m_roty;
   float m_panx;
   float m_tilty;
   float m_zoomz;

   // Constant vectors
   static const glm::vec3 CENETER;
   static const glm::vec3 UP;
   static const glm::vec3 EYE;

   static constexpr float ROTATION_STEP_SIZE = 5.0f;

   static constexpr float TILT_UPPER_LIMIT = 89.0f;
   static constexpr float TILT_LOWER_LIMIT = -89.0f;

   static constexpr float ZOOM_UPPER_LIMIT = 5.0f;
   static constexpr float ZOOM_LOWER_LIMIT = 0.5f;

   static std::once_flag s_Flag;
   static std::shared_ptr<Camera> s_Instance;
};
