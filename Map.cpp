#include "Map.h"
#include <random>

Map::Map(void){
  std::vector<char> default_row(default_map_width, kEmpty);
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
  bool (Map::*cases)(int, int);
  cases = &Map::AddActorCases;
  IterateThroughActorDimensions(actor.get_position(), cases);
}

bool Map::AddActorCases(int row, int col){
  if (map[row][col] == kEmpty){
    map[row][col] = kActor;
  }else{
    map[row][col] = kCollision;
  }
  return false;
}

void Map::ClearActor(Actor actor){
  bool (Map::*cases)(int, int);
  cases = &Map::ClearActorCases;
  IterateThroughActorDimensions(actor.get_position(), cases);
}

bool Map::ClearActorCases(int row, int col){
  map[row][col] = kEmpty;
  return false;
}

void Map::MoveActor(Actor actor, Coordinate new_pos){
  actor.set_position(new_pos);
  AddActor(actor);
}

void Map::CheckCollision(Actor actor){
  bool collision = false;
  bool (Map::*cases)(int, int);
  cases = &Map::CheckCollisionCases;
  collision = IterateThroughActorDimensions(actor.get_position(), cases);
  if (collision){
    actor.set_collided(kCollision);
  }
}

bool Map::CheckCollisionCases(int row, int col){
  if (map[row][col] == kCollision){
    return true;
  }else{
    return false;
  }
}


std::vector< std::vector<char> > Map::GetSurroundings(Coordinate coordinate){
  int i, j, map_x, map_y;
  int left_edge, top_edge, right_edge, bottom_edge;
  int grid_rows, grid_cols, grid_row, grid_col;;

  grid_rows    = (vision_radius * 2) + 1;
  grid_cols = (vision_radius * 2) + 1;

  map_x = coordinate.get_x();
  map_y = coordinate.get_y();

  left_edge   = map_x - vision_radius;
  top_edge    = map_y + vision_radius;
  right_edge  = map_x + vision_radius;
  bottom_edge = map_y - vision_radius;

  std::vector<char> vision_columns(grid_cols, kEmpty);
  std::vector< std::vector<char> > vision_grid(grid_rows, vision_columns);

  for (i = top_edge; i <= bottom_edge; i++){
    for (j = left_edge; j <= right_edge; j++){
      grid_row = i - top_edge;
      grid_col = j - left_edge;
      if (out_of_bounds(i, j)){
        vision_grid[grid_row][grid_col] = kObstacle;
      }else{
        vision_grid[grid_row][grid_col] = map[i][j];
      }
    }
  }
}


bool Map::out_of_bounds(int row, int col){
  bool out_of_bounds = false;
  if (row > map_width || row < 0){
    out_of_bounds = true;
  }
  if (col > map_length || col < 0){
    out_of_bounds = false;
  }
  return out_of_bounds;
}


Coordinate Map::RandomEmptyLocation(){
  int x, y;
  Coordinate test_coordinate;
  std::default_random_engine engine;
  std::uniform_int_distribution<int> distrX(0, map_length);
  std::uniform_int_distribution<int> distrY(0, map_width);
  bool (Map::*cases)(int, int);
  cases = &Map::RandomEmptyLocationCases;
  bool IsEmpty = false;
  while (!IsEmpty){
    x = distrX(engine);
    y = distrY(engine);
    test_coordinate.set_x(x);
    test_coordinate.set_y(y);
    IsEmpty = IterateThroughActorDimensions(test_coordinate, cases);
  }
  return test_coordinate;
}

bool Map::RandomEmptyLocationCases(int row, int col){
  if (map[row][col] == kEmpty){
    return false;
  }
  else{
    return true;
  }
}

bool Map::IterateThroughActorDimensions(Coordinate coordinate, bool (Map::*f)(int, int)){
  int i, j, actor_x, actor_y;
  int left_edge, top_edge, right_edge, bottom_edge;
  bool break_out = false;

  actor_x = coordinate.get_x();
  actor_y = coordinate.get_y();

  left_edge   = actor_x - actor_half_length;
  top_edge    = actor_y + actor_half_width;
  right_edge  = actor_x + actor_half_length;
  bottom_edge = actor_y - actor_half_width;

  for (i = top_edge; i <= bottom_edge; i++){
    for (j = left_edge; j <= right_edge; j++){
      break_out = (*this.*f)(i, j);
      if(break_out){
        return break_out;
      }
    }
  }
}
