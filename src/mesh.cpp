#include "mesh.hpp"
#include "viewer.hpp"
#include <iostream>

namespace V = COL781::Viewer;
Mesh::Mesh(glm::vec3 *vertices, glm::vec3* normals, int numVertices, glm::ivec3 *triangles, int numTriangles)
{
  std::unordered_map<std::pair<int, int>, HalfEdge*, hash_pair<int,int>> edgeMap;
  this->halfEdges = new HalfEdge[numTriangles * 3];
  this->triangles = new Face[numTriangles];
  this->vertices = new Vertex[numVertices];
  this->numVertices = numVertices;
  this->numTriangles = numTriangles;

  // Create the vertices
  for (int i = 0; i < numVertices; i++) {
    this->vertices[i].position = vertices[i];
    this->vertices[i].normal = normals[i];
    this->vertices[i].index_mesh = i;
  }
  // Create the half edges and faces
  for (int i = 0; i < numTriangles; i++) {
    // TODO: Orientation of the triangles
    Face* f = &this->triangles[i];
    HalfEdge* edges = &this->halfEdges[i * 3];
    // Set the half edge properties
    for(int j=0; j<3; j++){
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
    f->halfEdge = &edges[0];
  }
}

void Mesh::view(){
  glm::vec3* vertices = new glm::vec3[this->numVertices];
  glm::vec3* normals = new glm::vec3[this->numVertices];
  glm::ivec3* triangles = new glm::ivec3[this->numTriangles];

  // Copy the vertices and normals
  for (size_t i = 0; i < this->numVertices; i++) {
    vertices[i] = this->vertices[i].position;
    normals[i] = this->vertices[i].normal;
  }
  // Copy the triangles
  for (size_t i = 0; i < this->numTriangles; i++) {
    triangles[i] = glm::ivec3(this->triangles[i].halfEdge->head->index_mesh,
                         this->triangles[i].halfEdge->next->head->index_mesh,
                         this->triangles[i].halfEdge->prev->head->index_mesh);
  }

	V::Viewer v;
	if (!v.initialize("Mesh viewer", 640, 480)) {
		return;
	}
	v.setVertices(this->numVertices, vertices);
	v.setNormals(this->numVertices, normals);
	v.setTriangles(this->numTriangles, triangles);
	v.view();
  delete[] vertices;
  delete[] normals;
  delete[] triangles;
}

void Mesh::print(){
  // print the Mesh
  std::cout << "Mesh: " << std::endl;
  std::cout << "Vertices: " << std::endl;
  for (size_t i = 0; i < this->numVertices; i++) {
    std::cout << "  " << this->vertices[i].position.x << " " << this->vertices[i].position.y << " " << this->vertices[i].position.z << std::endl;
  }
  std::cout << "Triangles: " << std::endl;
  for (size_t i = 0; i < this->numTriangles; i++) {
    std::cout << " " << this->triangles[i].halfEdge->head->index_mesh;
    std::cout << " " << this->triangles[i].halfEdge->next->head->index_mesh;
    std::cout << " " << this->triangles[i].halfEdge->prev->head->index_mesh << std::endl;
  }
}

Mesh::~Mesh(){
  delete[] this->halfEdges;
  delete[] this->triangles;
  delete[] this->vertices;
}
