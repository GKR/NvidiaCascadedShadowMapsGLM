// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.

#include <math.h>
#define NV_REPORT_COMPILE_ERRORS
#include <nvShaderUtils.h>
#include "main.h"

#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;

GKR::Camera m_camera;
GKR::ShadowMap m_shadow_map;

float cam_pos[3] = {75.5, 30.0f, -110};
float cam_view[3] = {-0.7f, 0.0f, 0.7f};

//GLfloat light_dir[4] = {0.2f, 0.99f, 0.0f , 0.0f};
vec4 m_light_dir(0.2f, 0.99f, 0.0f , 0.0f);

bool rotate_light_dir = false;

const float sensitivity = 0.005;
const float walk_speed = 0.5;

static int old_x, old_y;
int half_width;
int half_height;

GKR::Camera* get_camera() {
  return &m_camera;
}

GKR::ShadowMap* get_shadow_map() {
  return &m_shadow_map;
}

void normalize(float *v) {
  float magnitude = sqrt(v[0]*v[0] + v[1]*v[1] + v[2]*v[2]);
  v[0] /= magnitude;
  v[1] /= magnitude;
  v[2] /= magnitude;
}

void rotate_view(float *view, float angle, float x, float y, float z) {
  float new_x;
  float new_y;
  float new_z;

  float c = cos(angle);
  float s = sin(angle);

  new_x  = (x*x*(1-c) + c)	* view[0];
  new_x += (x*y*(1-c) - z*s)	* view[1];
  new_x += (x*z*(1-c) + y*s)	* view[2];
  
  new_y  = (y*x*(1-c) + z*s)	* view[0];
  new_y += (y*y*(1-c) + c)	* view[1];
  new_y += (y*z*(1-c) - x*s)	* view[2];

  new_z  = (x*z*(1-c) - y*s)	* view[0];
  new_z += (y*z*(1-c) + x*s)	* view[1];
  new_z += (z*z*(1-c) + c)	* view[2];

  view[0] = new_x;
  view[1] = new_y;
  view[2] = new_z;

  normalize(view);
}

void motion(int x, int y) {

  int orig_x = x;
  int orig_y = y;
  float rot_x, rot_y;
  float rot_axis[3];

  x -= half_width;
  y -= half_height;

  rot_x = -(float)(x - old_x) * sensitivity;
  rot_y = -(float)(y - old_y) * sensitivity;

  old_x = x;
  old_y = y;

  //if(GetAsyncKeyState(VK_SHIFT))
  if(rotate_light_dir) {
    m_light_dir.y += rot_y;
    if(m_light_dir.y < 0.2f) {
      m_light_dir.y = 0.2f;
    }

    m_light_dir = glm::normalize(m_light_dir);
    m_light_dir = glm::rotate(m_light_dir, glm::degrees(-rot_x), m_camera.target());
    //rotate_view(light_dir, -rot_x, cam_view[0], cam_view[1], cam_view[2]);
  } else {
    m_camera.tracker()->track(orig_x, orig_y);

    rotate_view(cam_view, rot_x, 0.0f, 1.0f, 0.0f);

    rot_axis[0] = -cam_view[2];
    rot_axis[1] = 0.0f;
    rot_axis[2] = cam_view[0];

    normalize(rot_axis);

    rotate_view(cam_view, rot_y, rot_axis[0], rot_axis[1], rot_axis[2]);
  }
}

glm::mat4 camLook2() {
  /*glm::vec3 eye(cam_pos[0], cam_pos[1], cam_pos[2]);
  glm::vec3 center(cam_pos[0] + cam_view[0], cam_pos[1] + cam_view[1], cam_pos[2] + cam_view[2]);
  glm::vec3 up(0.0f, 1.0f, 0.0f);
  
  glm::mat4 t_mat_cam = glm::lookAt(eye, center, up);
  return t_mat_cam;*/

  return m_camera.view_matrix();
}

void camLook() {
  gluLookAt(
    cam_pos[0], cam_pos[1], cam_pos[2],
    cam_pos[0] + cam_view[0], cam_pos[1] + cam_view[1], cam_pos[2] + cam_view[2],
    0.0f, 1.0f, 0.0f);
}

void compare_matrix(float* t_mat_orig, const glm::mat4& t_glm_mat) {
  cout << "==============" << endl;
  cout << "-- GLM MAT4 --" << endl;
  for(int i = 0 ; i < 4 ; i++) {
    for(int j = 0 ; j < 4 ; j++) {
      cout << "[" << t_glm_mat[i][j] << "]";
    }
    cout << endl;
  }
  cout << "-- GLM MAT4 --" << endl;
  
  cout << "-- FLOAT MAT4 --" << endl;
  for(int i = 0 ; i < 4 ; i++) {
    for(int j = 0 ; j < 4 ; j++) {
      cout << "[" << t_mat_orig[i*4+j] << "]";
    }
    cout << endl;
  }
  cout << "-- FLOAT MAT4 --" << endl;
  cout << "================" << endl;
}

