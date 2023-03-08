#include "mesh.hpp"
#include "viewer.hpp"
#include <iostream>
#include <sstream>
#include <fstream>

namespace V = COL781::Viewer;

void Mesh::init(glm::vec3 *vertices, int numVertices, glm::vec3* normals, int numNormals, glm::ivec3 *triangles, int numTriangles){
  freeArrays();

  this->halfEdges = new HalfEdge[numTriangles * 3];
  this->triangles = new Face[numTriangles];
  this->vertices = new Vertex[numVertices];
  std::unordered_map<std::pair<int, int>, HalfEdge*, hash_pair<int,int>> edgeMap;
  this->numVertices = numVertices;
  this->numTriangles = numTriangles;

  // Create the vertices
  for (int i = 0; i < numVertices; i++) {
    this->vertices[i].position = vertices[i];
    if (i < numNormals){
      this->vertices[i].normal = normals[i];
    }
    else{
      this->vertices[i].normal = glm::vec3(0.0f, 0.0f, 0.0f);
    }
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

Mesh::Mesh(glm::vec3 *vertices, int numVertices, glm::vec3* normals, int numNormals, glm::ivec3 *triangles, int numTriangles)
{
  init(vertices, numVertices, normals, numNormals, triangles, numTriangles);
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

Mesh::Mesh(std::string filename){
  std::ifstream f(filename);
  std::string line;
  std::vector<glm::vec3> vertices, normals;
  std::vector<glm::ivec3> triangles;
  while(getline(f,line)){
    std::istringstream s(line);
    std::string head;
    s >> head;
    if(head == "vn"){
      glm::vec3 normal;
      s >> normal.x >> normal.y >> normal.z;
      normals.push_back(normal);
    }
    if(head == "v"){
      glm::vec3 vertex;
      s >> vertex.x >> vertex.y >> vertex.z;
      vertices.push_back(vertex);
    }
    if(head == "f"){
      std::string temp;
      glm::ivec3 triangle;
      for(int i=0; i<3; i++){
          s >> temp;
          //TODO: is this correct?
          std::string last_val(temp.substr(0,temp.find("/"))); 
          triangle[i] = stoi(last_val);
      }
      triangles.push_back(triangle - 1);
    }
  }
  init(&vertices[0], vertices.size(), &normals[0], normals.size(), &triangles[0], triangles.size());
}

void Mesh::recompute_normals(){
  // reset normals
  for (size_t i = 0; i < this->numVertices; i++) {
    this->vertices[i].normal = glm::vec3(0.0f, 0.0f, 0.0f);
  }
  // weighted sum of face normals
  for (size_t i = 0; i < this->numTriangles; i++) {
    glm::vec3 e1 = this->triangles[i].halfEdge->next->head->position - this->triangles[i].halfEdge->head->position;
    glm::vec3 e2 = this->triangles[i].halfEdge->prev->head->position - this->triangles[i].halfEdge->head->position;
    glm::vec3 normal = glm::cross(e1, e2);
//    normal = glm::normalize(normal);
    float weight = 1 / glm::length(e1) / glm::length(e2);
    normal = normal * weight * weight;
    this->triangles[i].halfEdge->head->normal += normal;
    this->triangles[i].halfEdge->next->head->normal += normal;
    this->triangles[i].halfEdge->prev->head->normal += normal;
  }
  // normalize
  for (size_t i = 0; i < this->numVertices; i++) {
    this->vertices[i].normal = glm::normalize(this->vertices[i].normal);
  }
}

void Mesh::freeArrays(){
  if(this->halfEdges != nullptr){
    delete[] this->halfEdges;
  }
  if(this->triangles != nullptr){
    delete[] this->triangles;
  }
  if(this->vertices != nullptr){
    delete[] this->vertices;
  }
  this->vertices = nullptr;
  this->triangles = nullptr;
  this->halfEdges = nullptr;
}

Mesh::~Mesh(){
  freeArrays();
}
