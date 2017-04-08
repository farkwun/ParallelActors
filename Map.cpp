#include "Map.h"
#include <random>

Map::Map(void){
  std::vector<char> default_row(default_map_cols, kEmpty);
  std::vector< std::vector<char> > default_map(default_map_rows, default_row);
  map_rows = default_map_rows;
  map_cols = default_map_cols;

  seed_engine();

  map = default_map;
  SynchronizePixels();
}

Map::Map(std::vector< std::vector<char> > input_map){
  map_rows = input_map.size();
  map_cols = input_map[0].size();

  seed_engine();

  map = input_map;
  SynchronizePixels();
}

void Map::seed_engine(){
  engine.seed(std::random_device{}());
}

unsigned char Map::get_pixel_for_char(char map_char, int pixel_type){
  unsigned char pixel_val;
  if (pixel_type == A){
    pixel_val = 255;
    return pixel_val;
  }
  switch (map_char) {
    case kObstacle  :
      switch (pixel_type){
        case R :
          pixel_val = 0;
          break;
        case G :
          pixel_val = 0;
          break;
        case B :
          pixel_val = 0;
          break;
      }
      break;
    case kEmpty     :
      switch (pixel_type){
        case R :
          pixel_val = 255;
          break;
        case G :
          pixel_val = 255;
          break;
        case B :
          pixel_val = 255;
          break;
      }
      break;
    case kActor     :
      switch (pixel_type){
        case R :
          pixel_val = 0;
          break;
        case G :
          pixel_val = 0;
          break;
        case B :
          pixel_val = 255;
          break;
      }
      break;
    case kCollision :
      switch (pixel_type){
        case R :
          pixel_val = 255;
          break;
        case G :
          pixel_val = 0;
          break;
        case B :
          pixel_val = 0;
          break;
      }
      break;
  }
  return pixel_val;
}

void Map::InitializeActorDimensions(){
  actor_half_rows = default_actor_half_rows;
  actor_half_cols = default_actor_half_cols;
}

void Map::InitializeActorDimensions(int in_actor_half_rows, int in_actor_half_cols){
  actor_half_rows = in_actor_half_rows;
  actor_half_cols = in_actor_half_cols;
}

void Map::InitializeVision(){
  int side_length;

  vision_radius = default_vision_radius;

  side_length = ((vision_radius*2)+1);
  total_surroundings_grid_size  = side_length * side_length;
}

void Map::InitializeStepSize(){
  step_size = 1;
}

void Map::SynchronizePixels(){
  int num_rows, num_cols, col, row, pindex;
  num_rows = map.size();
  num_cols = map[0].size();
  if (map_pixels == NULL){
    map_pixels = new sf::Uint8[num_cols * num_rows * bytes_per_pixel];
  }

  for (row = 0; row < num_rows; row++){
    for (col = 0; col < num_cols; col++){
      pindex = (row * num_cols + col) * bytes_per_pixel;

      map_pixels[pindex+R] = get_pixel_for_char(map[row][col], R);
      map_pixels[pindex+G] = get_pixel_for_char(map[row][col], G);
      map_pixels[pindex+B] = get_pixel_for_char(map[row][col], B);
      map_pixels[pindex+A] = get_pixel_for_char(map[row][col], A);
    }
  }
}

void Map::set_step_size(int new_step_size){
  step_size = new_step_size;
}

void Map::set_vision_radius(int new_radius){
  int side_length;
  vision_radius = new_radius;

  side_length = ((vision_radius*2)+1);
  total_surroundings_grid_size  = side_length * side_length;
}

std::vector< std::vector<char> > Map::get_map(){
  return map;
}

sf::Uint8 * Map::get_map_pixels(){
  return map_pixels;
}

int Map::get_map_rows(){
  return map_rows;
}

int Map::get_map_cols(){
  return map_cols;
}

int Map::get_total_surroundings_grid_size(){
  return total_surroundings_grid_size;
}

int Map::get_step_size(){
  return step_size;
}

void Map::set_map(std::vector< std::vector<char> > new_map){
  map = new_map;
  SynchronizePixels();
}

void Map::AddActor(Actor actor){
  bool (Map::*cases)(int, int);
  cases = &Map::AddActorCases;
  IterateThroughActorDimensions(actor.get_position(), cases);
}

