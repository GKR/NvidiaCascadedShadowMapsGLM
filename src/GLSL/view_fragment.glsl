//----------------------------------------------------------------------------------
// File:   view_fragment.glsl
// Author: Rouslan Dimitrov
// Email:  sdkfeedback@nvidia.com
// Copyright (c) NVIDIA Corporation. All rights reserved.
//----------------------------------------------------------------------------------
#version 120
#extension GL_EXT_texture_array : enable

uniform sampler2DArray tex;
uniform float layer;

void main() {
  vec4 tex_coord = vec4(gl_TexCoord[0].x, gl_TexCoord[0].y, layer, 1.0);
  gl_FragColor = texture2DArray(tex, tex_coord.xyz);
}