void updateKeys() {
  /*if(GetAsyncKeyState('W')){
    cam_pos[0] += cam_view[0] * walk_speed;
    cam_pos[1] += cam_view[1] * walk_speed;
    cam_pos[2] += cam_view[2] * walk_speed;
  }
  if(GetAsyncKeyState('S')){
    cam_pos[0] -= cam_view[0] * walk_speed;
    cam_pos[1] -= cam_view[1] * walk_speed;
    cam_pos[2] -= cam_view[2] * walk_speed;
  }
  if(GetAsyncKeyState('A')){
    cam_pos[0] += cam_view[2] * walk_speed;
        
    cam_pos[2] -= cam_view[0] * walk_speed;
  }
  if(GetAsyncKeyState('D')){
    cam_pos[0] -= cam_view[2] * walk_speed;
        
    cam_pos[2] += cam_view[0] * walk_speed;
  }

  if(GetAsyncKeyState(VK_SPACE)){
    cam_pos[1] += walk_speed;
  }*/
}

void mouse(int button, int state, int x, int y) {
  old_x = x - half_width;
  old_y = y - half_height;
  m_camera.tracker()->begin(x, y);
  glutPostRedisplay();
}

void idle() {
  glutPostRedisplay();
}

void key_down(unsigned char c, int x, int y) {
  switch (c) {
    case 27: //escape key
    case 'q':
    case 'Q':
      exit(0);
      break;
    case 'r': {
      rotate_light_dir = true; break;
    }
    case 'w': {
      m_camera.mover()->forward(true); break;
    }
    case 's': {
      m_camera.mover()->backward(true); break;
    }
    case 'a': {
      m_camera.mover()->left(true); break;
    }
    case 'd': {
      m_camera.mover()->right(true); break;
    }
  }
}

void key_up(unsigned char c, int x, int y) {
  switch (c) {
    case 'r': {
      rotate_light_dir = false; break;
    }
    case 'w': {
      m_camera.mover()->forward(false); break;
    }
    case 's': {
      m_camera.mover()->backward(false); break;
    }
    case 'a': {
      m_camera.mover()->left(false); break;
    }
    case 'd': {
      m_camera.mover()->right(false); break;
    }
  }
}

void keys( unsigned char c, int x, int y) {
  
	switch (c) {
		case 27: //escape key
		case 'q':
		case 'Q':
			exit(0);
			break;
    case 'w': {
      cam_pos[0] += cam_view[0] * walk_speed;
      cam_pos[1] += cam_view[1] * walk_speed;
      cam_pos[2] += cam_view[2] * walk_speed;
      break;
    }
    case 's': {
      cam_pos[0] -= cam_view[0] * walk_speed;
      cam_pos[1] -= cam_view[1] * walk_speed;
      cam_pos[2] -= cam_view[2] * walk_speed;
      break;
    }
    case 'a': {
      cam_pos[0] += cam_view[2] * walk_speed;
      cam_pos[2] -= cam_view[0] * walk_speed;
      break;
    }
    case 'd': {
      cam_pos[0] -= cam_view[2] * walk_speed;
      cam_pos[2] += cam_view[0] * walk_speed;
      break;
    }
		/*case '1':
			cur_num_splits = 1;
			break;
		case '2':
			cur_num_splits = 2;
			break;
		case '3':
			cur_num_splits = 3;
			break;
		case '4':
			cur_num_splits = 4;
			break;*/
    case 'g': shadow_type = 0; break;
    case 'h': shadow_type = 1; break;
    case 'j': shadow_type = 2; break;
    case 'k': shadow_type = 3; break;
    case 'l': shadow_type = 4; break;
		case '`':
			show_depth_tex = !show_depth_tex;
			break;
		case 'p':
			printf("cam pos: %f %f %f\n", cam_pos[0], cam_pos[1], cam_pos[2]);
			break;
        /*case '+':
            split_weight += 0.05f;
            printf( "split_weight changed to %f\n", split_weight);
            break;
        case '-':
            split_weight -= 0.05f;
            printf( "split_weight changed to %f\n", split_weight);
            break;*/
	}

	glutPostRedisplay();
}

