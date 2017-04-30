//===================================
// include guard
#ifndef ACTOR_H
#define ACTOR_H

//===================================
// included dependencies
#include <string>
#include "Coordinate.h"
#include <netinet/in.h>

//===================================
// the class
class Actor {
  private: 
    Coordinate position;
    Coordinate destination;
    Coordinate next_move;
    bool arrived      = false;
    bool invalid_move = false;
    bool timeout      = false;
    bool collided     = false;
    std::string id;
    int row_offset;
  public:
    Actor();
    Actor(std::string id, Coordinate position, Coordinate destination, int row_offset);
    Coordinate get_position();
    Coordinate get_destination();
    Coordinate get_next_move();
    Coordinate get_true_position();
    Coordinate get_relative_destination();
    bool get_collided();
    bool get_arrived();
    bool get_invalid_move();
    bool get_timeout();
    std::string get_id();
    void set_position(Coordinate new_position);
    void set_destination(Coordinate new_destination);
    void set_next_move(Coordinate new_next_move);
    void set_collided(bool collision);
    void set_arrived(bool arrival);
    void set_invalid_move(bool invalid);
    void set_timeout(bool timed_out);
    void set_id(std::string new_id);
    void print();
};

#endif /* ACTOR_H */
