#include <iostream>
#include "visual.h"
#include "visualizer.h"

int main(int argc, char** argv) {

//  std::cout << medium.evaluate(10, Point3D(2.0, 0.0, 0.0)) << std::endl;

  Visual viz;

  Visualizer v(viz);

  v.mainloop();
}
