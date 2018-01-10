#include "NeuralNet.h"
#include <random>
#include <armadillo>

NeuralNet::NeuralNet(int input_layer_size){
  arma::arma_rng::set_seed_random();
  for (int i = 0; i < hidden_layers; i++){
    arma::Mat<double> temp = arma::randu(input_layer_size+1, input_layer_size);
    weights.push_back (temp);
  }
    arma::Mat<double> temp = arma::randu(input_layer_size+1, output_nodes);
    weights.push_back (temp);
}

NeuralNet::NeuralNet(const NeuralNet& parent){
  arma::arma_rng::set_seed_random();
  weights = parent.get_weights();
  double range = mutation_coefficient * 2;
  arma::Mat<double> temp;
  for (int i = 0; i < weights.size(); i++){
    temp = weights[i];
    arma::Mat<double> mutate = arma::randu(temp.n_rows, temp.n_cols);
    mutate.transform( [range](double val) { return ((val * range) - mutation_coefficient); } );
    weights[i] = weights[i] + mutate;
  }
}

std::vector<arma::Mat<double>> NeuralNet::get_weights(void) const {
  return weights;
}

int NeuralNet::GetMove(std::vector<int> input){
  std::vector<double> inputvec (input.begin(), input.end());
  arma::Mat<double> temp_layer = arma::rowvec(inputvec);

  for (int i = 0; i < weights.size(); i++){
    temp_layer.insert_cols(temp_layer.n_cols, 1);
    temp_layer = temp_layer * weights[i];
    temp_layer = temp_layer.transform( [](double val) { return (val/(1+fabs(val))); } );
  }

  return temp_layer.index_max();
}
