#include "MC.h"
#include "MC.inl"
#include <algorithm>

Isosurface::Isosurface(const Volume& volume, uint8_t isovalue) {
  for (size_t z=0;z<volume.depth-1;z++) {
    for (size_t y=0;y<volume.height-1;y++) {
      for (size_t x=0;x<volume.width-1;x++) {
        std::array<uint8_t, 8> cell{};
        std::array<Vec3, 8> vertexNormal{};
        uint8_t vertexFlag{0};
        for (uint8_t i=0;i<8;i++) {
            const size_t u=x+size_t(vertexPosTable[i][0]);
            const size_t v=y+size_t(vertexPosTable[i][1]);
            const size_t w=z+size_t(vertexPosTable[i][2]);
            const size_t idx=u+v*volume.width+w*volume.width*volume.height;
            cell[i]=volume.data[idx];
            vertexNormal[i]=volume.normals[idx];
            vertexFlag+=uint8_t(cell[i]>isovalue)<<i;
        }
        const Vec3 offset{
          (float(x)-float(volume.width)/2)/float(volume.maxSize),
          (float(y)-float(volume.height)/2)/float(volume.maxSize),
          (float(z)-float(volume.depth)/2)/float(volume.maxSize)
        };

        std::array<Vertex, 12> vertexInEdge;
        for (uint8_t i=0;i<12;i++) {
            if (!(edgeTable[vertexFlag] & 1<<i)) continue;
            const uint8_t vIdx0=edgeToVertexTable[i][0];
            const uint8_t vIdx1=edgeToVertexTable[i][1];
            const float voxelVal0=float(cell[vIdx0]);
            const float voxelVal1=float(cell[vIdx1]);
            float alpha = std::clamp((float(isovalue)-voxelVal0)/(voxelVal1-voxelVal0),0.0f,1.0f);
            const Vec3& vertexpos0=vertexPosTable[vIdx0];
            const Vec3& vertexpos1=vertexPosTable[vIdx1];
            const Vec3 position{vertexpos0+alpha*(vertexpos1-vertexpos0)};
            const Vec3 normalizeisopoint=(offset+position/float(volume.maxSize))*volume.scale;
            const Vec3 normal(vertexNormal[vIdx0]+alpha*(vertexNormal[vIdx1]-vertexNormal[vIdx0]));
            const Vec3 normalizeNormal=Vec3::normalize(normal);
            vertexInEdge[i]=Vertex{normalizeisopoint, normalizeNormal};
        }
        if (trisTable[vertexFlag].size()) continue;
        for (uint8_t i : trisTable[vertexFlag]) {
          vertices.push_back(vertexInEdge[i]);
        }
      }
    }
  }
}
