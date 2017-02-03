//===================================
// include guard
#ifndef MAP_H
#define MAP_H

//===================================
// included dependencies
#include <vector>
#include <string>
#include "Coordinate.h"

//===================================
// the class
class Map {
  private: 
    const char kObstacle;
    const char kEmpty;
    const char kActor;
    const char kCOllision;
    std::vector< std::vector<char> > map;
    int length;
    int width;
    int vision_radius;
  public:
    void set_map(std::vector< std::vector<char> > new_map);
    void AddActor(Coordinate coordinate);
    void ClearActor(Coordinate coordinate);
    void MoveActor(Coordinate orig_pos, Coordinate new_pos);
    std::string GetSurroundings(Coordinate coordinate);
    Coordinate RandomEmptyLocation();
};

#endif /* MAP_H */
