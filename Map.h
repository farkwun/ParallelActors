//===================================
// include guard
#ifndef MAP_H
#define MAP_H

//===================================
// included dependencies
#include <vector>
#include <string>
#include <random>
#include "Actor.h"
#include "Coordinate.h"
#include <SFML/Graphics.hpp>

//===================================
// the class
#define kObstacle  'O'
#define kEmpty     'E'
#define kActor     'A'
#define kCollision 'C'

class Map {
  private: 
    const static int default_map_cols = 300;
    const static int default_map_rows = 300;
    const static int default_vision_radius = 5;
    const static int default_actor_half_rows = 2;
    const static int default_actor_half_cols = 2;
    const static int R = 0;
    const static int G = 1;
    const static int B = 2;
    const static int A = 3;
    const static int bytes_per_pixel = 4;
    std::vector< std::vector<char> > map;
    sf::Uint8 * map_pixels = NULL;
    std::default_random_engine engine;
    int map_rows;
    int map_cols;
    int actor_half_rows;
    int actor_half_cols;
    int vision_radius;
    int total_surroundings_grid_size;
    int step_size;
    void seed_engine();
    unsigned char get_pixel_for_char(char map_char, int pixel_type);
    bool AddActorCases(int row, int col);
    bool ClearActorCases(int row, int col);
    bool CheckCollisionCases(int row, int col);
    bool RandomEmptyLocationCases(int row, int col);
    bool IsValidStep(Coordinate old_pos, Coordinate new_pos);
  public:
    Map();
    Map(std::vector< std::vector<char> > input_map);
    void InitializeActorDimensions();
    void InitializeActorDimensions(int in_actor_half_rows, int in_actor_half_cols);
    void InitializeVision();
    void InitializeStepSize();
    void SynchronizePixels();
    void set_step_size(int new_step_size);
    void set_vision_radius(int new_radius);
    int  get_total_surroundings_grid_size();
    int  get_step_size();
    std::vector< std::vector<char> > get_map();
    sf::Uint8 * get_map_pixels();
    int get_map_rows();
    int get_map_cols();
    void set_map(std::vector< std::vector<char> > new_map);
    void AddActor(Actor actor);
    void ClearActor(Actor actor);
    Actor MoveActor(Actor actor, Coordinate new_pos);
    Actor CheckCollision(Actor actor);
    bool AtDestination(Actor actor);
    std::vector< std::vector<char> > GetSurroundings(Coordinate coordinate);
    bool out_of_bounds(int row, int col);
    Coordinate RandomEmptyLocation();
    Coordinate RandomDestination();
    bool IterateThroughActorDimensions(Coordinate coordinate, bool (Map::*f)(int, int));
};

#endif /* MAP_H */
