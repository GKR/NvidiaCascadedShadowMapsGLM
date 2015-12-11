//----------------------------------------------------------------------------------
// File:   shadow_single_hl_fragment.glsl
// Author: Rouslan Dimitrov
// Email:  sdkfeedback@nvidia.com
// Cascaded shadows maps, highlight layers using different colors
// Copyright (c) NVIDIA Corporation. All rights reserved.
//----------------------------------------------------------------------------------
#version 120
#extension GL_EXT_texture_array : enable

uniform sampler2D tex;
uniform vec4 farbounds;

varying vec4 position;

// List of shadow coord lookup matrices for each shadow map segment
uniform mat4 textureMatrixList[4];

// Shadow split colors
uniform vec4 color[4] = vec4[4](
  vec4(0.7, 0.7, 1.0, 1.0),
  vec4(0.7, 1.0, 0.7, 1.0),
  vec4(1.0, 0.7, 0.7, 1.0),
  vec4(1.0, 1.0, 1.0, 1.0));

uniform sampler2DArray shadowmap;

vec4 shadowCoef() {
  int index = 3;

  if(gl_FragCoord.z < farbounds.x) {
    index = 0;
  } else if(gl_FragCoord.z < farbounds.y) {
    index = 1;
  } else if(gl_FragCoord.z < farbounds.z) {
    index = 2;
  }

  vec4 shadow_coord = textureMatrixList[index] * position;

  shadow_coord.w = shadow_coord.z;
  shadow_coord.z = float(index);

  float shadow_d = texture2DArray(shadowmap, shadow_coord.xyz).x;
  float diff = shadow_d - shadow_coord.w;
  return clamp( diff*250.0 + 1.0, 0.0, 1.0) * color[index];
}

void main() {
  const float shadow_ambient = 0.9;
  vec4 color_tex = vec4(1.0); //texture2D(tex, gl_TexCoord[0].st);
  vec4 shadow_coef = shadowCoef();
  float fog = clamp(gl_Fog.scale*(gl_Fog.end + position.z), 0.0, 1.0);
  gl_FragColor = mix(gl_Fog.color, (shadow_ambient * shadow_coef * gl_Color * color_tex + (1.0 - shadow_ambient) * color_tex), fog);
}
