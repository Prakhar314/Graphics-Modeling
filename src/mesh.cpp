#include "mesh.hpp"
#include <iostream>

Mesh::Mesh(glm::vec4 *vertices, int numVertices, glm::ivec3 *triangles, int numTriangles)
{
  std::unordered_map<std::pair<int, int>, HalfEdge*, hash_pair<int,int>> edgeMap;
  // Create the vertices
  for (size_t i = 0; i < numVertices; i++) {
    Vertex v;
    v.position = glm::vec3(vertices[i]);
    v.index_mesh = i;
    this->vertices.push_back(v);
  }
  // Create the half edges and faces
  for (size_t i = 0; i < numTriangles; i++) {
    // TODO: Orientation of the triangles
    Face f;
    for (size_t j = 0; j < 3; j++) {
      // Create 3 new half edges
      HalfEdge e;
      this->halfEdges.push_back(e);
    }
    HalfEdge* edges = &this->halfEdges[i * 3];
    // Set the half edge properties
    for(size_t j=0; j<3; j++){
      edges[j].head = &this->vertices[triangles[i][j]];
      // Check if the other half of this edge already exists
      int v0 = triangles[i][j];
      int v1 = triangles[i][(j + 1) % 3];
      int v0_m = std::min(v0, v1);
      int v1_m = std::max(v0, v1);
      std::pair<int, int> p = std::make_pair(v0_m, v1_m);
      if (edgeMap.find(p) != edgeMap.end()) {
        edges[j].pair = edgeMap[p];
        edgeMap[p]->pair = &edges[j];
      } else {
        edgeMap[p] = &edges[j];
      }
      edges[j].next = &edges[(j + 1) % 3];
      edges[j].prev = &edges[(j + 2) % 3];
      edges[j].left = &this->triangles[i];
    }
    // Set the face properties
    f.halfEdge = &edges[0];
    this->triangles.push_back(f);
  }
}

void Mesh::print(){
  // print the Mesh
  std::cout << "Mesh: " << std::endl;
  std::cout << "Vertices: " << std::endl;
  for (size_t i = 0; i < this->vertices.size(); i++) {
    std::cout << "  " << this->vertices[i].position.x << " " << this->vertices[i].position.y << " " << this->vertices[i].position.z << std::endl;
  }
  std::cout << "Triangles: " << std::endl;
  for (size_t i = 0; i < this->triangles.size(); i++) {
    std::cout << "  " << this->triangles[i].halfEdge->head->index_mesh << " " << this->triangles[i].halfEdge->next->head->index_mesh << " " << this->triangles[i].halfEdge->prev->head->index_mesh << std::endl;
  }
}

int main(){
  // declare a mesh
  // using the constructor
  Mesh m{
    new glm::vec4[3]{glm::vec4(0,0,0,1), glm::vec4(1,0,0,1), glm::vec4(0,1,0,1)},
    3,
    new glm::ivec3[1]{glm::ivec3(0,1,2)},
    1
  };
  m.print();
  return 0;
}
