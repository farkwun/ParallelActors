#include "Map.h"

Map::Map(void){
  std::vector<char> default_row(kEmpty, default_map_width);
  std::vector< std::vector<char> > default_map(default_map_length, default_row);
  map_width = default_map_width;
  map_length = default_map_length;
}

Map::Map(std::vector< std::vector<char> > input_map){
}

void Map::InitializeActorDimensions(){
  actor_half_width = default_actor_half_width;
  actor_half_length = default_actor_half_length;
}

void Map::InitializeActorDimensions(int in_actor_half_width, int in_actor_half_length){
  actor_half_width = in_actor_half_width;
  actor_half_length = in_actor_half_length;
}

void Map::InitializeVision(){
  vision_radius = default_vision_radius;
}

void Map::set_vision_radius(int new_radius){
  vision_radius = new_radius;
}

void Map::set_map(std::vector< std::vector<char> > new_map){
  map = new_map;
}

void Map::AddActor(Actor actor){
}

void Map::ClearActor(Actor actor){
}

void Map::MoveActor(Actor actor, Coordinate new_pos){
}

std::string Map::GetSurroundings(Coordinate coordinate){
}

Coordinate Map::RandomEmptyLocation(){
}
