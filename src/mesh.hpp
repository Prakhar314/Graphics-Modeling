#pragma once
#include <glm/glm.hpp>
#include <memory>
#include <vector>
#include <unordered_map>

struct HalfEdge;
struct Face
{
    HalfEdge* halfEdge;
};
struct Vertex
{
    HalfEdge* halfEdge;
    int index_mesh;
    glm::vec3 normal;
    glm::vec3 position;
};
struct HalfEdge
{
    HalfEdge* next;
    HalfEdge* prev;
    HalfEdge* pair;
    Vertex* head;
    Face* left;
};

template <class T1, class T2>
struct hash_pair {
    size_t operator()(const std::pair<T1, T2>& p) const
    {
        size_t hash1 = std::hash<T1>()(p.first);
        size_t hash2 = std::hash<T2>()(p.second);
 
        if (hash1 != hash2) {
            return hash1 ^ hash2;             
        }
         
        return hash1;
    }
};

// Define a mesh data structure to store the connectivity and geometry of a triangle mesh
class Mesh
{
  private:
    /* data */
    Face* triangles;
    Vertex* vertices;
    HalfEdge* halfEdges;
    size_t numVertices;
    size_t numTriangles;

  public:
    Mesh(glm::vec3 *vertices, glm::vec3* normals, int numVertices, glm::ivec3 *triangles, int numTriangles); 
    void print();
    void view();
    ~Mesh();
};