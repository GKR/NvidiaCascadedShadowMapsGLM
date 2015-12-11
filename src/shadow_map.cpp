#include <shadow_map.hpp>
#include <camera.hpp>

#include <iostream>

using std::cout;
using std::endl;

/** */
namespace GKR {

struct obj_BoundingSphere {
  vec3 center;
  float radius;
};

#define NUM_OBJECTS 4
obj_BoundingSphere obj_BSphere[NUM_OBJECTS];

/** */
ShadowMap::ShadowMap() :
    m_fbo(0),
    m_texture_array(0),
    m_num_splits(4),
    m_depth_tex_size(2048), // 1024, 2048
    m_split_weight(0.75f) {

  int td = 128; //terrain->getDim()/2;
  obj_BSphere[0].center = vec3(-td, 50.0, -td);
  obj_BSphere[1].center = vec3(-td, 50.0,  td);
  obj_BSphere[2].center = vec3( td, 50.0,  td);
  obj_BSphere[3].center = vec3( td, 50.0, -td);
  obj_BSphere[0].radius = 1.0f;
  obj_BSphere[1].radius = 1.0f;
  obj_BSphere[2].radius = 1.0f;
  obj_BSphere[3].radius = 1.0f;
}

/** */
ShadowMap::~ShadowMap() {
}

/** */
/*mat4 ShadowMap::crop_matrix(int t_split_index) {
  return m_crop_matrices[t_split_index];
}*/

/** */
mat4 ShadowMap::projection_matrix(int t_split_index) {
  return m_projection_matrices[t_split_index];
}

/** */
mat4 ShadowMap::modelview_matrix() {
  return m_modelview;
}

/** */
/*mat4 ShadowMap::bias_matrix() {
  return m_bias;
}*/

/** */
/*Frustum* ShadowMap::frustum(int t_split_index) {
  return &m_frustums[t_split_index];
}*/

/** */
int ShadowMap::num_splits() const {
  return m_num_splits;
}

/** */
int ShadowMap::depth_tex_size() const {
  return m_depth_tex_size;
}

/** */
GLuint ShadowMap::fbo() const {
  return m_fbo;
}

/** */
GLuint ShadowMap::texture() const {
  return m_texture_array;
}

/** */
float* ShadowMap::far_bounds() {
  return &m_far_bounds[0];
}

/** */
float* ShadowMap::texture_matrices() {
  //return &m_texture_matrices[0][0][0];
  return glm::value_ptr(m_texture_matrices[0]);
}

/** */
void ShadowMap::init(Camera* camera) {
  float camera_fov = camera->frustum()->fov();
  float width = camera->viewport()->width();
  float height = camera->viewport()->height();
  float ratio = width / height;

  // note that fov is in radians here and in OpenGL it is in degrees.
  // the 0.2f factor is important because we might get artifacts at
  // the screen borders.
  for(int i = 0 ; i < CSM_MAX_SPLITS ; i++) {
    m_frustums[i].fov(camera_fov / 57.2957795 + 0.2f);
    m_frustums[i].ratio(ratio);
  }

  m_bias = mat4(
    0.5f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.5f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.5f, 0.0f,
    0.5f, 0.5f, 0.5f, 1.0f
  );

  update_split_distances(camera);

  create_fbo();
  create_texture();
}

/** */
void ShadowMap::pre_depth_write(Camera* camera, const vec4& lightdir) {
  mat4 t_modelview = glm::lookAt(
    vec3(0.0, 0.0, 0.0),
    vec3(-lightdir.x, -lightdir.y, -lightdir.z),
    vec3(-1.0f, 0.0f, 0.0f));

  //update_split_distances(camera);
  update_split_frustum_points(camera);
  generate_crop_matrices(t_modelview);
  m_modelview = t_modelview;

  // Required camera matices
  mat4 t_view = camera->view_matrix();
  mat4 t_view_inverse = glm::inverse(t_view);
  mat4 t_projection = camera->projection_matrix();

  update_far_bounds(t_projection, t_view_inverse);
  update_texture_matrices(t_projection, t_view_inverse);
}

/** */
void ShadowMap::update_far_bounds(const mat4& projection, const mat4& view_inverse) {
  for(int i = m_num_splits ; i < CSM_MAX_SPLITS ; i++) {
    m_far_bounds[i] = 0;
  }

  // for every active split
  for(int i = 0 ; i < m_num_splits ; i++) {
    // f[i].fard is originally in eye space - tell's us how far we can see.
    // Here we compute it in camera homogeneous coordinates. Basically, we calculate
    // cam_proj * (0, 0, f[i].fard, 1)^t and then normalize to [0; 1]

    Frustum& split_frustum = m_frustums[i];
    m_far_bounds[i] = 0.5f * (-split_frustum.far() * projection[2][2] + projection[3][2]) / split_frustum.far() + 0.5f;
  }
}

/** */
void ShadowMap::update_texture_matrices(const mat4& projection, const mat4& view_inverse) {
  for(int i = 0 ; i < m_num_splits ; i++) { // for every active split
    // compute a matrix that transforms from camera eye space to light clip space
    // and pass it to the shader through the OpenGL texture matrices, since we
    // don't use them now

    // multiply the light's (bias*crop*proj*modelview) by the inverse camera modelview
    // so that we can transform a pixel as seen from the camera
    m_texture_matrices[i] = m_bias * m_crop_matrices[i] * view_inverse;

    // compute a normal matrix for the same thing (to transform the normals)
    // Basically, N = ((L)^-1)^-t

    /* TODO: Why is this here?
    glm::mat4 t_mat_nm = glm::inverse(t_mat_texture);
    t_mat_nm = glm::transpose(t_mat_nm);

    glActiveTexture(GL_TEXTURE0 + (GLenum)(i+4));
    glMatrixMode(GL_TEXTURE);
    glLoadMatrixf(glm::value_ptr(t_mat_nm));*/
  }
}

/** */
void ShadowMap::create_fbo() {
  glGenFramebuffersEXT(1, &m_fbo);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, m_fbo);
  glDrawBuffer(GL_NONE);
  glReadBuffer(GL_NONE);
  glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
}

