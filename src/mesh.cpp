#include "mesh.hpp"
#include "viewer.hpp"
#include <algorithm>
#include <cstdint>
#include <iostream>
#include <sstream>
#include <fstream>
#include <unordered_map>

namespace V = COL781::Viewer;

void Mesh::init(glm::vec3 *vertices, int numVertices, glm::vec3* normals, int numNormals, glm::ivec3 *triangles, int numTriangles){
  freeArrays();

  this->halfEdges = std::vector<HalfEdge>(1 + numTriangles * 3);
  this->triangles = std::vector<Face>(1 + numTriangles);
  this->vertices = std::vector<Vertex>(1 + numVertices);
  std::unordered_map<std::pair<uint32_t, uint32_t>, uint32_t, hash_pair<int,int>> edgeMap;

  // Create the vertices
  for (int i = 0; i < numVertices; i++) {
    this->vertices[i + 1].position = vertices[i];
    if (i < numNormals){
      this->vertices[i + 1].normal = normals[i];
    }
    else{
      this->vertices[i + 1].normal = glm::vec3(0.0f, 0.0f, 0.0f);
    }
  }
  // Create the half edges and faces
  for (uint32_t i = 0; i < numTriangles; i++) {
    // TODO: Orientation of the triangles
    HalfEdge* edges = &this->halfEdges[i * 3 + 1];
    // Set the half edge properties
    for(uint32_t j=0; j<3; j++){
      uint32_t index = i * 3 + j + 1;
      edges[j].head = triangles[i][j] + 1;
      vertex_halfEdge(triangles[i][j] + 1) = index;
      // Check if the other half of this edge already exists
      uint32_t v0 = triangles[i][j] + 1;
      uint32_t v1 = triangles[i][(j + 1) % 3] + 1;
      uint32_t v0_m = std::min(v0, v1);
      uint32_t v1_m = std::max(v0, v1);
      std::pair<uint32_t, uint32_t> p = std::make_pair(v0_m, v1_m);
      if (edgeMap.find(p) != edgeMap.end()) {
        edges[j].pair = edgeMap[p];
        this->halfEdges[edgeMap[p]].pair = index;
      } else {
        edgeMap[p] = index;
      }
      edges[j].next = i * 3 + (j + 1) % 3 + 1;
      edges[j].prev = i * 3 + (j + 2) % 3 + 1;
      edges[j].left = i + 1;
    }
    // Set the face properties
    face_halfEdge(i + 1) = i * 3 + 1;
  }
}

Mesh::Mesh(glm::vec3 *vertices, int numVertices, glm::vec3* normals, int numNormals, glm::ivec3 *triangles, int numTriangles)
{
  init(vertices, numVertices, normals, numNormals, triangles, numTriangles);
}

void Mesh::view(){
  uint32_t numVertices = this->vertices.size();
  uint32_t numTriangles = this->triangles.size();
  glm::vec3* vertices = new glm::vec3[numVertices - 1];
  glm::vec3* normals = new glm::vec3[numVertices - 1];
  glm::ivec3* triangles = new glm::ivec3[numTriangles - 1];

  // Copy the vertices and normals
  for (size_t i = 1; i < numVertices; i++) {
    vertices[i - 1] = this->vertices[i].position;
    normals[i - 1] = this->vertices[i].normal;
  }
  // Copy the triangles
  for (size_t i = 1; i < numTriangles; i++) {
    uint32_t he = face_halfEdge(i);
    triangles[i - 1] = glm::ivec3(edge_head(he), edge_head(edge_next(he)), edge_head(edge_prev(he))) - 1;
  }

	V::Viewer v;
	if (!v.initialize("Mesh viewer", 640, 480)) {
		return;
	}
	v.setVertices(numVertices - 1, vertices);
	v.setNormals(numVertices - 1, normals);
	v.setTriangles(numTriangles - 1, triangles);
	v.view();
  // local arrays
  delete [] vertices;
  delete [] normals;
  delete [] triangles;
}

