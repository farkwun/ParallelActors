#include "Coordinate.h"

Coordinate::Coordinate(){
  row = -1;
  col = -1;
}

int Coordinate::get_row(){
  return row;
}

int Coordinate::get_col(){
  return col;
}

void Coordinate::set_row(int new_row){
  row = new_row;
}

void Coordinate::set_col(int new_col){
  col = new_col;
}

void Coordinate::set_row_col(int new_row, int new_col){
  row = new_row;
  col = new_col;
}
