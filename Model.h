//===================================
// include guard
#ifndef MODEL_H
#define MODEL_H


//===================================
// included dependencies
#include <vector>

//===================================
// the class
class Model{
  private:
    double score;
  public:
    double get_score() const;
    void set_score(double new_score);
    virtual int GetMove(std::vector<int> input){}
};
#endif /* MODEL_H */
