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

#include "Camera.h"
#include "glm/gtc/matrix_transform.hpp"

std::once_flag Camera::s_Flag;
std::shared_ptr<Camera> Camera::s_Instance;

const glm::vec3 Camera::CENETER(0.0f, 0.0f, 0.0f);
const glm::vec3 Camera::UP(0.0f, 1.0f, 0.0f);
const glm::vec3 Camera::EYE(0.0f, 0.0f, 35.0f);

glm::mat4 Camera::GetViewMatrix() const
{
   glm::mat4 view_matrix;
   view_matrix = glm::lookAt(EYE, CENETER, UP);
   //mouse actions
   view_matrix = glm::translate(view_matrix, glm::vec3(m_panx, 0.0f, 0.0f));
   view_matrix = glm::rotate(view_matrix, glm::radians(m_tilty), glm::vec3(0.0f, 0.0f, 1.0f)); // apply tilt on y axis
   view_matrix = glm::scale(view_matrix, glm::vec3(m_zoomz));
   //arrow key actions
   view_matrix = glm::rotate(view_matrix, glm::radians(m_rotx), glm::vec3(1.0f, 0.0f, 0.0f)); // apply rotation on x axis
   view_matrix = glm::rotate(view_matrix, glm::radians(m_roty), glm::vec3(0.0f, 1.0f, 0.0f)); // apply rotation on y axis

   return view_matrix;
}

void Camera::AdjustTiltY(const float & adj)
{
   m_tilty += adj;

   if (m_tilty > TILT_UPPER_LIMIT) m_tilty = TILT_UPPER_LIMIT;
   if (m_tilty < TILT_LOWER_LIMIT) m_tilty = TILT_LOWER_LIMIT;
}

void Camera::AdjustZoomZ(const float & adj)
{
   m_zoomz += adj;

   if (m_zoomz > ZOOM_UPPER_LIMIT) m_zoomz = ZOOM_UPPER_LIMIT;
   if (m_zoomz < ZOOM_LOWER_LIMIT) m_zoomz = ZOOM_LOWER_LIMIT;
}
