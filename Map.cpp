#include "Map.h"

void Map::set_map(std::vector< std::vector<char> > new_map){
  map = new_map;
}

void Map::AddActor(Coordinate coordinate){
}

void Map::ClearActor(Coordinate coordinate){
}

void Map::MoveActor(Coordinate orig_pos, Coordinate new_pos){
}

std::string Map::GetSurroundings(Coordinate coordinate){
}

Coordinate Map::RandomEmptyLocation(){
}


