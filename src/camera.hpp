#ifndef GKR_CAMERA_HPP
#define GKR_CAMERA_HPP

#include <math.hpp>
#include <frustum.hpp>

/** */
namespace GKR {

/** Some forward declarations */
class Camera;

/** */
class Viewport {
private:
  int m_x;
  int m_y;
  int m_width;
  int m_height;
  int m_half_width;
  int m_half_height;

public:
  Viewport();
  void set(int n_x, int n_y, int n_width, int n_height);
  
  int x() const;
  int y() const;
  int width() const;
  int height() const;
  int half_width() const;
  int half_height() const;
};

/** */
class CameraTracker {
private:
  Camera* m_camera;
  vec2 m_prev_point;
  float m_sensitivity;

public:
  CameraTracker();

  /** Sets the camera this tracker will use */
  void set_camera(Camera* t_camera);

  /** Marks the start of mouse tracking (e.g. called from onMouseClick() ) */
  void begin(int x, int y);

  /** Updates mouse tracking (e.g. called from onMouseMove() ) */
  void track(int x, int y);
};

/** */
class CameraMover {
private:
  Camera* m_camera;

  bool m_forward;
  bool m_backward;
  bool m_left;
  bool m_right;
public:
  CameraMover();

  void update(double dt);

  void set_camera(Camera* t_camera);

  void forward(bool t_value);
  void backward(bool t_value);
  void left(bool t_value);
  void right(bool t_value);
};

/** */
class Camera {
private:
  Frustum m_frustum;
  
  vec3 m_position;
  vec3 m_target;
  vec3 m_up;

  Viewport m_viewport;
  CameraTracker m_tracker;
  CameraMover m_mover;
public:
  Camera();
  Camera(const vec3& t_position, const vec3& t_target, const vec3& t_up);
  ~Camera();

  void update(double dt);
  
  /** Rotates target vector by angle, given */
  void rotate(float angle, const vec3& axis);

  /** Returns the view matrix of the camera */
  mat4 view_matrix();
  
  /** Returns the projection matrix defined by the current Frustum */
  mat4 projection_matrix();

  /** Returns the viewport used by this camera */
  Viewport* viewport();

  /** Returns the mouse track class that can be used to control this camera */
  CameraTracker* tracker();

  /** Returns the class that contols the camera movement */
  CameraMover* mover();

  vec3 position() const;
  vec3 target() const;
  vec3 up() const;

  void position(const vec3& t_position);
  void target(const vec3& t_target);
  void up(const vec3& t_up);
  
  /** */
  Frustum* frustum();
};

}

#endif
