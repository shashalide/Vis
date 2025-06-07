#include<algorithm>

#include "MS.h"
#include "MS.inl"

Isoline::Isoline(const Image& image, uint8_t isovalue,
                 bool useAsymptoticDecider) {
  for (uint32_t y=0;y<image.height-1;y++) {
    for (uint32_t x=0;x<image.width-1;x++) {
        std::array<uint8_t, 4> cell{};
        uint16_t vertexflag{0};
        for (uint8_t i=0;i<4;i++) {
            const size_t idx=(x+size_t(vertexPosTable[i][0])+(y+size_t(vertexPosTable[i][1]))*image.width)*image.componentCount;
            cell[i]=image.data[idx];
            vertexflag+=uint16_t(cell[i]>=isovalue)<<i;
        }
        
        for (uint8_t i=0;i<4;i++) {
            if (!(edgeTable[vertexflag] & 1<<i)) continue;
            const uint8_t vidx0=edgeToVertexTable[i][0];
            const uint8_t vidx1=edgeToVertexTable[i][1];
            const float vertex0=float(cell[vidx0]);
            const float vertex1=float(cell[vidx1]);
            // isoline = (1-alpha)*vertex0 + alpha*vertex1
            float alpha = (vertex0-isovalue)/(vertex0-vertex1);
            const Vec2& vertexpos0=vertexPosTable[vidx0];
            const Vec2& vertexpos1=vertexPosTable[vidx1];
            Vec2 isopoint{vertexpos0+alpha*(vertexpos1-vertexpos0)};
            Vec2 normalizeisopoint{((Vec2{float(x),float(y)}+isopoint)/Vec2{float(image.width-1), float(image.height-1)})*2.0f-1.0f};
            vertices.push_back(normalizeisopoint);
        }
        
        if (useAsymptoticDecider && (vertexflag==5 || vertexflag==10)) {
            const float adecider=(cell[3]*cell[1]-cell[0]*cell[2])/(cell[1]+cell[3]-cell[2]-cell[0]);
            if((vertexflag==5 && adecider<isovalue) || (vertexflag==10 && adecider>isovalue)) {
                std::swap(vertices[vertices.size()-1], vertices[vertices.size()-3]);
            }
        }
    }
  }
}
