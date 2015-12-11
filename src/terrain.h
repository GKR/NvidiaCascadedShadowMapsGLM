// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.

#pragma once

#include "main.h"
#include <vector>
#include <nvModel.h>

#define SCALE 0.2f;
#define MODEL_Y_TRANSLATE -0.1f
#define MODEL_HEIGHT 3.0f


const char TERRAIN_TEX_FILENAME[] = "../../media/textures/gcanyon.png";
const char DEPTH_TEX_FILENAME[] = "../../media/textures/gcanyond.png";
const char ENTITIES_TEX_FILENAME[] = "../../media/textures/entities.png";
const char MODEL_FILENAMET[] = "../../media/models/trunk.obj";
const char MODEL_FILENAMEL[] = "../../media/models/leaves.obj";

class Terrain
{
public:
	Terrain();
	~Terrain();
	bool	Load();
  void Draw(GLuint t_current_program, const glm::mat4& t_view);
	void	DrawCoarse();
	int		getDim(){ return (width>height)?width:height;	}
private:
	void	MakeTerrain();
	bool	LoadTree();
	void	DrawTree();

	GLuint	tex;
	float	*heights;
	float	*normals;
	std::vector<nv::vec3f *> entities;

	int		height;
	int		width;

	nv::Model	*modelT;
	nv::Model	*modelL;

	GLuint	vboIdT;
	GLuint	eboIdT;
	GLuint	vboIdL;
	GLuint	eboIdL;

	GLuint	terrain_list;
};
