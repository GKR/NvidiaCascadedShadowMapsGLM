#include <camera.hpp>

/** */
namespace GKR {

/** */
Viewport::Viewport() :
    m_x(0),
    m_y(0),
    m_width(0),
    m_height(0),
    m_half_width(0),
    m_half_height(0) {
}

/** */
void Viewport::set(int n_x, int n_y, int n_width, int n_height) {
  m_x = n_x;
  m_y = n_y;
  m_width = n_width;
  m_height = n_height;
  m_half_width = m_width / 2;
  m_half_height = m_height / 2;
}

/** */
int Viewport::x() const {
  return m_x;
}

/** */
int Viewport::y() const {
  return m_y;
}

/** */
int Viewport::width() const {
  return m_width;
}

/** */
int Viewport::height() const {
  return m_height;
}

/** */
int Viewport::half_width() const {
  return m_half_width;
}

/** */
int Viewport::half_height() const {
  return m_half_height;
}

/** */
CameraTracker::CameraTracker() :
    m_camera(NULL),
    m_prev_point(0.0),
    m_sensitivity(0.005) {
}

/** */
void CameraTracker::set_camera(Camera* t_camera) {
  m_camera = t_camera;
}

/** */
void CameraTracker::begin(int x, int y) {
  if(!m_camera) { return; }
  m_prev_point = vec2(x - m_camera->viewport()->half_width(), y - m_camera->viewport()->half_height());
}

/** */
void CameraTracker::track(int x, int y) {
  if(!m_camera) { return; }

  vec2 n_point(x, y);
  vec2 n_rotation(0.0f);
  vec3 t_rotation_axis(0.0f, 1.0f, 0.0f);

  n_point.x -= m_camera->viewport()->half_width();
  n_point.y -= m_camera->viewport()->half_height();

  n_rotation.x = -(float)(n_point.x - m_prev_point.x) * m_sensitivity;
  n_rotation.y = -(float)(n_point.y - m_prev_point.y) * m_sensitivity;

  m_camera->rotate(n_rotation.x, t_rotation_axis);

  vec3 t_camtarget = m_camera->target();
  t_rotation_axis = glm::normalize(vec3(-t_camtarget.z, 0.0f, t_camtarget.x));
  m_camera->rotate(n_rotation.y, t_rotation_axis);

  m_prev_point = n_point;
}

CameraMover::CameraMover() :
    m_camera(NULL),
    m_forward(false),
    m_backward(false),
    m_left(false),
    m_right(false) {
}

/** */
void CameraMover::set_camera(Camera* t_camera) {
  m_camera = t_camera;
}

void CameraMover::forward(bool t_value) {
  if(m_backward) {
    m_backward = false;
  }
  m_forward = t_value;
}
void CameraMover::backward(bool t_value) {
  if(m_forward) {
    m_forward = false;
  }
  m_backward = t_value;
}
void CameraMover::left(bool t_value) {
  if(m_right) {
    m_right = false;
  }
  m_left = t_value;
}
void CameraMover::right(bool t_value) {
  if(m_left) {
    m_left = false;
  }
  m_right = t_value;
}

void CameraMover::update(double dt) {
  if(!m_camera) { return; }

  vec3 t_campos = m_camera->position();
  vec3 t_camtarget = m_camera->target();

  // Move 5 units per second
  float t_speed = 5.0f * dt;

  if(m_forward) {
    t_campos += t_camtarget * t_speed;
  } else if(m_backward) {
    t_campos -= t_camtarget * t_speed;
  }

  if(m_left) {
    t_campos += vec3(t_camtarget.z, 0.0f, -t_camtarget.x) * t_speed;
  } else if(m_right) {
    t_campos += vec3(-t_camtarget.z, 0.0f, t_camtarget.x) * t_speed;
  }

  m_camera->position(t_campos);
}

/** */
Camera::Camera() :
    m_position(0.0f),
    m_target(0.0f, 0.0f, 1.0f),
    m_up(0.0f, 1.0f, 0.0f) {
  m_tracker.set_camera(this);
  m_mover.set_camera(this);
}

Camera::Camera(const vec3& t_position, const vec3& t_target, const vec3& t_up) :
    m_position(t_position),
    m_target(t_target),
    m_up(t_up) {
  m_tracker.set_camera(this);
  m_mover.set_camera(this);
}

Camera::~Camera() {
}

/** */
void Camera::update(double dt) {
  m_mover.update(dt);
}

/** */
void Camera::rotate(float angle, const vec3& axis) {
  vec3 n_target = glm::rotate(m_target, glm::degrees(angle), axis);
  m_target = glm::normalize(n_target);

  /*vec3 n_target(0.0);

  float c = cos(angle);
  float s = sin(angle);

  n_target.x  = (x * x * (1.0 - c) + c) * m_target.x;
  n_target.x += (x * y * (1.0 - c) - z * s) * m_target.y;
  n_target.x += (x * z * (1.0 - c) + y * s) * m_target.z;
  
  n_target.y  = (y * x * (1.0 - c) + z * s) * m_target.x;
  n_target.y += (y * y * (1.0 - c) + c) * m_target.y;
  n_target.y += (y * z * (1.0 - c) - x * s) * m_target.z;

  n_target.z  = (x * z * (1.0 - c) - y * s) * m_target.x;
  n_target.z += (y * z * (1.0 - c) + x * s) * m_target.y;
  n_target.z += (z * z * (1.0 - c) + c) * m_target.z;

  m_target = glm::normalize(n_target);*/
}

/** */
mat4 Camera::view_matrix() {
  vec3 n_target = m_position + m_target;
  mat4 t_view_matrix = glm::lookAt(m_position, n_target, m_up);
  return t_view_matrix;
}

/** */
mat4 Camera::projection_matrix() {
  return glm::perspective(m_frustum.fov(), m_frustum.ratio(), m_frustum.near(), m_frustum.far());
}

/** */
Viewport* Camera::viewport() {
  return &m_viewport;
}

/** */
CameraTracker* Camera::tracker() {
  return &m_tracker;
}

/** */
CameraMover* Camera::mover() {
  return &m_mover;
}

/** */
vec3 Camera::position() const {
  return m_position;
}

/** */
vec3 Camera::target() const {
  return m_target;
}

/** */
vec3 Camera::up() const {
  return m_up;
}

/** */
Frustum* Camera::frustum() {
  return &m_frustum;
}

/** */
void Camera::position(const vec3& t_position) {
  m_position = t_position;
}

/** */
void Camera::target(const vec3& t_target) {
  m_target = t_target;
}

/** */
void Camera::up(const vec3& t_up) {
  m_up = t_up;
}

}
