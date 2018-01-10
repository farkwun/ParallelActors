//===================================
// include guard
#ifndef NEURAL_NET_H
#define NEURAL_NET_H


//===================================
// included dependencies
#include <vector>
#include <armadillo>

//===================================
// the class
class NeuralNet {
  private:
    constexpr static double mutation_coefficient = 0.05;
    const static int hidden_layers = 4;
    const static int output_nodes = 8;
    std::vector<arma::Mat<double>> weights;
  public:
    NeuralNet(int input_layer_size);
    NeuralNet(const NeuralNet&);
    std::vector<arma::Mat<double>> get_weights() const;
    int GetMove(std::vector<int> input);
};
#endif /* NEURAL_NET_H */
