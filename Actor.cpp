#include "Actor.h"
#include <iostream>

Actor::Actor(){
  Coordinate coordinate;

  position    = coordinate;
  destination = coordinate;
  id          = "";
  row_offset  = 0;
}

Actor::Actor(std::string id, Coordinate position, Coordinate destination, int row_offset){
  this->position    = position;
  this->destination = destination;
  this->id          = id;
  this->row_offset  = row_offset;
}


Coordinate Actor::get_position(){
  return position;
}

Coordinate Actor::get_destination(){
  return destination;
}

Coordinate Actor::get_next_move(){
  return next_move;
}

Coordinate Actor::get_true_position(){
  Coordinate true_position;
  true_position.set_row(position.get_row() + row_offset);
  true_position.set_col(position.get_col());
  return true_position;
}

Coordinate Actor::get_relative_destination(){
  Coordinate relative_destination;
  relative_destination.set_row(destination.get_row() - row_offset);
  relative_destination.set_col(destination.get_col());
  return relative_destination;
}

bool Actor::get_collided(){
  return collided;
}

bool Actor::get_arrived(){
  return arrived;
}

bool Actor::get_invalid_move(){
  return invalid_move;
}

bool Actor::get_timeout(){
  return timeout;
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

void Actor::set_next_move(Coordinate new_next_move){
  next_move = new_next_move;
}

void Actor::set_collided(bool collision){
  collided = collision;
}

void Actor::set_arrived(bool arrival){
  arrived = arrival;
}

void Actor::set_invalid_move(bool invalid){
  invalid_move = invalid;
}

void Actor::set_timeout(bool timed_out){
  timeout = timed_out;
}

void Actor::set_id(std::string new_id){
  id = new_id;
}

void Actor::print(){
  std::cout << std::boolalpha;
  std::cout << "Actor information for ID: " << id << std::endl;
  std::cout << "Position: " << position.to_str() << std::endl;
  std::cout << "Destination: " << destination.to_str() << std::endl;
  std::cout << "Next Move: " << next_move.to_str() << std::endl;
  std::cout << "Actor Arrived - " << arrived << std::endl;
  std::cout << "Actor Collided - " << collided << std::endl;
  std::cout << "Next Move Invalid - " << invalid_move << std::endl;
  std::cout << std::endl;
}