/** */
void ShadowMap::create_texture() {
  if(m_texture_array) {
    glDeleteTextures(1, &m_texture_array);
  }

  glGenTextures(1, &m_texture_array);
  glBindTexture(GL_TEXTURE_2D_ARRAY, m_texture_array);
  glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_DEPTH_COMPONENT24, m_depth_tex_size, m_depth_tex_size, CSM_MAX_SPLITS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);

  glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_COMPARE_MODE, GL_NONE);
  //glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);

  glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, 0);
}

/**
 * Computes the near and far distances for every frustum slice
 * in camera eye space - that is, at what distance does a slice start and end
*/
void ShadowMap::update_split_distances(Camera* camera) {
  float nd = camera->frustum()->near();
  float fd = camera->frustum()->far();

  float lambda = m_split_weight;
  float ratio = fd / nd;
  m_frustums[0].near(nd);

  for(int i = 1 ; i < m_num_splits ; i++) {
    float si = i / (float)m_num_splits;

    float t_near = lambda * (nd * powf(ratio, si)) + (1 - lambda) * (nd + (fd - nd) * si);
    float t_far = t_near * 1.005f;
    m_frustums[i].near(t_near);
    m_frustums[i-1].far(t_far);
  }
  m_frustums[m_num_splits-1].far(fd);
}

/** Compute the camera frustum slice boundary points in world space */
void ShadowMap::update_split_frustum_points(Camera* camera) {
  vec3 center = camera->position();
  vec3 view_dir = camera->target();

  vec3 up(0.0f, 1.0f, 0.0f);
  vec3 right = glm::cross(view_dir, up);

  for(int i = 0 ; i < m_num_splits ; i++) {
    Frustum& t_frustum = m_frustums[i];

    vec3 fc = center + view_dir * t_frustum.far();
    vec3 nc = center + view_dir * t_frustum.near();

    right = glm::normalize(right);
    up = glm::normalize(glm::cross(right, view_dir));

    // these heights and widths are half the heights and widths of
    // the near and far plane rectangles
    float near_height = tan(t_frustum.fov() / 2.0f) * t_frustum.near();
    float near_width = near_height * t_frustum.ratio();
    float far_height = tan(t_frustum.fov() / 2.0f) * t_frustum.far();
    float far_width = far_height * t_frustum.ratio();

    t_frustum.m_points[0] = nc - up * near_height - right * near_width;
    t_frustum.m_points[1] = nc + up * near_height - right * near_width;
    t_frustum.m_points[2] = nc + up * near_height + right * near_width;
    t_frustum.m_points[3] = nc - up * near_height + right * near_width;

    t_frustum.m_points[4] = fc - up * far_height - right * far_width;
    t_frustum.m_points[5] = fc + up * far_height - right * far_width;
    t_frustum.m_points[6] = fc + up * far_height + right * far_width;
    t_frustum.m_points[7] = fc - up * far_height + right * far_width;
  }
}

