#ifndef GKR_SHADOW_MAP_HPP
#define GKR_SHADOW_MAP_HPP

#include <math.hpp>
#include <frustum.hpp>

#include <GL/glew.h>

/** */
namespace GKR {

#define CSM_MAX_SPLITS 4

class Camera;

/** */
class ShadowMap {
private:
  GLuint m_fbo;
  GLuint m_texture_array;
  
  int m_num_splits;
  int m_depth_tex_size;
  float m_split_weight;
  float m_far_bounds[CSM_MAX_SPLITS];
  
  Frustum m_frustums[CSM_MAX_SPLITS];
  
  mat4 m_bias;
  mat4 m_modelview;
  mat4 m_crop_matrices[CSM_MAX_SPLITS];
  mat4 m_projection_matrices[CSM_MAX_SPLITS];
  mat4 m_texture_matrices[CSM_MAX_SPLITS];
public:
  ShadowMap();
  ~ShadowMap();
  
  void init(Camera* camera);
  
  /** Generate crop and projection matrices */
  void pre_depth_write(Camera* camera, const vec4& lightdir);
  
  /** Getters for the various matrices (shadow map generation) */
  mat4 projection_matrix(int t_split_index);
  mat4 modelview_matrix();
  
  //Frustum* frustum(int t_split_index);
  
  int num_splits() const;
  int depth_tex_size() const;
  
  /** OpenGL handles for FBO and texture array */
  GLuint fbo() const;
  GLuint texture() const;
  
  /** Array of depth far values to use in shader lookup during rendering */
  float* far_bounds();
  
  /** Returns texture matrices as float array (that can be passed to shader) */
  float* texture_matrices();
private:
  void create_fbo();
  void create_texture();
  
  void update_split_distances(Camera* camera);
  void update_split_frustum_points(Camera* camera);
  void generate_crop_matrices(const mat4& t_modelview);
  
  /** Update far bounds */
  void update_far_bounds(const mat4& projection, const mat4& view_inverse);
  void update_texture_matrices(const mat4& projection, const mat4& view_inverse);
};

}

#endif

