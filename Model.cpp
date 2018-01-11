#include "Model.h"

double Model::get_score() const{
  return score;
}

void Model::set_score(double new_score){
  score = new_score;
}
