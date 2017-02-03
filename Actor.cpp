#include "Actor.h"

Coordinate Actor::get_position(){
  return position;
}

Coordinate Actor::get_destination(){
  return destination;
}

std::string Actor::get_id(){
  return id;
}

void Actor::set_position(Coordinate new_position){
  position = new_position;
}

void Actor::set_destination(Coordinate new_destination){
  destination = new_destination;
}

void Actor::set_id(std::string new_id){
  id = new_id;
}
