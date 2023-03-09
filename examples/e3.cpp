#include "../src/mesh.hpp"
#include <iostream>

/**
 * Mesh reading example
*/
int main(){
  //Mesh mesh("meshes/bunny-1k.obj");
  //Mesh mesh("meshes/cube.obj");
  Mesh mesh("meshes/teapot.obj");
  //Mesh mesh("meshes/noisycube.obj");
  mesh.recompute_normals();
  mesh.view();
  return 0;
}
