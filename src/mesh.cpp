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
        {
          uint32_t temp = e;
          // anti-clockwise
          do{
            e = temp;
            temp = edge_pair(edge_prev(e));
            if(temp == startEdge){
              e = temp;
              break;
            }
          }while(temp!=0);
        }
        int neighbors = 0;                                    
        // finding all the neighbours, clockwise
        do{
            neighbors++;                        
            delta[j] += this->vertices[edge_head(edge_next(e))].position;
            e = edge_pair(e);
            if(e==0){
              break;
            }
            e = edge_next(e);
        }
        while(e!=startEdge);
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
  assert(i < this->vertices.size() && i > 0);
  return this->vertices[i].halfEdge;
}

uint32_t& Mesh::edge_head(uint32_t i){
  assert(i < this->halfEdges.size() && i > 0);
  return this->halfEdges[i].head;
}

uint32_t& Mesh::edge_next(uint32_t i){
  assert(i < this->halfEdges.size() && i > 0);
  return this->halfEdges[i].next;
}

uint32_t& Mesh::edge_prev(uint32_t i){
  assert(i < this->halfEdges.size() && i > 0);
  return this->halfEdges[i].prev;
}

uint32_t& Mesh::edge_pair(uint32_t i){
  assert(i < this->halfEdges.size() && i > 0);
  return this->halfEdges[i].pair;
}

uint32_t& Mesh::edge_left(uint32_t he){
  assert(he < this->halfEdges.size() && he > 0);
  return this->halfEdges[he].left;
}

uint32_t& Mesh::face_halfEdge(uint32_t i){
  assert(i < this->triangles.size() && i > 0);
  return this->triangles[i].halfEdge;
}

void Mesh::freeArrays(){
  this->vertices.clear();
  this->triangles.clear();
  this->halfEdges.clear();
}

uint32_t Mesh::push_vertex(){
  this->vertices.push_back(Vertex());
  return this->vertices.size() - 1;
}

uint32_t Mesh::push_triangle(){
  this->triangles.push_back(Face());
  return this->triangles.size() - 1;
}

uint32_t Mesh::push_halfEdge(){
  this->halfEdges.push_back(HalfEdge());
  return this->halfEdges.size() - 1;
}

