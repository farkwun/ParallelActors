//===================================
// include guard
#ifndef NEURAL_NET_H
#define NEURAL_NET_H


//===================================
// included dependencies
#include <vector>
#include <armadillo>
#include "Model.h"

//===================================
// the class
class NeuralNet : public Model{
  private:
    constexpr static double mutation_coefficient = 0.05;
    const static int hidden_layers = 1;
    const static int output_nodes = 4;
    std::vector<arma::Mat<double>> weights;
  public:
    NeuralNet(int input_layer_size);
    NeuralNet(const NeuralNet&);
    std::vector<arma::Mat<double>> get_weights() const;
    int GetMove(std::vector<int> input) override;
};
#endif /* NEURAL_NET_H */
