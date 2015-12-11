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
uniform vec2 texSize; // x - size, y - 1/size


varying vec4 position;

uniform sampler2DArrayShadow stex;
float shadowCoef()
{
	int index = 3;
	
	// find the appropriate depth map to look up in based on the depth of this fragment
	if(gl_FragCoord.z < farbounds.x)
		index = 0;
	else if(gl_FragCoord.z < farbounds.y)
		index = 1;
	else if(gl_FragCoord.z < farbounds.z)
		index = 2;
	
	// transform this fragment's position from view space to scaled light clip space
	// such that the xy coordinates are in [0;1]
	// note there is no need to divide by w for othogonal light sources
	vec4 shadow_coord = gl_TextureMatrix[index]*position;

	shadow_coord.w = shadow_coord.z;
	
	// tell glsl in which layer to do the look up
	shadow_coord.z = float(index);

	//Bilinear weighted 4-tap filter
	vec2 pos = mod( shadow_coord.xy * texSize.x, 1.0);
	vec2 offset = (0.5 - step( 0.5, pos)) * texSize.y;
	float ret = shadow2DArray( stex, shadow_coord + vec4( offset, 0, 0)).x * (pos.x) * (pos.y);
	ret += shadow2DArray( stex, shadow_coord + vec4( offset.x, -offset.y, 0, 0)).x * (pos.x) * (1-pos.y);
	ret += shadow2DArray( stex, shadow_coord + vec4( -offset.x, offset.y, 0, 0)).x * (1-pos.x) * (pos.y);
	ret += shadow2DArray( stex, shadow_coord + vec4( -offset.x, -offset.y, 0, 0)).x * (1-pos.x) * (1-pos.y);
	
	
	return ret;
}

void main()
{
    const float shadow_ambient = 0.9;
	vec4 color_tex = texture2D(tex, gl_TexCoord[0].st);
	float shadow_coef = shadowCoef();
	float fog = clamp(gl_Fog.scale*(gl_Fog.end + position.z), 0.0, 1.0);
	gl_FragColor = mix(gl_Fog.color, (shadow_ambient * shadow_coef * gl_Color * color_tex + (1.0 - shadow_ambient) * color_tex), fog);
}