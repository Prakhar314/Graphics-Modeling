#pragma once
#include <cstdint>
#include <glm/glm.hpp>
#include <memory>
#include <vector>

struct HalfEdge;
struct Face
{
    uint32_t halfEdge = 0;
};
struct Vertex
{
    uint32_t halfEdge = 0;
    glm::vec3 normal;
    glm::vec3 position;
};
struct HalfEdge
{
    uint32_t next = 0;
    uint32_t prev = 0;
    uint32_t pair = 0;
    uint32_t head = 0;
    uint32_t left = 0;
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
    std::vector<Vertex> vertices;
    std::vector<Face> triangles;
    std::vector<HalfEdge> halfEdges;

  public:
    Mesh(glm::vec3 *vertices, int numVertices, glm::vec3* normals, int numNormals, glm::ivec3 *triangles, int numTriangles);
    Mesh(std::string filename);
    void init(glm::vec3 *vertices, int numVertices, glm::vec3* normals, int numNormals, glm::ivec3 *triangles, int numTriangles);
    void recompute_normals();
    void smoothing(int iter, float lambda, float mu=0.0f);
    void print();
    void view();
    void freeArrays();
    
    uint32_t& edge_next(uint32_t he);
    uint32_t& edge_prev(uint32_t he);
    uint32_t& edge_pair(uint32_t he);
    uint32_t& edge_head(uint32_t he);
    uint32_t& edge_left(uint32_t he);

    uint32_t& vertex_halfEdge(uint32_t v);

    uint32_t& face_halfEdge(uint32_t f);
    
    uint32_t push_vertex();
    uint32_t push_triangle();
    uint32_t push_halfEdge();

    void edge_flip(uint32_t he);
    void edge_split(uint32_t he);

    void loop_subdivision();
};