void reshape(int w, int h) {
  width = w;
  height = h;
  half_height = height/2;
  half_width = width/2;

  float ratio = (float)width / (float)height;
  m_camera.position(vec3(75.5f, 30.0f, -110.0f));
  m_camera.target(vec3(-0.7f, 0.0f, 0.7f));
  m_camera.viewport()->set(0, 0, width, height);
  m_camera.frustum()->set(45.0, ratio, 1.0, FAR_DIST);

  m_shadow_map.init(&m_camera);
  
  //glMatrixMode(GL_PROJECTION);
  //glLoadIdentity();
  //gluPerspective(45.0, (double)width/(double)height, 1.0, FAR_DIST);

  glViewport(0, 0, width, height);
  /*for(int i = 0 ; i < MAX_SPLITS ; i++) {
    f[i].fov = CAMERA_FOV / 57.2957795 + 0.2f;
    f[i].ratio = (double)width/(double)height;
  }*/
}

void regenerateDepthTex(GLuint depth_size) {
  /*glDeleteTextures(1, &depth_tex_ar);
  glGenTextures(1, &depth_tex_ar);

  glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, depth_tex_ar);
  glTexImage3D(GL_TEXTURE_2D_ARRAY_EXT, 0, GL_DEPTH_COMPONENT24, depth_size, depth_size, MAX_SPLITS, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
  glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_FUNC, GL_LEQUAL);*/
}

void menu(int m) {
	switch (m) {
		case 'q':
			exit(0);
			break;
		/*case '1':
			cur_num_splits = 1;
			break;
		case '2':
			cur_num_splits = 2;
			break;
		case '3':
			cur_num_splits = 3;
			break;
		case '4':
			cur_num_splits = 4;
			break;*/
		case '`':
			show_depth_tex = !show_depth_tex;
			break;
		case 0:
			shadow_type = 0;
			break;
		case 1:
			shadow_type = 1;
			break;
		case 2:
			shadow_type = 2;
			break;
		case 3:
			shadow_type = 3;
			break;
        case 4:
			shadow_type = 4;
			break;
        case 5:
			shadow_type = 5;
			break;
        case 6:
			shadow_type = 6;
			break;
        case 7:
			shadow_type = 7;
			break;
        case 8:
			shadow_type = 8;
			break;
		/*case 10:
			depth_size = 512;
			regenerateDepthTex(512);
			break;
		case 11:
			depth_size = 1024;
			regenerateDepthTex(1024);
			break;
		case 12:
			depth_size = 2048;
			regenerateDepthTex(2048);
			break;
		case 13:
			depth_size = 4096;
			regenerateDepthTex(4096);
			break;*/
	}
	glutPostRedisplay();
}

void cameraInverse(float dst[16], float src[16]) {
  dst[0] = src[0];
  dst[1] = src[4];
  dst[2] = src[8];
  dst[3] = 0.0f;
  dst[4] = src[1];
  dst[5] = src[5];
  dst[6]  = src[9];
  dst[7] = 0.0f;
  dst[8] = src[2];
  dst[9] = src[6];
  dst[10] = src[10];
  dst[11] = 0.0f;
  dst[12] = -(src[12] * src[0]) - (src[13] * src[1]) - (src[14] * src[2]);
  dst[13] = -(src[12] * src[4]) - (src[13] * src[5]) - (src[14] * src[6]);
  dst[14] = -(src[12] * src[8]) - (src[13] * src[9]) - (src[14] * src[10]);
  dst[15] = 1.0f;
}

GLuint createShaders(const char* vert, const char* frag) {
  GLuint v, f;

  if(!(v = nv::CompileGLSLShaderFromFile(GL_VERTEX_SHADER, vert))) {
    v = nv::CompileGLSLShaderFromFile(GL_VERTEX_SHADER, &vert[3]); //skip the first three chars to deal with path differences
  }

  if(!(f = nv::CompileGLSLShaderFromFile(GL_FRAGMENT_SHADER, frag))) {
    f = nv::CompileGLSLShaderFromFile(GL_FRAGMENT_SHADER, &frag[3]); //skip the first three chars to deal with path differences
  }

  return nv::LinkGLSLProgram(v, f);
}

void CheckFramebufferStatus() {
  int status;
  status = (GLenum) glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT);
  switch(status) {
    case GL_FRAMEBUFFER_COMPLETE_EXT:
      break;
    case GL_FRAMEBUFFER_UNSUPPORTED_EXT:
      printf("Unsupported framebuffer format\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT:
      printf("Framebuffer incomplete, missing attachment\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT:
      printf("Framebuffer incomplete, incomplete attachment\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT:
      printf("Framebuffer incomplete, attached images must have same dimensions\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_FORMATS_EXT:
      printf("Framebuffer incomplete, attached images must have same format\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT:
      printf("Framebuffer incomplete, missing draw buffer\n");
      break;
    case GL_FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT:
      printf("Framebuffer incomplete, missing read buffer\n");
      break;
    default:
      exit(0);
  }
}
