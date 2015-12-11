//----------------------------------------------------------------------------------
// File:   shadow_single_fragment.glsl
// Author: Rouslan Dimitrov
// Email:  sdkfeedback@nvidia.com
// Cascaded shadows maps, single shadow sample
// Copyright (c) NVIDIA Corporation. All rights reserved.
//----------------------------------------------------------------------------------
#version 120
#extension GL_EXT_texture_array : enable

uniform sampler2D tex;
uniform vec4 farbounds;

varying vec4 position;

//uniform sampler2DArray shadowmap;
uniform sampler2DArrayShadow shadowmap;

// List of shadow coord lookup matrices for each shadow map segment
uniform mat4 textureMatrixList[4];

const int nsamples = 4;
uniform vec2 poissonDisk[nsamples];

float getOccCoef(vec4 shadow_coord) {
  // get the stored depth, let the hardware do the comparison for us
  float shadow_d = shadow2DArray(shadowmap, shadow_coord).x;

  // get the difference of the stored depth and the distance of this fragment to the light
  float diff = shadow_d - shadow_coord.w;

  // smoothen the result a bit, to avoid aliasing at shadow contact point
  //return clamp(diff * 250.0 + 1.0, 0.0, 1.0);
  return clamp(diff * 30.0 + 1.0, 0.0, 1.0);
}

float shadowCoef() {
  int index = 3;

  // find the appropriate depth map to look up in based on the depth of this fragment
  if(gl_FragCoord.z < farbounds.x) {
    index = 0;
  } else if(gl_FragCoord.z < farbounds.y) {
    index = 1;
  } else if(gl_FragCoord.z < farbounds.z) {
    index = 2;
  }

  // transform this fragment's position from view space to scaled light clip space
  // such that the xy coordinates are in [0;1]
  // note there is no need to divide by w for othogonal light sources
  vec4 shadow_coord = textureMatrixList[index] * position;

  shadow_coord.w = shadow_coord.z;

  // tell glsl in which layer to do the look up
  shadow_coord.z = float(index);

  float shadow_d = getOccCoef(shadow_coord);
  return shadow_d;
}

void main() {
  const float shadow_ambient = 0.9;
  vec4 color_tex = texture2D(tex, gl_TexCoord[0].st);
  float shadow_coef = shadowCoef();
  float fog = clamp(gl_Fog.scale*(gl_Fog.end + position.z), 0.0, 1.0);
  gl_FragColor = mix(gl_Fog.color, (shadow_ambient * shadow_coef * gl_Color * color_tex + (1.0 - shadow_ambient) * color_tex), fog);
}
