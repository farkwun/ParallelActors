//===================================
// include guard
#ifndef MAP_H
#define MAP_H

//===================================
// included dependencies
#include <vector>
#include <string>
#include "Actor.h"
#include "Coordinate.h"

//===================================
// the class
class Map {
  private: 
    const static char kObstacle = 'O';
    const static char kEmpty = ' ';
    const static char kActor = 'A';
    const static char kCollision = 'C';
    const static int default_map_length = 100;
    const static int default_map_width = 100;
    const static int default_vision_radius = 10;
    const static int default_actor_half_width = 5;
    const static int default_actor_half_length = 5;
    std::vector< std::vector<char> > map;
    int map_width;
    int map_length;
    int actor_half_width;
    int actor_half_length;
    int vision_radius;
  public:
    Map();
    Map(std::vector< std::vector<char> > input_map);
    void InitializeActorDimensions();
    void InitializeActorDimensions(int in_actor_half_width, int in_actor_half_length);
    void InitializeVision();
    void set_vision_radius(int new_radius);
    void set_map(std::vector< std::vector<char> > new_map);
    void AddActor(Actor actor);
    void ClearActor(Actor actor);
    void MoveActor(Actor actor, Coordinate new_pos);
    std::string GetSurroundings(Coordinate coordinate);
    Coordinate RandomEmptyLocation();
};

#endif /* MAP_H */
