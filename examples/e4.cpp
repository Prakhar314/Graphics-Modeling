
#include "../src/mesh.hpp"
#include <iostream>
/**
 * Smoothing example
*/
int main(){
  //Mesh mesh("meshes/bunny-1k.obj");
  //Mesh mesh("meshes/cube.obj");
  //Mesh mesh("meshes/teapot.obj");
  Mesh mesh("meshes/noisycube.obj");
  // Umbrella operator
  //mesh.smoothing(10, 0.33);
  // Taubin smoothing
  mesh.smoothing(200, 0.33, -0.34);
  mesh.recompute_normals();
  mesh.view();
  return 0;
}