bool Map::AddActorCases(int row, int col){
  char type;
  if (out_of_bounds(row, col)){
    return false;
  }

  type = map[row][col];

  if (type == kEmpty){
    map[row][col] = kActor;
  }else if (type == kObstacle){
    map[row][col] = type;
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
  char type;
  if (!out_of_bounds(row, col)){
    type = map[row][col];
    if (type == kObstacle){
      map[row][col] = type;
    }else{
      map[row][col] = kEmpty;
    }
  }
  return false;
}

bool Map::IsValidStep(Coordinate old_pos, Coordinate new_pos){
  int manhattan_distance;
  bool step_validity = false;

  if(out_of_bounds(new_pos.get_row(), new_pos.get_col())){
    return false;
  }

  manhattan_distance = abs(new_pos.get_row() - old_pos.get_row()) +
    abs(new_pos.get_col() - old_pos.get_col());

  if (manhattan_distance <= step_size){
    step_validity = true;
  }else{
    step_validity = false;
  }

  return step_validity;
}

Actor Map::MoveActor(Actor actor, Coordinate new_pos){
  if(IsValidStep(actor.get_position(), new_pos)){
    actor.set_invalid_move(false);
    ClearActor(actor);
    actor.set_position(new_pos);
    AddActor(actor);
  }else{
    actor.set_invalid_move(true);
  }
  return actor;
}

Actor Map::CheckCollision(Actor actor){
  bool collision = false;
  bool (Map::*cases)(int, int);
  cases = &Map::CheckCollisionCases;
  collision = IterateThroughActorDimensions(actor.get_position(), cases);
  if (collision){
    actor.set_collided(true);
  }
  return actor;
}

bool Map::CheckCollisionCases(int row, int col){
  if (out_of_bounds(row, col) ||
      map[row][col] == kCollision ||
      map[row][col] == kObstacle){
    return true;
  }else{
    return false;
  }
}

bool Map::AtDestination(Actor actor){
  int actor_row, actor_col;
  int actor_row_min, actor_row_max, actor_col_min, actor_col_max;
  int dest_row, dest_col;
  Coordinate position, destination;

  position    = actor.get_position();
  destination = actor.get_destination();

  actor_row = position.get_row();
  actor_col = position.get_col();

  dest_row = destination.get_row();
  dest_col = destination.get_col();

  actor_row_min = actor_row - actor_half_rows;
  actor_row_max = actor_row + actor_half_rows;

  actor_col_min = actor_col - actor_half_cols;
  actor_col_max = actor_col + actor_half_cols;

  if (dest_row >= actor_row_min &&
      dest_row <= actor_row_max &&
      dest_col >= actor_col_min &&
      dest_col <= actor_col_max
     ){
    return true;
  }else{
    return false;
  }
}

std::vector< std::vector<char> > Map::GetSurroundings(Coordinate coordinate){
  int i, j, map_row, map_col;
  int left_edge, top_edge, right_edge, bottom_edge;
  int grid_rows, grid_cols, grid_row, grid_col;;

  grid_rows = (vision_radius * 2) + 1;
  grid_cols = (vision_radius * 2) + 1;

  map_row = coordinate.get_row();
  map_col = coordinate.get_col();

  left_edge   = map_col - vision_radius;
  right_edge  = map_col + vision_radius;
  top_edge    = map_row - vision_radius;
  bottom_edge = map_row + vision_radius;

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
  return vision_grid;
}


bool Map::out_of_bounds(int row, int col){
  bool out_of_bounds = false;
  if (row >= map_rows || row < 0){
    out_of_bounds = true;
  }
  if (col >= map_cols || col < 0){
    out_of_bounds = true;
  }
  return out_of_bounds;
}


Coordinate Map::RandomEmptyLocation(){
  int row, col;
  Coordinate test_coordinate;
  std::uniform_int_distribution<int> distrRow(0, map_rows);
  std::uniform_int_distribution<int> distrCol(0, map_cols);
  bool (Map::*cases)(int, int);
  cases = &Map::RandomEmptyLocationCases;
  bool IsOccupied = true;
  while (IsOccupied){
    row = distrRow(engine);
    col = distrCol(engine);
    test_coordinate.set_row_col(row, col);
    IsOccupied = IterateThroughActorDimensions(test_coordinate, cases);
  }
  return test_coordinate;
}

bool Map::RandomEmptyLocationCases(int row, int col){
  if (out_of_bounds(row, col)){
    return true;
  }
  if (map[row][col] == kEmpty){
    return false;
  }
  else{
    return true;
  }
}

Coordinate Map::RandomDestination(){
  int row, col;
  Coordinate destination;
  std::uniform_int_distribution<int> distrRow(0, map_rows);
  std::uniform_int_distribution<int> distrCol(0, map_cols);

  row = distrRow(engine);
  col = distrCol(engine);
  destination.set_row_col(row, col);

  return destination;
}

bool Map::IterateThroughActorDimensions(Coordinate coordinate, bool (Map::*f)(int, int)){
  int i, j, actor_row, actor_col;
  int left_edge, top_edge, right_edge, bottom_edge;
  bool break_out = false;

  actor_row = coordinate.get_row();
  actor_col = coordinate.get_col();

  left_edge   = actor_col - actor_half_cols;
  right_edge  = actor_col + actor_half_cols;
  top_edge    = actor_row - actor_half_rows;
  bottom_edge = actor_row + actor_half_rows;

  for (i = top_edge; i <= bottom_edge; i++){
    for (j = left_edge; j <= right_edge; j++){
      break_out = (this->*f)(i, j);
      if(break_out){
        return break_out;
      }
    }
  }
  return break_out;
}
