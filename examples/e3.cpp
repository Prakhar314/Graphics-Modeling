#include "../src/mesh.hpp"
#include <iostream>

int main(){
  Mesh mesh("meshes/bunny-1k.obj");
  //Mesh mesh("meshes/cube.obj");
  //Mesh mesh("meshes/teapot.obj");
  mesh.view();
  return 0;
}
