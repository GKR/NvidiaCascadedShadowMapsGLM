// Email: sdkfeedback@nvidia.com
//
// Copyright (c) NVIDIA Corporation. All rights reserved.

#include <nvImage.h>
#include "terrain.h"

#ifndef max
#define max(a,b)            (((a) > (b)) ? (a) : (b))
#endif

#ifndef min
#define min(a,b)            (((a) < (b)) ? (a) : (b))
#endif

Terrain::Terrain()
{
	tex = 0;
	heights = NULL;
	normals = NULL;
}

Terrain::~Terrain()
{
	if(tex)
		glDeleteTextures(1, &tex);
	if(heights)
		delete [] heights;
	if(normals)
		delete [] normals;
	if(modelT)
		delete modelT;
	if(modelL)
		delete modelL;
	height = 0;
	width = 0;

	for(unsigned int i=0; i<entities.size(); i++)
		delete entities[i];
}

bool Terrain::Load()
{
	nv::Image iTex;
	nv::Image dTex;
	nv::Image eTex;

	printf("loading terrain...\n");

	if(!iTex.loadImageFromFile(&TERRAIN_TEX_FILENAME[0])) {
		if(!iTex.loadImageFromFile(&TERRAIN_TEX_FILENAME[3])) {
			return false;
    }
  }
	if(!dTex.loadImageFromFile(&DEPTH_TEX_FILENAME[0]))
		if(!dTex.loadImageFromFile(&DEPTH_TEX_FILENAME[3]))
			return false;
	if(!eTex.loadImageFromFile(&ENTITIES_TEX_FILENAME[0]))
		if(!eTex.loadImageFromFile(&ENTITIES_TEX_FILENAME[3]))
			return false;

    GET_GLERROR()

	glGenTextures(1, &tex);
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
  //glTexImage2D( GL_TEXTURE_2D, 0, iTex.getInternalFormat(), iTex.getWidth(), iTex.getHeight(), 0, iTex.getImageSize(), iTex.getLevel(0));
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, iTex.getWidth(), iTex.getHeight(), 0, GL_RGBA, GL_UNSIGNED_BYTE, iTex.getLevel(0));
	//glCompressedTexImage2DARB( GL_TEXTURE_2D, 0, iTex.getInternalFormat(), iTex.getWidth(), iTex.getHeight(), 0, iTex.getImageSize(), iTex.getLevel(0));

  GET_GLERROR()

	height = dTex.getHeight();
	width = dTex.getWidth();

	int size = height * width;

	heights = new float [size * sizeof(float)];

	GLubyte *src = (GLubyte*)dTex.getLevel(0);
	for(int i=0; i<size; i++)
		heights[i] = (float)src[3*i] * SCALE;


	normals = new float [3 * size * sizeof(float)];

	for(int z=1; z<height-1; z++) {
		for(int x=1; x<width-1; x++) {
			float dyx =  heights[x+1 + z*width]
						-heights[x-1 + z*width];
			float dyz =  heights[x + (z+1)*width]
						-heights[x + (z-1)*width];

			nv::vec3f vx(1,   dyx, 0.0f);
			nv::vec3f vz(0.0f, -dyz,   -1);
			nv::vec3f v = normalize(cross(vx, vz));
			normals[3*(x + z*width)    ] = v.x;
			normals[3*(x + z*width) + 1] = v.y;
			normals[3*(x + z*width) + 2] = v.z;
		}
	}

	int e_width = eTex.getWidth();
	int e_height = eTex.getHeight();
	float e_ratio_x = (float)e_width / (float)width;
	float e_ratio_z = (float)e_height / (float)height;

	GLubyte *ent = (GLubyte*)eTex.getLevel(0);

	for(int z=0; z<e_height; z++) {
		for(int x=0; x<e_width; x++) {
			if(ent[3*(x + z*e_width)] == 255) {
				nv::vec3f *v = new nv::vec3f;
				v->x = e_ratio_x * (float)x;
				v->y = heights[x + z*width];
				v->z = e_ratio_z * (float)z;
				entities.push_back(v);
			}
		}
	}

	modelT = new nv::Model;
	modelL = new nv::Model;
	if(!LoadTree())
	{
		printf("Couldn't find model .obj.\n");
		exit(0);
	}


	MakeTerrain();

	return true;
}

