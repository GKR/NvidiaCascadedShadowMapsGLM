//----------------------------------------------------------------------------------
// File:   shadow_vertex.glsl
// Author: Rouslan Dimitrov
// Email:  sdkfeedback@nvidia.com
// Copyright (c) NVIDIA Corporation. All rights reserved.
//----------------------------------------------------------------------------------

uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main() {
  vec4 position = projectionMatrix * modelViewMatrix * gl_Vertex;
  gl_Position = position;
}