void Mesh::edge_split(uint32_t he){
  if (edge_pair(he) == 0) {
    // boundary edge
    uint32_t f0 = edge_left(he);

    uint32_t v0 = edge_head(he);
    uint32_t v1 = edge_head(edge_next(he));
    uint32_t v2 = edge_head(edge_prev(he));
    
    uint32_t e0 = he;
    uint32_t e1 = edge_next(he);
    uint32_t e2 = edge_prev(he);
    
    //             v1
    //            --|
    //       e1 --  |
    //        --    | e3
    //      --   e5 |
    // v2 ----------| v3
    //      --   e4 |
    //        --    | e0
    //       e2 --  |
    //            --|
    //             v0
    
    // create new structures
    uint32_t v3 = push_vertex();
    this->vertices[v3].position = (this->vertices[v0].position + this->vertices[v1].position) / 2.0f;

    uint32_t e3 = push_halfEdge();
    uint32_t e4 = push_halfEdge();
    uint32_t e5 = push_halfEdge();

    std::cout << "e5: " << e5 << std::endl;

    uint32_t f1 = push_triangle();

    // update old structures
    edge_next(e0) = e4;
    edge_prev(e0) = e2;
    edge_head(e0) = v0;
    edge_left(e0) = f0;

    edge_next(e1) = e5;
    edge_prev(e1) = e3;
    edge_head(e1) = v1;
    edge_left(e1) = f1;

    edge_next(e2) = e0;
    edge_prev(e2) = e4;
    edge_head(e2) = v2;
    edge_left(e2) = f0;

    face_halfEdge(f1) = e3;
    
    // update new structures
    edge_next(e3) = e1;
    edge_prev(e3) = e5;
    edge_head(e3) = v3;
    edge_pair(e3) = 0;
    edge_left(e3) = f1;

    edge_next(e4) = e2;
    edge_prev(e4) = e0;
    edge_head(e4) = v3;
    edge_pair(e4) = e5;
    edge_left(e4) = f0;

    edge_next(e5) = e3;
    edge_prev(e5) = e1;
    edge_head(e5) = v2;
    edge_pair(e5) = e4;
    edge_left(e5) = f1;

    face_halfEdge(f0) = e0;

    vertex_halfEdge(v0) = e0;
    vertex_halfEdge(v1) = e1;
    vertex_halfEdge(v2) = e2;
    vertex_halfEdge(v3) = e3;
  }
  else{
    // interior edge
    uint32_t f0 = edge_left(he);
    uint32_t f1 = edge_left(edge_pair(he));

    uint32_t e0 = he;
    uint32_t e1 = edge_next(he);
    uint32_t e2 = edge_prev(he);
    uint32_t e3 = edge_pair(he);
    uint32_t e4 = edge_next(edge_pair(he));
    uint32_t e5 = edge_prev(edge_pair(he));

    uint32_t v0 = edge_head(he);
    uint32_t v1 = edge_head(e1);
    uint32_t v2 = edge_head(e2);
    uint32_t v3 = edge_head(e5);

    //                 v1
    //            --|      |--
    //       e1 --  |      |  -- e5
    //        --    | e6   | e3  -- 
    //      --   e8 |      |   e10  --
    // v2 ----------|  v4  |----------- v3
    //      --   e7 |      |   e11  --
    //        --    | e0   | e9  --
    //       e2 --  |      |  -- e4
    //            --|      |--
    //                 v0
    // create new structures
    uint32_t v4 = push_vertex();
    this->vertices[v4].position = (3.0f * this->vertices[v0].position + 3.0f * this->vertices[v1].position + this->vertices[v2].position + this->vertices[v3].position) / 8.0f;
    
    uint32_t e6 = push_halfEdge();
    uint32_t e7 = push_halfEdge();
    uint32_t e8 = push_halfEdge();
    uint32_t e9 = push_halfEdge();
    uint32_t e10 = push_halfEdge();
    uint32_t e11 = push_halfEdge();

    uint32_t f2 = push_triangle();
    uint32_t f3 = push_triangle();
    
    // update new structures
    edge_next(e6) = e1;
    edge_prev(e6) = e8;
    edge_head(e6) = v4;
    edge_pair(e6) = e3;
    edge_left(e6) = f2;

    edge_next(e7) = e2;
    edge_prev(e7) = e0;
    edge_head(e7) = v4;
    edge_pair(e7) = e8;
    edge_left(e7) = f0;

    edge_next(e8) = e6;
    edge_prev(e8) = e1;
    edge_head(e8) = v2;
    edge_pair(e8) = e7;
    edge_left(e8) = f2;

    edge_next(e9) = e4;
    edge_prev(e9) = e11;
    edge_head(e9) = v4;
    edge_pair(e9) = e0;
    edge_left(e9) = f3;

    edge_next(e10) = e5;
    edge_prev(e10) = e3;
    edge_head(e10) = v4;
    edge_pair(e10) = e11;
    edge_left(e10) = f1;

    edge_next(e11) = e9;
    edge_prev(e11) = e4;
    edge_head(e11) = v3;
    edge_pair(e11) = e10;
    edge_left(e11) = f3;

    face_halfEdge(f2) = e6;
    face_halfEdge(f3) = e9;
    vertex_halfEdge(v4) = e6;

    // update old structures
    edge_next(e0) = e7;
    edge_prev(e0) = e2;
    edge_head(e0) = v0;
    edge_pair(e0) = e9;
    edge_left(e0) = f0;

    edge_next(e1) = e8;
    edge_prev(e1) = e6;
    edge_head(e1) = v1;
    edge_left(e1) = f2;

    edge_next(e2) = e0;
    edge_prev(e2) = e7;
    edge_head(e2) = v2;
    edge_left(e2) = f0;

    edge_next(e3) = e10;
    edge_prev(e3) = e5;
    edge_head(e3) = v1;
    edge_pair(e3) = e6;
    edge_left(e3) = f1;

    edge_next(e4) = e11;
    edge_prev(e4) = e9;
    edge_head(e4) = v0;
    edge_left(e4) = f3;

    edge_next(e5) = e3;
    edge_prev(e5) = e10;
    edge_head(e5) = v3;
    edge_left(e5) = f1;

    face_halfEdge(f0) = e0;
    face_halfEdge(f1) = e3;
    vertex_halfEdge(v0) = e0;
    vertex_halfEdge(v1) = e1;
    vertex_halfEdge(v2) = e2;
    vertex_halfEdge(v3) = e5;
  }
}