void Terrain::Draw(GLuint t_current_program, const glm::mat4& t_view) {
  float minCamZ = -10000.0f;

  float half_width = 0.5f*(float)width;
  float half_height = 0.5f*(float)height;

  glm::mat4 t_modelview1 = glm::translate(t_view, glm::vec3(-half_width, 0, -half_height));
  glm::mat3 t_normalmatrix1 = glm::inverseTranspose(glm::mat3(t_modelview1));

  int far_dist = (int)FAR_DIST - 1;

  int camx = (int)(cam_pos[0] + half_width);
  int camz = (int)(cam_pos[2] + half_height);

  int zmin = max(camz-far_dist, 1);
  int zmax = min(camz+far_dist, height - 1);

  int xmin = max(camx-far_dist, 1);
  int xmax = min(camx+far_dist, width - 1);

  for(unsigned int i=0; i<entities.size(); i++) {
    nv::vec3f *v = entities[i];
    float d = m_light_dir.x*(v->x-half_width) + m_light_dir.y*v->y + m_light_dir.z*(v->z-half_height);
    if(minCamZ < d) { //MODEL_HEIGHT
      glm::mat4 t_modelview2 = glm::translate(t_modelview1, glm::vec3(v->x, v->y, v->z));
      glm::mat3 t_normalmatrix2 = glm::inverseTranspose(glm::mat3(t_modelview2));

      glUniformMatrix4fv(glGetUniformLocation(t_current_program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(t_modelview2));
      glUniformMatrix3fv(glGetUniformLocation(t_current_program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(t_normalmatrix2));

      DrawTree();
    }
  }

  glUniformMatrix4fv(glGetUniformLocation(t_current_program, "modelViewMatrix"), 1, GL_FALSE, glm::value_ptr(t_modelview1));
  glUniformMatrix3fv(glGetUniformLocation(t_current_program, "normalMatrix"), 1, GL_FALSE, glm::value_ptr(t_normalmatrix1));

  glActiveTexture(GL_TEXTURE1);
  glCallList(terrain_list);

  glMatrixMode(GL_MODELVIEW);
  glActiveTexture(GL_TEXTURE0);
}

void Terrain::DrawCoarse()
{
	float half_width = 0.5f*(float)width;
	float half_height = 0.5f*(float)height;

	const float inv_height = 1.0f / (float)height;
	const float inv_width = 1.0f / (float)width;

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(-half_width, 0, -half_height);

	glBindTexture(GL_TEXTURE_2D, tex);

	for(int z=1; z<height-2; z+=8)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for(int x=1; x<width-1; x+=8)
		{
			float fx = (float)x;
			float fz = (float)z;
			glTexCoord2f( fx*inv_width, fz*inv_height );
			glNormal3fv(&normals[3*(x + z*width)]);
			glVertex3f( fx, heights[x + z*width], fz );
			glTexCoord2f( fx*inv_width, (fz+8.0f)*inv_height );
			glNormal3fv(&normals[3*(x + (z+8)*width)]);
			glVertex3f( fx, heights[x + (z+8)*width], fz+8.0f );
		}
		glEnd();
	}
	glPopMatrix();
}
void Terrain::MakeTerrain()
{
	printf("building terrain...\n");
	float half_width = 0.5f*(float)width;
	float half_height = 0.5f*(float)height;

	const float inv_height = 1.0f / (float)height;
	const float inv_width = 1.0f / (float)width;

	terrain_list = glGenLists(1);
	glNewList(terrain_list, GL_COMPILE);

	glMatrixMode(GL_MODELVIEW);
	glPushMatrix();
	glTranslatef(-half_width, 0, -half_height);

	glBindTexture(GL_TEXTURE_2D, tex);

	for(int z=1; z<height-2; z++)
	{
		glBegin(GL_TRIANGLE_STRIP);
		for(int x=1; x<width-1; x++)
		{
			float fx = (float)x;
			float fz = (float)z;
			glTexCoord2f( fx*inv_width, fz*inv_height );
			glNormal3fv(&normals[3*(x + z*width)]);
			glVertex3f( fx, heights[x + z*width], fz );
			glTexCoord2f( fx*inv_width, (fz+1.0f)*inv_height );
			glNormal3fv(&normals[3*(x + (z+1)*width)]);
			glVertex3f( fx, heights[x + (z+1)*width], fz+1.0f );

		}
		glEnd();
	}
	glPopMatrix();

	glEndList();
}

bool Terrain::LoadTree()
{
	printf("loading OBJ trunk...\n");
	if (!modelT->loadModelFromFile(&MODEL_FILENAMET[0])) {
		if (!modelT->loadModelFromFile(&MODEL_FILENAMET[3]))
			return false;
	}
	modelT->compileModel();

	int totalVertexSize = modelT->getCompiledVertexCount() * modelT->getCompiledVertexSize() * sizeof(GLfloat);
	int totalIndexSize = modelT->getCompiledIndexCount() * sizeof(GLuint);

	glGenBuffers(1, &vboIdT);
	glBindBuffer(GL_ARRAY_BUFFER, vboIdT);
    glBufferData(GL_ARRAY_BUFFER, totalVertexSize, modelT->getCompiledVertices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &eboIdT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIdT);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndexSize, modelT->getCompiledIndices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

	printf("loading OBJ leaves...\n");
	if (!modelL->loadModelFromFile(&MODEL_FILENAMEL[0])) {
		if (!modelL->loadModelFromFile(&MODEL_FILENAMEL[3]))
			return false;
	}
	modelL->compileModel();

	totalVertexSize = modelL->getCompiledVertexCount() * modelL->getCompiledVertexSize() * sizeof(GLfloat);
	totalIndexSize = modelL->getCompiledIndexCount() * sizeof(GLuint);

	glGenBuffers(1, &vboIdL);
	glBindBuffer(GL_ARRAY_BUFFER, vboIdL);
    glBufferData(GL_ARRAY_BUFFER, totalVertexSize, modelL->getCompiledVertices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ARRAY_BUFFER, 0);

	glGenBuffers(1, &eboIdL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIdL);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, totalIndexSize, modelL->getCompiledIndices(), GL_STATIC_DRAW);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	return true;
}