/**
 * Adjust the view frustum of the light, so that it encloses the camera frustum slice fully.
 * Note that this function sets the projection matrix as it sees best fit
 * minZ is just for optimization to cull trees that do not affect the shadows
*/
void ShadowMap::generate_crop_matrices(const mat4& t_modelview) {
  mat4 t_projection;
  for(int i = 0 ; i < m_num_splits ; i++) {
    Frustum& t_frustum = m_frustums[i];

    vec3 tmax(-1000.0f, -1000.0f, 0.0f);
    vec3 tmin(1000.0f, 1000.0f, 0.0f);

    // find the z-range of the current frustum as seen from the light
    // in order to increase precision

    // note that only the z-component is need and thus
    // the multiplication can be simplified
    // transf.z = shad_modelview[2] * f.point[0].x + shad_modelview[6] * f.point[0].y + shad_modelview[10] * f.point[0].z + shad_modelview[14];
    vec4 t_transf = t_modelview * vec4(t_frustum.m_points[0], 1.0f);

    tmin.z = t_transf.z;
    tmax.z = t_transf.z;
    for(int j = 1 ; j < 8 ; j++) {
      t_transf = t_modelview * vec4(t_frustum.m_points[j], 1.0f);
      if(t_transf.z > tmax.z) { tmax.z = t_transf.z; }
      if(t_transf.z < tmin.z) { tmin.z = t_transf.z; }
    }

    // make sure all relevant shadow casters are included
    // note that these here are dummy objects at the edges of our scene
    /*for(int i = 0 ; i < NUM_OBJECTS ; i++) {
      t_transf = t_modelview * vec4(obj_BSphere[i].center, 1.0f);
      if(t_transf.z + obj_BSphere[i].radius > tmax.z) {
        tmax.z = t_transf.z + obj_BSphere[i].radius;
      }
      //if(transf.z - obj_BSphere[i].radius < minZ) { minZ = transf.z - obj_BSphere[i].radius; }
    }*/

    tmax.z += 50; // TODO: This solves the dissapearing shadow problem. but how to fix?

    mat4 t_ortho = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -tmax.z, -tmin.z);
    mat4 t_shad_mvp = t_ortho * t_modelview;

    // find the extends of the frustum slice as projected in light's homogeneous coordinates
    for(int j = 0 ; j < 8 ; j++) {
      t_transf = t_shad_mvp * vec4(t_frustum.m_points[j], 1.0f);

      t_transf.x /= t_transf.w;
      t_transf.y /= t_transf.w;

      if(t_transf.x > tmax.x) { tmax.x = t_transf.x; }
      if(t_transf.x < tmin.x) { tmin.x = t_transf.x; }
      if(t_transf.y > tmax.y) { tmax.y = t_transf.y; }
      if(t_transf.y < tmin.y) { tmin.y = t_transf.y; }
    }

    vec2 tscale(2.0f / (tmax.x - tmin.x), 2.0f / (tmax.y - tmin.y));
    vec2 toffset(-0.5f * (tmax.x + tmin.x) * tscale.x, -0.5f * (tmax.y + tmin.y) * tscale.y);

    mat4 t_shad_crop;
    t_shad_crop[0][0] = tscale.x;
    t_shad_crop[1][1] = tscale.y;
    t_shad_crop[0][3] = toffset.x;
    t_shad_crop[1][3] = toffset.y;
    t_shad_crop = glm::transpose(t_shad_crop);

    t_projection = t_shad_crop * t_ortho;

    //return tmin.z;

    // Store the projection matrix
    m_projection_matrices[i] = t_projection;

    // store the product of all shadow matries for later
    m_crop_matrices[i] = t_projection * t_modelview;
  }
}

}
