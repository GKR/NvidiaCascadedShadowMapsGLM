//----------------------------------------------------------------------------------
// File:   shadowmapping.cpp
// Author: Rouslan Dimitrov
// Email:  sdkfeedback@nvidia.com
//
// Copyright (c) 2007 NVIDIA Corporation. All rights reserved.
//
// TO  THE MAXIMUM  EXTENT PERMITTED  BY APPLICABLE  LAW, THIS SOFTWARE  IS PROVIDED
// *AS IS*  AND NVIDIA AND  ITS SUPPLIERS DISCLAIM  ALL WARRANTIES,  EITHER  EXPRESS
// OR IMPLIED, INCLUDING, BUT NOT LIMITED  TO, IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE.  IN NO EVENT SHALL  NVIDIA OR ITS SUPPLIERS
// BE  LIABLE  FOR  ANY  SPECIAL,  INCIDENTAL,  INDIRECT,  OR  CONSEQUENTIAL DAMAGES
// WHATSOEVER (INCLUDING, WITHOUT LIMITATION,  DAMAGES FOR LOSS OF BUSINESS PROFITS,
// BUSINESS INTERRUPTION, LOSS OF BUSINESS INFORMATION, OR ANY OTHER PECUNIARY LOSS)
// ARISING OUT OF THE  USE OF OR INABILITY  TO USE THIS SOFTWARE, EVEN IF NVIDIA HAS
// BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
//
//----------------------------------------------------------------------------------

// This sample shows an implementation of cascaded shadow maps.
// The important pieces of code are fairly commented, so you can jump straight to display();
// The main idea of cascaded shadow maps is to split the camera view frustum into several
// smaller frusta and then calculate a separate shadow map for each. This has the advantage
// that objects near the camera get more resolution of the shadow map and ones that are far
// away get less, in an attempt to provide an uniform error in screen space.

// This sample uses the following artwork:
// Palmtree model
// http://telias.free.fr/zipsz/models_3ds/plants/palm1.zip
//
// Terrain textures
// http://www.cc.gatech.edu/projects/large_models/gcanyon.html

#include "main.h"
#include "terrain.h"

#include <iostream>
#include <string>

using std::string;
using std::cout;
using std::endl;

//using namespace nv;

struct obj_BoundingSphere {
  vec3 center;
  float radius;
};

//int cur_num_splits = 2;
int show_depth_tex = 1;
int shadow_type = 0;
//const int m_num_matrices = 4;

Terrain *terrain;

bool m_uniform_offsets = false;
bool m_uniform_poisson = false;

int width = 1152;
int height = 720;
//int depth_size = 1024;

//GLuint depth_fb;//, depth_rb;
//GLuint depth_tex_ar;

GLuint write_depth_prog;
GLuint view_prog;
GLuint shad_single_prog;

//frustum f[MAX_SPLITS];
//float shad_cpm[MAX_SPLITS][16];
//glm::mat4 t_mat_shad_cpm[MAX_SPLITS];

obj_BoundingSphere obj_BSphere[NUM_OBJECTS];

//float split_weight = 0.75f;

void makeScene() {
  terrain = new Terrain;
  if(!terrain->Load()) {
    printf("Couldn't find terrain textures.\n");
    exit(0);
  }

  int td = terrain->getDim()/2;
  obj_BSphere[0].center = glm::vec3(-td, 50.0f, -td);
  obj_BSphere[1].center = glm::vec3(-td, 50.0f,  td);
  obj_BSphere[2].center = glm::vec3( td, 50.0f,  td);
  obj_BSphere[3].center = glm::vec3( td, 50.0f, -td);
  obj_BSphere[0].radius = 1.0f;
  obj_BSphere[1].radius = 1.0f;
  obj_BSphere[2].radius = 1.0f;
  obj_BSphere[3].radius = 1.0f;
}

