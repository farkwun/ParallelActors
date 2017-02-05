//===================================
// include guard
#ifndef ACTOR_H
#define ACTOR_H

//===================================
// included dependencies
#include <string>
#include "Coordinate.h"

//===================================
// the class
class Actor {
  private: 
    Coordinate position;
    Coordinate destination;
    Coordinate next_move;
    std::string id;
  public:
    Coordinate get_position();
    Coordinate get_destination();
    Coordinate get_next_move();
    std::string get_id();
    void set_position(Coordinate new_position);
    void set_destination(Coordinate new_destination);
    void set_next_move(Coordinate new_next_move);
    void set_id(std::string new_id);
};

#endif /* ACTOR_H */