void Mesh::edge_flip(uint32_t i){
  uint32_t e0 = i;
  uint32_t e1 = edge_next(e0);
  uint32_t e2 = edge_next(e1);
  uint32_t e3 = edge_pair(e0);
  uint32_t e4 = edge_next(e3);
  uint32_t e5 = edge_next(e4);
  
  uint32_t v0 = edge_head(e0);
  uint32_t v1 = edge_head(e1);
  uint32_t v2 = edge_head(e2);
  uint32_t v3 = edge_head(e5);

  uint32_t f0 = edge_left(e0);
  uint32_t f1 = edge_left(e3);

  // update new structures
  edge_next(e0) = e2;
  edge_prev(e0) = e4;
  edge_head(e0) = v3;
  edge_pair(e0) = e3;
  edge_left(e0) = f0;

  edge_next(e1) = e3;
  edge_prev(e1) = e5;
  edge_head(e1) = v1;
  edge_left(e1) = f1;

  edge_next(e2) = e4;
  edge_prev(e2) = e0;
  edge_head(e2) = v2;
  edge_left(e2) = f0;

  edge_next(e3) = e5;
  edge_prev(e3) = e1;
  edge_head(e3) = v2;
  edge_pair(e3) = e0;
  edge_left(e3) = f1;

  edge_next(e4) = e0;
  edge_prev(e4) = e2;
  edge_head(e4) = v0;
  edge_left(e4) = f0;

  edge_next(e5) = e1;
  edge_prev(e5) = e3;
  edge_head(e5) = v3;
  edge_left(e5) = f1;

  face_halfEdge(f0) = e0;
  face_halfEdge(f1) = e1;

  vertex_halfEdge(v0) = e4;
  vertex_halfEdge(v1) = e1;
  vertex_halfEdge(v2) = e2;
  vertex_halfEdge(v3) = e5;
}

void Mesh::loop_subdivision(){
    
  uint32_t initial_vertex_count = this->vertices.size();
  uint32_t initial_edge_count = this->halfEdges.size();
  uint32_t initial_face_count = this->triangles.size();

  // recording the new position of the vertices in the original mesh
  glm::vec3* new_vertex_pos = new glm::vec3[this->vertices.size()];

  for(uint32_t i=1; i<initial_vertex_count; i++){
    Vertex& v = this->vertices[i];
    glm::vec3 temp(0.0f);
    uint32_t he = v.halfEdge;
    {
      uint32_t temp = he;
      // anti-clockwise
      do{
        he = temp;
        temp = edge_pair(edge_prev(he));
        if(temp == v.halfEdge){
          he = temp;
          break;
        }
      }while(temp!=0);
    }
    int count = 0;
    // finding all the neighbours, clockwise
    do{
        temp += this->vertices[edge_head(edge_next(he))].position;
        count++;
        he = edge_pair(he);
        if(he==0){
          break;
        }
        he = edge_next(he);
    }
    while(he!=v.halfEdge);
    float u;
    if(count==3){
        u = 3.0f/16.0f;
    }
    else{
        u = 3.0f/(8.0f*count);
    }
    temp = ((1-count*u)*v.position + u*temp);
    new_vertex_pos[i] = temp; 
  }

  // split all the edges
  for(uint32_t i=1; i<initial_edge_count; i++){
    if (edge_pair(i) < i) {
      edge_split(i);
    }
  }
  
  // flip all the edges
  for(uint32_t i=initial_edge_count + 2; i<this->halfEdges.size(); i+=3){
    if(
        ((edge_head(edge_next(i)) < initial_vertex_count) && (edge_head(i) >= initial_vertex_count)) ||
        ((edge_head(edge_next(i)) >= initial_vertex_count) && (edge_head(i) < initial_vertex_count))
      ){
      edge_flip(i);
    }
  }

  // set the new position of the vertices
  for(uint32_t i=1; i<initial_vertex_count; i++){
    this->vertices[i].position = new_vertex_pos[i];
  }
  recompute_normals();
}