void Mesh::print(){
  // print the Mesh
  std::cout << "Mesh: " << std::endl;
  std::cout << "Vertices: " << std::endl;
  // print the vertices
  for (size_t i = 1; i < this->vertices.size(); i++) {
    Vertex& v = this->vertices[i];
    std::cout << "  " << v.position.x << " " << v.position.y << " " << v.position.z << std::endl;
  }
  std::cout << "Triangles: " << std::endl;
  for (size_t i = 1; i < this->triangles.size(); i++) {
    uint32_t he = face_halfEdge(i);
    std::cout << " " << edge_head(he) - 1;
    std::cout << " " << edge_head(edge_next(he)) - 1;
    std::cout << " " << edge_head(edge_prev(he)) - 1 << std::endl;
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
  for (size_t i = 1; i < this->vertices.size(); i++) {
    this->vertices[i].normal = glm::vec3(0.0f, 0.0f, 0.0f);
  }
  // weighted sum of face normals
  for (size_t i = 1; i < this->triangles.size(); i++) {
    uint32_t he = face_halfEdge(i);
    glm::vec3 e1 = this->vertices[edge_head(edge_next(he))].position - this->vertices[edge_head(he)].position;
    glm::vec3 e2 = this->vertices[edge_head(edge_prev(he))].position - this->vertices[edge_head(he)].position;
    glm::vec3 normal = glm::cross(e1, e2);
//    normal = glm::normalize(normal);
    float weight = 1 / glm::length(e1) / glm::length(e2);
    normal = normal * weight * weight;
    this->vertices[edge_head(he)].normal += normal;
    this->vertices[edge_head(edge_next(he))].normal += normal;
    this->vertices[edge_head(edge_prev(he))].normal += normal;
  }
  // normalize
  for (Vertex& v : this->vertices) {
    v.normal = glm::normalize(v.normal);
  }
}

void Mesh::smoothing(int iter, float lambda, float mu){
  int numVertices = this->vertices.size();
  glm::vec3* delta = new glm::vec3[numVertices];
  for(int i=0; i<iter; i++){
    for(int stage=0; stage<2; stage++){
      // for Taubin smoothing
      float lambda_applied = lambda;
      if(stage%2==1){
        lambda_applied = mu;
      }
      // reset delta
      std::fill_n(delta, numVertices, glm::vec3(0));
      // compute delta                                        
      for (size_t j = 1; j < numVertices; j++) {        
        uint32_t startEdge = vertex_halfEdge(j);
        uint32_t e = startEdge;
        int neighbors = 0;                                    
        // go anti-clockwise                                  
        do{                    
          e = edge_pair(edge_prev(e));
        }while(e!=0 && e != startEdge);                 
        // go clockwise                                       
        do{                                                   
          neighbors++;                        
          delta[j] += this->vertices[edge_head(edge_next(e))].position;
          e = edge_next(edge_pair(e));
        }while(e!=0 && e != startEdge);                 
        // average                                            
        delta[j] /= (float) neighbors;                        
        delta[j] -= this->vertices[j].position;               
      }                                                       
      // update vertices at the end of each iteration         
      for (size_t j = 1; j < numVertices; j++) {        
        this->vertices[j].position += lambda_applied * delta[j];      
      }                                                       
    }   
  }
  delete [] delta;
}

uint32_t& Mesh::vertex_halfEdge(uint32_t i){
  return this->vertices[i].halfEdge;
}

uint32_t& Mesh::edge_head(uint32_t i){
  return this->halfEdges[i].head;
}

uint32_t& Mesh::edge_next(uint32_t i){
  return this->halfEdges[i].next;
}

uint32_t& Mesh::edge_prev(uint32_t i){
  return this->halfEdges[i].prev;
}

uint32_t& Mesh::edge_pair(uint32_t i){
  return this->halfEdges[i].pair;
}

uint32_t& Mesh::face_halfEdge(uint32_t i){
  return this->triangles[i].halfEdge;
}

void Mesh::freeArrays(){
  this->vertices.clear();
  this->triangles.clear();
  this->halfEdges.clear();
}
