#include "../src/mesh.hpp"
#include <iostream>

int main(){
  int m = 10;
  int n = 10;
  int numVertices = (m+1)*(n+1);
  int numTriangles = 2*m*n;
  glm::vec3 *vertices = new glm::vec3[numVertices];
  glm::vec3 *normals = new glm::vec3[numVertices];
  glm::ivec3 *triangles = new glm::ivec3[numTriangles];
  // Create the vertices
  for (size_t i = 0; i < m+1; i++) {
    for (size_t j = 0; j < n+1; j++) {
      vertices[i*(n+1)+j] = glm::vec3(1.0f * i / m, 1.0f * j / n, 0);
      normals[i*(n+1)+j] = glm::vec3(0,0,1);
    }
  }
  // Create the triangles
  for (size_t i = 0; i < m; i++) {
    for (size_t j = 0; j < n; j++) {
      triangles[2*(i*n+j)] = glm::ivec3(i*(n+1)+j, i*(n+1)+j+1, (i+1)*(n+1)+j);
      triangles[2*(i*n+j)+1] = glm::ivec3(i*(n+1)+j+1, (i+1)*(n+1)+j+1, (i+1)*(n+1)+j);
    }
  }
  Mesh mesh(vertices, normals, numVertices, triangles, numTriangles);
  mesh.view();
  return 0;
}
