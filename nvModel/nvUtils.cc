#include "nvUtils.h"
#include "nvModel.h"

namespace nv {
inline void SetVertexNormal(Model *model) {
  glVertexPointer(model->getPositionSize(), GL_FLOAT,  
                  model->getCompiledVertexSize() * sizeof(float), 
                  model->getCompiledVertices());
  glNormalPointer(GL_FLOAT, model->getCompiledVertexSize() * sizeof(float), 
                  model->getCompiledVertices() + 
                      model->getCompiledNormalOffset());

  glEnableClientState( GL_VERTEX_ARRAY);
  glEnableClientState( GL_NORMAL_ARRAY);
}

inline void SetTexCoord(Model *model) {
  if ( model->hasTexCoords()) {
    glClientActiveTexture(GL_TEXTURE0);
    glTexCoordPointer(model->getTexCoordSize(), GL_FLOAT,  
      model->getCompiledVertexSize() * sizeof(float), 
      model->getCompiledVertices() + 
      model->getCompiledTexCoordOffset());
    glEnableClientState( GL_TEXTURE_COORD_ARRAY);
  }
}
inline void SetTangent(Model *model) {
  if ( model->hasTangents()) {
    glClientActiveTexture(GL_TEXTURE1);
    glTexCoordPointer(model->getTangentSize(), GL_FLOAT,  
      model->getCompiledVertexSize() * sizeof(float),  
      model->getCompiledVertices() +  
      model->getCompiledTangentOffset());
    glEnableClientState( GL_TEXTURE_COORD_ARRAY);
  }
}

inline void DisableVertexNormalTexCoord() {
  glDisableClientState( GL_VERTEX_ARRAY);
  glDisableClientState( GL_NORMAL_ARRAY);
  glDisableClientState( GL_TEXTURE_COORD_ARRAY);
}

inline void DisableVertexNormalTexCoordTangent() {
  glDisableClientState( GL_VERTEX_ARRAY);
  glDisableClientState( GL_NORMAL_ARRAY);
  glClientActiveTexture(GL_TEXTURE0);
  glDisableClientState( GL_TEXTURE_COORD_ARRAY);
  glClientActiveTexture(GL_TEXTURE1);
  glDisableClientState( GL_TEXTURE_COORD_ARRAY);
}

void DrawWithAdjacency(Model *model) {
  SetVertexNormal(model);
  SetTexCoord(model);
  SetTangent(model);
  nv::Model::PrimType prim_type = nv::Model::eptTrianglesWithAdjacency;
  glDrawElements(GL_TRIANGLES_ADJACENCY_EXT, 
                 model->getCompiledIndexCount(prim_type),
                 GL_UNSIGNED_INT,   
                 model->getCompiledIndices(prim_type));
  DisableVertexNormalTexCoordTangent();
}
void DrawTriangles(Model *model) {
  SetVertexNormal(model);
  SetTexCoord(model);
  SetTangent(model);
  glDrawElements(GL_TRIANGLES, 
                 model->getCompiledIndexCount(nv::Model::eptTriangles), 
                 GL_UNSIGNED_INT,
                 model->getCompiledIndices( nv::Model::eptTriangles));
  DisableVertexNormalTexCoordTangent();
}
void DrawEdges(Model *model) {
  SetVertexNormal(model);
  SetTexCoord(model);
  glDrawElements(GL_LINES, 
                 model->getCompiledIndexCount(nv::Model::eptEdges), 
                 GL_UNSIGNED_INT, 
                 model->getCompiledIndices(nv::Model::eptEdges));
  DisableVertexNormalTexCoord();
}
void DrawPoints(Model *model) {
  SetVertexNormal(model);
  SetTexCoord(model);
  glDrawElements(GL_POINTS, 
                 model->getCompiledIndexCount(nv::Model::eptPoints), 
                 GL_UNSIGNED_INT, 
                 model->getCompiledIndices(nv::Model::eptPoints));
  DisableVertexNormalTexCoord();
}
} // namespace nv
