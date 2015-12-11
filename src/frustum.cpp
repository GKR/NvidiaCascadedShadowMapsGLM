#include <frustum.hpp>

/** */
namespace GKR {

/** */
Frustum::Frustum() :
    m_fov(45.0),
    m_ratio(0.5),
    m_near(1.0),
    m_far(200.0) {
}

/** */
void Frustum::set(float t_fov, float t_ratio, float t_near, float t_far) {
  m_fov = t_fov;
  m_ratio = t_ratio;
  m_near = t_near;
  m_far = t_far;
}

/** */
float Frustum::fov() const {
  return m_fov;
}

/** */
float Frustum::ratio() const {
  return m_ratio;
}

/** */
float Frustum::near() const {
  return m_near;
}

/** */
float Frustum::far() const {
  return m_far;
}

/** */
void Frustum::near(float t_near) {
  m_near = t_near;
}

void Frustum::far(float t_far) {
  m_far = t_far;
}

/** */
void Frustum::fov(float t_fov) {
  m_fov = t_fov;
}

/** */
void Frustum::ratio(float t_ratio) {
  m_ratio = t_ratio;
}

}
