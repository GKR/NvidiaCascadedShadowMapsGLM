#ifndef GKR_FRUSTUM_HPP
#define GKR_FRUSTUM_HPP

#include <math.hpp>

/** */
namespace GKR {

/** */
class Frustum {
private:  
  float m_fov;
  float m_ratio;
  float m_near;
  float m_far;

public:
  vec3 m_points[8];
  
  Frustum();
  
  /** */
  void set(float t_fov, float t_ratio, float t_near, float t_far);
  
  float fov() const;
  float ratio() const;
  float near() const;
  float far() const;
  
  void near(float t_near);
  void far(float t_far);
  void fov(float t_fov);
  void ratio(float t_ratio);
};

}

#endif
