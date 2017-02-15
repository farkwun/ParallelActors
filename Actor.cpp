#include "Actor.h"

Actor::Actor(std::string id, Coordinate position, Coordinate destination, struct sockaddr_in sock_addr){
  this->address     = sock_addr;
  this->position    = position;
  this->destination = destination;
  this->id          = id;
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

bool Actor::get_collided(){
  return collided;
}

bool Actor::get_arrived(){
  return arrived;
}

bool Actor::get_invalid_move(){
  return invalid_move;
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

void Actor::set_id(std::string new_id){
  id = new_id;
}
