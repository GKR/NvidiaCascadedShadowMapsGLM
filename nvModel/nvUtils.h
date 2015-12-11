#ifndef VENUS_NVMODEL_NVUTILS_H
#define VENUS_NVMODEL_NVUTILS_H

namespace nv {
class Model;
void DrawTriangles(Model *model);
void DrawWithAdjacency(Model *model);
void DrawPoints(Model *model);
void DrawEdges(Model *model);
//void DrawNvModel(Model *model);
} // namespace nv

#endif // VENUS_NVMODEL_NVUTILS_H