/** here all shadow map textures and their corresponding matrices are created */
void render_shadow_map() {

  GKR::Camera* camera = get_camera();
  GKR::ShadowMap* shadow_map = get_shadow_map();

  vec4 t_lightdir = m_light_dir;

  glDisable(GL_TEXTURE_2D);
  // since the shadow maps have only a depth channel, we don't need color computation
  // glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);

  glUseProgram(write_depth_prog);

  // redirect rendering to the depth texture
  glBindFramebuffer(GL_FRAMEBUFFER, shadow_map->fbo());

  // store the screen viewport
  glPushAttrib(GL_VIEWPORT_BIT);

  // and render only to the shadowmap
  glViewport(0, 0, shadow_map->depth_tex_size(), shadow_map->depth_tex_size());

  // offset the geometry slightly to prevent z-fighting
  // note that this introduces some light-leakage artifacts
  glPolygonOffset(1.0f, 4096.0f);
  glEnable(GL_POLYGON_OFFSET_FILL);

  // draw all faces since our terrain is not closed.
  glDisable(GL_CULL_FACE);

  // Generate crop and projection matrices
  shadow_map->pre_depth_write(camera, t_lightdir);

  mat4 t_modelview = shadow_map->modelview_matrix();
  glUniformMatrix4fv(glGetUniformLocation(write_depth_prog, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(t_modelview));

  // Write depth to shadow map segments (draw geometry)
  for(int i = 0 ; i < shadow_map->num_splits() ; i++) {
    mat4 t_projection = shadow_map->projection_matrix(i);

    glUniformMatrix4fv(glGetUniformLocation(write_depth_prog, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(t_projection));
    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadow_map->texture(), 0, i);

    // clear the depth texture from last time
    glClear(GL_DEPTH_BUFFER_BIT);

    // draw the scene
    terrain->Draw(write_depth_prog, t_modelview);
  }

  // revert to normal back face culling as used for rendering
  glEnable(GL_CULL_FACE);

  glDisable(GL_POLYGON_OFFSET_FILL);
  glPopAttrib();
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  glEnable(GL_TEXTURE_2D);

  glUseProgram(0);
}

/** */
void render_scene() {
  GKR::ShadowMap* shadow_map = get_shadow_map();
  GKR::Camera* camera = get_camera();

  vec4 t_lightdir = camera->view_matrix() * m_light_dir;

  // approximate the atmosphere's filtering effect as a linear function
  vec4 t_skycolor(0.8f, t_lightdir.y * 0.1f + 0.7f, t_lightdir.y * 0.4f + 0.5f, 1.0f);

  glClearColor(t_skycolor.x, t_skycolor.y, t_skycolor.z, t_skycolor.w);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // update the camera, so that the user can have a free look
  mat4 t_view = camera->view_matrix();
  //mat4 t_view_inverse = glm::inverse(t_view);
  mat4 t_projection = camera->projection_matrix();

  // Update far bounds and texture matrices
  //shadow_map->pre_render(t_projection, t_view_inverse);

  // Bind all depth maps
  glBindTexture(GL_TEXTURE_2D_ARRAY, shadow_map->texture());
  /*if(shadow_type >= 4) {
    glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_COMPARE_R_TO_TEXTURE);
  } else {
    glTexParameteri(GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_NONE);
  }*/

  // == Setup shader
  GLuint t_current_program = shad_single_prog;
  glUseProgram(shad_single_prog);
  glUniform1i(glGetUniformLocation(shad_single_prog, "shadowmap"), 0); // depth-maps
  glUniform1i(glGetUniformLocation(shad_single_prog, "tex"), 1); // terrain tex
  // the shader needs to know the split distances, so that it can choose in which
  // texture to to the look up. Note that we pass them in homogeneous coordinates -
  // this the same space as gl_FragCoord is in. In this way the shader is more efficient
  glUniform4fv(glGetUniformLocation(shad_single_prog, "farbounds"), 1, shadow_map->far_bounds());
  glUniform4fv(glGetUniformLocation(shad_single_prog, "lightdir"), 1, glm::value_ptr(t_lightdir));
  glUniform4fv(glGetUniformLocation(shad_single_prog, "lightcolor"), 1, glm::value_ptr(t_skycolor));

  if(m_uniform_offsets) {
    const int nsamples = 8;
    const vec4 offset[nsamples] = {
      vec4(0.000000, 0.000000, 0.0, 0.0),
      vec4(0.079821, 0.165750, 0.0, 0.0),
      vec4(-0.331500, 0.159642, 0.0, 0.0),
      vec4(-0.239463, -0.497250, 0.0, 0.0),
      vec4(0.662999, -0.319284, 0.0, 0.0),
      vec4(0.399104, 0.828749, 0.0, 0.0),
      vec4(-0.994499, 0.478925, 0.0, 0.0),
      vec4(-0.558746, -1.160249, 0.0, 0.0)
    };

    glUniform4fv(glGetUniformLocation(shad_single_prog, "offset"), nsamples, glm::value_ptr(offset[0]));
  }

  // http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/#PCF
  if(m_uniform_poisson) {
    const int nsamples = 4;
    const vec2 poissonDisk[nsamples] = {
      vec2(-0.94201624, -0.39906216),
      vec2(0.94558609, -0.76890725),
      vec2(-0.094184101, -0.92938870),
      vec2(0.34495938, 0.29387760)
    };
    glUniform4fv(glGetUniformLocation(shad_single_prog, "poissonDisk"), nsamples, glm::value_ptr(poissonDisk[0]));
  }

  glUniformMatrix4fv(glGetUniformLocation(shad_single_prog, "projectionMatrix"), 1, GL_FALSE, glm::value_ptr(t_projection));
  //glUniformMatrix4fv(glGetUniformLocation(shad_single_prog, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(t_view));
  glUniformMatrix4fv(glGetUniformLocation(shad_single_prog, "textureMatrixList"), shadow_map->num_splits(), GL_FALSE, shadow_map->texture_matrices());
  // == Setup shader

  //glLightfv(GL_LIGHT0, GL_POSITION, light_dir);
  //glLightfv(GL_LIGHT0, GL_DIFFUSE, sky_color);

  glFogfv(GL_FOG_COLOR, glm::value_ptr(t_skycolor));

  // finally, draw the scene
  terrain->Draw(t_current_program, t_view);

  glUseProgram(0);

  GET_GLERROR()
}

// here we render the terrain from top and show the camera frusta.
// this display can be enabled from within the sample
void overviewCam() {
  /*glActiveTexture(GL_TEXTURE0);
  glMatrixMode(GL_TEXTURE);
  glLoadIdentity();

  glMatrixMode(GL_MODELVIEW);
  glLoadIdentity();
  glDisable(GL_LIGHTING);
  glPointSize(10);
  glColor3f(1.0f, 1.0f, 0.0f);
  gluLookAt(0, FAR_DIST/2, 0, 0, 0, 0, 0, 0, 1.0f);

  glScalef(0.2f, 0.2f, 0.2f);
  glRotatef(20, 1, 0, 0);
  for(int i = 0 ; i < cur_num_splits ; i++) {
    glBegin(GL_LINE_LOOP);
    for(int j = 0 ; j < 4 ; j++) {
      glVertex3f(f[i].point[j].x, f[i].point[j].y, f[i].point[j].z);
    }
    glEnd();

    glBegin(GL_LINE_LOOP);
    for(int j = 4 ; j < 8 ; j++) {
      glVertex3f(f[i].point[j].x, f[i].point[j].y, f[i].point[j].z);
    }
    glEnd();
  }

  for(int j = 0 ; j < 4 ; j++) {
    glBegin(GL_LINE_STRIP);
    glVertex3fv(cam_pos);
    for(int i = 0 ; i < cur_num_splits ; i++) {
      glVertex3f(f[i].point[j].x, f[i].point[j].y, f[i].point[j].z);
    }
    glVertex3f(f[cur_num_splits-1].point[j+4].x, f[cur_num_splits-1].point[j+4].y, f[cur_num_splits-1].point[j+4].z);
    glEnd();
  }

  glLightfv(GL_LIGHT0, GL_POSITION, glm::value_ptr(m_light_dir));
  glColor3f(0.9f, 0.9f, 1.0f);
  glEnable(GL_LIGHTING);
  glEnable(GL_LIGHT0);
  terrain->DrawCoarse();*/
}

// here we show all depth maps that have been generated for the current frame
// note that a special shader is required to display the depth-component-only textures
void showDepthTex() {
  GKR::ShadowMap* shadow_map = get_shadow_map();

  int loc;
  glPushAttrib(GL_VIEWPORT_BIT | GL_DEPTH_BUFFER_BIT);
  glDisable(GL_DEPTH_TEST);
  glDisable(GL_CULL_FACE);
  glUseProgram(view_prog);
  glUniform1i(glGetUniformLocation(view_prog,"tex"), 0);
  loc = glGetUniformLocation(view_prog,"layer");

  for(int i = 0 ; i < shadow_map->num_splits() ; i++) {
    glViewport(130*i, 0, 128, 128);
    glBindTexture(GL_TEXTURE_2D_ARRAY_EXT, shadow_map->texture());
    glTexParameteri( GL_TEXTURE_2D_ARRAY_EXT, GL_TEXTURE_COMPARE_MODE, GL_NONE);
    glUniform1f(loc, (float)i);

    glBegin(GL_QUADS);
    glVertex3f(-1.0f, -1.0f, 0.0f);
    glVertex3f( 1.0f, -1.0f, 0.0f);
    glVertex3f( 1.0f,  1.0f, 0.0f);
    glVertex3f(-1.0f,  1.0f, 0.0f);
    glEnd();
  }
  glUseProgram(0);

  glViewport(width - 129, 0, 128, 128);
  glEnable(GL_DEPTH_TEST);
  glClear(GL_DEPTH_BUFFER_BIT);
  overviewCam();

  glEnable(GL_CULL_FACE);
  glPopAttrib();
}

// this is the main display function that glut calls for us. There are 2 main steps in the algorithm:
void display() {
  updateKeys();

  GKR::Camera* camera = get_camera();
  camera->update(0.1);

  // 1. Render the shadow map
  render_shadow_map();

  // 2. Render the world by applying the shadow maps
  render_scene();

  // additionally, we can display information to aid the understanding
  // of what is going on
  //if(show_depth_tex) {
    showDepthTex();
    //}

  glutSwapBuffers();
}

void init() {
  glClearColor(0.8f, 0.8f , 0.9f, 1.0f);
  glEnable(GL_CULL_FACE);
  glEnable(GL_DEPTH_TEST);

  makeScene();

  string t_vertex_shader("../../src/GLSL/shadow_vertex.glsl");
  //string t_fragment_shader("../../src/GLSL/shadow_single_fragment.glsl");
  //string t_fragment_shader("../../src/GLSL/shadow_pcf.glsl"); m_uniform_poisson = true;

  //string t_fragment_shader("../../src/GLSL/shadow_single_hl_fragment.glsl");
  string t_fragment_shader("../../src/GLSL/shadow_multi_leak_fragment.glsl"); m_uniform_offsets = true;
  //string t_fragment_shader("../../src/GLSL/shadow_pcf_fragment.glsl");
  //string t_fragment_shader("../../src/GLSL/shadow_pcf_gaussian_fragment.glsl");
  //string t_fragment_shader("../../src/GLSL/shadow_pcf_trilinear_fragment.glsl");

  string t_depth_vertex_shader("../../src/GLSL/write_depth_vertex.glsl");
  string t_depth_fragment_shader("../../src/GLSL/write_depth_fragment.glsl");

  string t_debugview_vertex_shader("../../src/GLSL/view_vertex.glsl");
  string t_debugview_fragment_shader("../../src/GLSL/view_fragment.glsl");

  shad_single_prog = createShaders(t_vertex_shader.c_str(), t_fragment_shader.c_str());
  view_prog = createShaders(t_debugview_vertex_shader.c_str(), t_debugview_fragment_shader.c_str());
  write_depth_prog = createShaders(t_depth_vertex_shader.c_str(), t_depth_fragment_shader.c_str());

  /*for(int i = 0 ; i < MAX_SPLITS ; i++) {
    // note that fov is in radians here and in OpenGL it is in degrees.
    // the 0.2f factor is important because we might get artifacts at
    // the screen borders.
    f[i].fov = CAMERA_FOV / 57.2957795 + 0.2f;
    f[i].ratio = (double)width/(double)height;
  }*/

  glFogf(GL_FOG_DENSITY, 0.4f);
  glFogf(GL_FOG_START, 16.0f);
  glFogf(GL_FOG_END, FAR_DIST);

  GET_GLERROR()
}

/** */
int main(int argc, char** argv) {
  glutInit(&argc, argv);
  glutInitDisplayString("double rgb~8 depth~24 samples=4");
  glutInitWindowSize(width, height);
  glutCreateWindow("Cascaded Shadow Maps");

  glewInit();
  if(!glewIsSupported( "GL_VERSION_2_0 ")) {
    printf( "Required extensions not supported.\n");
    return 1;
  }

  glutIgnoreKeyRepeat(true);

  glutDisplayFunc(display);
  //glutKeyboardFunc(keys);
  glutReshapeFunc(reshape);
  glutMouseFunc(mouse);
  glutMotionFunc(motion);
  glutIdleFunc(idle);

  glutKeyboardFunc(key_down);
  glutKeyboardUpFunc(key_up);

  glutCreateMenu(menu);
  glutAddMenuEntry("SSM (1 split) [1]", '1');
  glutAddMenuEntry("CSM (2 split) [2]", '2');
  glutAddMenuEntry("CSM (3 split) [3]", '3');
  glutAddMenuEntry("CSM (4 split) [4]", '4');
  glutAddMenuEntry("Show Shadow Maps [`]", '`');
  glutAddMenuEntry("------------", -1);
  glutAddMenuEntry("Normal Mode", 0);
  glutAddMenuEntry("Show Splits", 1);
  glutAddMenuEntry("Smooth shadows", 2);
  glutAddMenuEntry("Smooth shadows, no leak", 3);
  glutAddMenuEntry("PCF", 4);
  glutAddMenuEntry("PCF w/ trilinear", 5);
  glutAddMenuEntry("PCF w/ 4 taps", 6);
  glutAddMenuEntry("PCF w/ 8 random taps", 7);
  glutAddMenuEntry("PCF w/ gaussian blur", 8);
  glutAddMenuEntry("------------", -1);
  glutAddMenuEntry("512x512", 10);
  glutAddMenuEntry("1024x1024", 11);
  glutAddMenuEntry("2048x2048", 12);
  glutAddMenuEntry("4096x4096", 13);
  glutAddMenuEntry("------------", -1);
  glutAddMenuEntry("Exit [q]", 'q');
  glutAttachMenu(GLUT_RIGHT_BUTTON);

  init();

  printf("\nKeys:\n");
  printf("W, A, S, D        - move around\n");
  printf("Left Mouse Button - free look\n");
  printf("Shift + LMB       - move light\n");
  printf("1, 2, 3, 4        - number of splits\n");
  printf("~                 - show depth textures\n");

  glutMainLoop();

  return 0;
}
