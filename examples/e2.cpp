#include "../src/mesh.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

/**
 * Sphere example
 */
int main(int argc, char* argv[]){
  int m = atoi(argv[1]);
  int n = atoi(argv[2]);
  int numVertices = m*(n-1) + 2;
  int numTriangles = 2*m*(n-1);
  glm::vec3 *vertices = new glm::vec3[numVertices];
  glm::vec3 *normals = new glm::vec3[numVertices];
  glm::ivec3 *triangles = new glm::ivec3[numTriangles];
  
  glm::mat4 translation = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.0f, -0.0f));

  float pi = 3.1415926535;
  float long_angle = 2 * pi / m;
  float lat_angle = pi / n;
  float lat_off = - pi / 2 + lat_angle;
  // Create the vertices over a sphere
  for (size_t i = 0; i < n - 1; i++) {
    for (size_t j = 0; j < m; j++) {
      float x = cos(long_angle * j) * cos(lat_angle * i + lat_off);
      float y = sin(long_angle * j) * cos(lat_angle * i + lat_off);
      float z = sin(lat_angle * i + lat_off);
      vertices[i*m+j] = translation * glm::vec4(x / 2, y / 2, z / 2, 1);
      normals[i*m+j] = translation * glm::vec4(x, y, z, 0);
    }
  }
  vertices[m*(n-1)] = translation * glm::vec4(0, 0, 0.5, 1);
  normals[m*(n-1)] = translation * glm::vec4(0, 0, 0.5, 0);
  vertices[m*(n-1)+1] = translation * glm::vec4(0, 0, -0.5, 1);
  normals[m*(n-1)+1] = translation * glm::vec4(0, 0, -0.5, 0);
  // Create the triangles
  for (size_t i = 0; i < n - 2; i++) {
    for (size_t j = 0; j < m; j++) {
      triangles[2*(i*m+j)] = glm::ivec3(i*m+j, i*m+(j+1)%m, (i+1)*m+j);
      triangles[2*(i*m+j)+1] = glm::ivec3(i*m+(j+1)%m, (i+1)*m+(j+1)%m, (i+1)*m+j);
    }
  }
  for (size_t j = 0; j < m; j++) {
    triangles[2*(m*(n-2)+j)] = glm::ivec3(m*(n-2)+j, m*(n-2)+(j+1)%m, m*(n-1));
    triangles[2*(m*(n-2)+j)+1] = glm::ivec3(j, m*(n-1) + 1, (j+1)%m);
  }
  Mesh mesh(vertices, numVertices, normals, numVertices, triangles, numTriangles);
  mesh.view();
  return 0;
}
