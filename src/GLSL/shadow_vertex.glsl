//----------------------------------------------------------------------------------
// File:   shadow_vertex.glsl
// Author: Rouslan Dimitrov
// Email:  sdkfeedback@nvidia.com
// Copyright (c) NVIDIA Corporation. All rights reserved.
//----------------------------------------------------------------------------------

varying vec4 position;

uniform vec4 lightdir;
uniform vec4 lightcolor;

uniform mat3 normalMatrix;
uniform mat4 modelViewMatrix;
uniform mat4 projectionMatrix;

void main() {
  position = modelViewMatrix * gl_Vertex;
  gl_Position = projectionMatrix * position;
  vec3 normal = normalize(normalMatrix * gl_Normal);

  gl_FrontColor = gl_Color * lightcolor * vec4(max(dot(normal, lightdir.xyz), 0.0));

  gl_TexCoord[0] = gl_MultiTexCoord0;
}