void Terrain::DrawTree()
{
	glBindBuffer(GL_ARRAY_BUFFER, vboIdT);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIdT);
	int stride = modelT->getCompiledVertexSize() * sizeof(GLfloat);
	int normalOffset = modelT->getCompiledNormalOffset() * sizeof(GLfloat);
	glVertexPointer(modelT->getPositionSize(), GL_FLOAT, stride, NULL);
	glNormalPointer(GL_FLOAT, stride, (GLubyte *)NULL + normalOffset);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glColor3f(0.917647f, 0.776471f, 0.576471f);
	glDrawElements(GL_TRIANGLES, modelT->getCompiledIndexCount(), GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ARRAY_BUFFER, vboIdL);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, eboIdL);
	stride = modelL->getCompiledVertexSize() * sizeof(GLfloat);
	normalOffset = modelL->getCompiledNormalOffset() * sizeof(GLfloat);
	glVertexPointer(modelL->getPositionSize(), GL_FLOAT, stride, NULL);
	glNormalPointer(GL_FLOAT, stride, (GLubyte *)NULL + normalOffset);
	glEnableClientState(GL_VERTEX_ARRAY);
	glEnableClientState(GL_NORMAL_ARRAY);

	glColor3f(0.301961f, 0.588235f, 0.309804f);
	glDrawElements(GL_TRIANGLES, modelL->getCompiledIndexCount(), GL_UNSIGNED_INT, NULL);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	glBindBuffer(GL_ARRAY_BUFFER, 0);
	glDisableClientState(GL_VERTEX_ARRAY);
	glDisableClientState(GL_NORMAL_ARRAY);
	glColor3f(1.0f, 1.0f, 1.0f);
}
