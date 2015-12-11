//----------------------------------------------------------------------------------
// File:   shadow_multi_noleak_fragment.glsl
// Author: Rouslan Dimitrov
// Email:  sdkfeedback@nvidia.com
// Cascaded shadows maps, multiple shadow samples with leak suppression
// Copyright (c) NVIDIA Corporation. All rights reserved.
//----------------------------------------------------------------------------------
#version 120
#extension GL_EXT_texture_array : enable

uniform sampler2DArray stex;
uniform sampler2D tex;

uniform vec4 farbounds;

varying vec4 position;
varying vec3 normal;

// sample offsets
const int nsamples = 8;
uniform vec4 offset[nsamples] = { vec4(0.000000, 0.000000, 0.0, 0.0),
								  vec4(0.079821, 0.165750, 0.0, 0.0),
								  vec4(-0.331500, 0.159642, 0.0, 0.0),
								  vec4(-0.239463, -0.497250, 0.0, 0.0),
								  vec4(0.662999, -0.319284, 0.0, 0.0),
								  vec4(0.399104, 0.828749, 0.0, 0.0),
								  vec4(-0.994499, 0.478925, 0.0, 0.0),
								  vec4(-0.558746, -1.160249, 0.0, 0.0) };

float getOccCoef(vec4 shadow_coord)
{
	// get the stored depth
	float shadow_d = texture2DArray(stex, shadow_coord.xyz).x;
	
	// get the difference of the stored depth and the distance of this fragment to the light
	float diff = shadow_d - shadow_coord.w;
	
	// smoothen the result a bit, so that we don't get hard shadows
	return clamp( diff*250.0 + 1.0, 0.0, 1.0);
}


float shadowCoef()
{
	const float scale = 2.0/4096.0;
	int index = 3;
	
	// find the appropriate depth map to look up in based on the depth of this fragment
	if(gl_FragCoord.z < farbounds.x)
		index = 0;
	else if(gl_FragCoord.z < farbounds.y)
		index = 1;
	else if(gl_FragCoord.z < farbounds.z)
		index = 2;
	
	// transform this fragment's position from world space to scaled light clip space
	// such that the xy coordinates are in [0;1]
	vec4 shadow_coord = gl_TextureMatrix[index]*position;
	
	vec4 light_normal4 = gl_TextureMatrix[index+4]*vec4(normal, 0.0);
	vec3 light_normal = normalize(light_normal4.xyz);
	
	float d = -dot(light_normal, shadow_coord.xyz);
	
	shadow_coord.w = shadow_coord.z;
	
	// tell glsl in which layer to do the look up
	shadow_coord.z = float(index);

    // sum shadow samples	
	float shadow_coef = getOccCoef(shadow_coord);

	for(int i=1; i<nsamples; i++)
	{
		vec4 shadow_lookup = shadow_coord + scale*offset[i];
		float lookup_z = -(light_normal.x*shadow_lookup.x + light_normal.y*shadow_lookup.y + d)/light_normal.z;
		shadow_lookup.w = lookup_z;
		shadow_coef += getOccCoef(shadow_lookup);
	}
	shadow_coef /= nsamples;
	
	return shadow_coef;
}

void main()
{
    const float shadow_ambient = 0.9;
	vec4 color_tex = texture2D(tex, gl_TexCoord[0].st);
	float shadow_coef = shadowCoef();
	float fog = clamp(gl_Fog.scale*(gl_Fog.end + position.z), 0.0, 1.0);
	gl_FragColor = mix(gl_Fog.color, (shadow_ambient * shadow_coef * gl_Color * color_tex + (1.0 - shadow_ambient) * color_tex), fog);
}
