#include "../src/mesh.hpp"
#include <iostream>
#include <glm/gtc/matrix_transform.hpp>

/**
 * Sphere example
 */
int main(){
  Mesh mesh("meshes/bunny-1k.obj");
  mesh.view();
  mesh.loop_subdivision();
  mesh.view();
  return 0;
}

