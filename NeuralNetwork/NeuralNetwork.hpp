#ifndef __NEURAL_NETWORK_HPP__
#define __NEURAL_NETWORK_HPP__

#include "Matrix.h"
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>

#include <cassert>
#include <utility>
#include <type_traits>

using namespace std;

struct NetworkParams {
    NetworkParams(unsigned inputs, unsigned outputs, unsigned hidden_layers, unsigned hidden_layer_size):
    inputs(inputs),  outputs(outputs), hidden_layers(hidden_layers), hidden_layer_size(hidden_layer_size) {}

    NetworkParams(): inputs(0),  outputs(0), hidden_layers(0), hidden_layer_size(0) {}

    unsigned inputs;
    unsigned outputs;
    unsigned hidden_layers;
    unsigned hidden_layer_size;
};

class NeuralNetwork {
private:
    unsigned num_layers;
    unsigned inputs;
    unsigned outputs;
    unsigned hidden_layer_size;

    float*** adjacency_matrices; //array of 2d arrays

    float** biases;
    float** activations;

public:
    //note: at least 1 hidden layer is required;
    NeuralNetwork(unsigned inputs, unsigned outputs, unsigned hidden_layers, unsigned hidden_layer_size):
    inputs(inputs),  outputs(outputs), hidden_layer_size(hidden_layer_size) {
        num_layers = hidden_layers + 2;

        //initializing the weights in the adjacency matrices
        adjacency_matrices = new float**[num_layers]; //all layers have a adjacency matrix except for input layer
        for (unsigned index = 0; index < num_layers-1; ++index) {
            if (index == 0) { //first adjacency_matrix is under the second layer
                adjacency_matrices[index] = new float*[hidden_layer_size];
                init_layer(index, hidden_layer_size, inputs);
            }

            else if (index == num_layers-2) { //last adjacency matrix on the output layer
                adjacency_matrices[index] = new float*[outputs];
                init_layer(index, outputs, hidden_layer_size);
            }

            else { // hidden layers connected to hidden layers
                adjacency_matrices[index] = new float*[hidden_layer_size];
                init_layer(index, hidden_layer_size, hidden_layer_size);
            }
        }

        //initializing the biases and activations
        biases = new float*[num_layers];
        activations = new float*[num_layers];
        for (unsigned i = 0; i < num_layers; ++i) {
            if (i == 0) { //input layer
                biases[i] = new float[inputs];
                activations[i] = new float[inputs];
                init_nodes(i, inputs);
            }
            else if (i == num_layers - 1) { //output layer
                biases[i] = new float[outputs];
                activations[i] = new float[outputs];
                init_nodes(i, outputs);
            }
            else { //hidden_layers
                biases[i] = new float[hidden_layer_size];
                activations[i] = new float[hidden_layer_size];
                init_nodes(i, hidden_layer_size);
            }
        }



    }

    NeuralNetwork(NetworkParams & params): NeuralNetwork(params.inputs, params.outputs, params.hidden_layers, params.hidden_layer_size) {}

    NeuralNetwork(unsigned inputs, unsigned outputs, unsigned hidden_layers, unsigned hidden_layer_size, NeuralNetwork* nn1, NeuralNetwork* nn2, float mutation_rate):
    NeuralNetwork(inputs, outputs, hidden_layers, hidden_layer_size) {
        for(unsigned i = 0; i < num_layers; ++i) {
            unsigned size = hidden_layer_size;
            if ( i == 0) {
                size = inputs;
            }
            else if (i == num_layers-1) {
                size = outputs;
            }
            for (unsigned j = 0; j < size; ++j) {
                float new_bias = choose(nn1->get_biases()[i][j], nn2->get_biases()[i][j]);
                biases[i][j] = mutate(new_bias, mutation_rate);
            }
        }

        //copy weights
        for (unsigned index = 0; index < num_layers-1; ++index) {
            unsigned rows = hidden_layer_size;
            unsigned cols = hidden_layer_size;
            if (index == 0) {
                cols = inputs;
            }
            if (index == num_layers-2) {
                rows = outputs;
            }
            for (unsigned i = 0; i < rows; ++i) {
                for (unsigned j = 0; j < cols; ++j) {
                    float new_weight = choose(nn1->get_weights()[index][i][j], nn2->get_weights()[index][i][j]);
                    adjacency_matrices[index][i][j] = mutate(new_weight, mutation_rate);
                }
            }
        }
    }

    NeuralNetwork(NetworkParams & params, NeuralNetwork* nn1, NeuralNetwork* nn2, float mutation_rate):
    NeuralNetwork(params.inputs, params.outputs, params.hidden_layers, params.hidden_layer_size, nn1, nn2, mutation_rate) {}

    NeuralNetwork(NeuralNetwork* nn, NetworkParams & params): NeuralNetwork(params.inputs, params.outputs, params.hidden_layers, params.hidden_layer_size) {
        //copy biases
        //nn->print_biases();
        // std::cout << "saving weights + biases" << std::endl;
        for(unsigned i = 0; i < num_layers; ++i) {
            unsigned size = hidden_layer_size;
            if ( i == 0) {
                size = inputs;
            }
            else if (i == num_layers-1) {
                size = outputs;
            }
            for (unsigned j = 0; j < size; ++j) {
                biases[i][j] = nn->get_biases()[i][j];
            }
        }

        //copy weights
        for (unsigned index = 0; index < num_layers-1; ++index) {
            unsigned rows = hidden_layer_size;
            unsigned cols = hidden_layer_size;
            if (index == 0) {
                cols = inputs;
            }
            if (index == num_layers-2) {
                rows = outputs;
            }
            for (unsigned i = 0; i < rows; ++i) {
                for (unsigned j = 0; j < cols; ++j) {
                    adjacency_matrices[index][i][j] = nn->get_weights()[index][i][j];
                }
            }
        }

        //std::cout << "done" << std::endl;
    }

    NeuralNetwork(string directory) {
        ifstream fin(directory);
        if (!fin.is_open()) {
            cout << "could not open file: " << directory << endl;
        }

        fin >> inputs;
        fin >> outputs;
        fin >> num_layers;
        num_layers += 2;
        fin >> hidden_layer_size;

        //initializing the biases and activations
        biases = new float*[num_layers];
        activations = new float*[num_layers];
        for (unsigned i = 0; i < num_layers; ++i) {
            if (i == 0) { //input layer
                biases[i] = new float[inputs];
                activations[i] = new float[inputs];
                for (unsigned j = 0; j < inputs; ++j) {
                    fin >> biases[i][j];
                }
            }
            else if (i == num_layers - 1) { //output layer
                biases[i] = new float[outputs];
                activations[i] = new float[outputs];
                init_nodes(i, outputs);
                for (unsigned j = 0; j < outputs; ++j) {
                    fin >> biases[i][j];
                }
            }
            else { //hidden_layers
                biases[i] = new float[hidden_layer_size];
                activations[i] = new float[hidden_layer_size];
                for (unsigned j = 0; j < hidden_layer_size; ++j) {
                    fin >> biases[i][j];
                }
            }
        }

        //initializing the weights in the adjacency matrices
        adjacency_matrices = new float**[num_layers]; //all layers have a adjacency matrix except for input layer
        for (unsigned index = 0; index < num_layers-1; ++index) {
            if (index == 0) { //first adjacency_matrix is under the second layer
                adjacency_matrices[index] = new float*[hidden_layer_size];
                //init_layer(index, hidden_layer_size, inputs);
                for (unsigned i = 0; i < hidden_layer_size; ++i) {
                    adjacency_matrices[index][i] = new float[inputs];
                    for (unsigned j = 0; j < inputs; ++j) {
                        fin >> adjacency_matrices[index][i][j];
                    }
                }
            }

            else if (index == num_layers-2) { //last adjacency matrix on the output layer
                adjacency_matrices[index] = new float*[outputs];
                //init_layer(index, outputs, hidden_layer_size);
                for (unsigned i = 0; i < outputs; ++i) {
                    adjacency_matrices[index][i] = new float[hidden_layer_size];
                    for (unsigned j = 0; j < hidden_layer_size; ++j) {
                        fin >> adjacency_matrices[index][i][j];
                    }
                }
            }

            else { // hidden layers connected to hidden layers
                adjacency_matrices[index] = new float*[hidden_layer_size];
                //init_layer(index, hidden_layer_size, hidden_layer_size);
                for (unsigned i = 0; i < hidden_layer_size; ++i) {
                    adjacency_matrices[index][i] = new float[hidden_layer_size];
                    for (unsigned j = 0; j < hidden_layer_size; ++j) {
                        fin >> adjacency_matrices[index][i][j];
                    }
                }
            }
        }
    }

    string save(string directory, unsigned fitness) const {
        srand(this->summnation());

        string file_name = directory;
        file_name += to_string(inputs);
        file_name += "_";
        file_name += to_string(outputs);
        file_name += "_";
        file_name += to_string(num_layers - 2);
        file_name += "_";
        file_name += to_string(hidden_layer_size);
        file_name += "_";
        file_name += "score";
        file_name += to_string(fitness);
        file_name += "_";

        unsigned ID_SIZE = 10;
        for (unsigned i = 0; i < ID_SIZE; ++i) {
            if (rand() % 2 == 0) {
                file_name += rand() % 26 + 'a';
            }
            else {
                file_name += rand() % 10 + '0';
            }
        }

        ofstream fout;
        fout.open(file_name);
        if (!fout.is_open()) {
            cout << "could not open file" << endl;
        }
        cout << file_name << endl;

        fout << inputs << ' ';
        fout << outputs << ' ';
        fout << num_layers-2 << ' ';
        fout << hidden_layer_size << endl;

        //writing weights
        for(unsigned i = 0; i < num_layers; ++i) {
            unsigned size = hidden_layer_size;
            if ( i == 0) {
                size = inputs;
            }
            else if (i == num_layers-1) {
                size = outputs;
            }
            for (unsigned j = 0; j < size; ++j) {
                fout << biases[i][j] << ' ';
            }
        }
        fout << endl;

        //writing biases
        for (unsigned index = 0; index < num_layers-1; ++index) {
            unsigned rows = hidden_layer_size;
            unsigned cols = hidden_layer_size;
            if (index == 0) {
                cols = inputs;
            }
            if (index == num_layers-2) {
                rows = outputs;
            }
            for (unsigned i = 0; i < rows; ++i) {
                for (unsigned j = 0; j < cols; ++j) {
                    fout << adjacency_matrices[index][i][j] << ' ';
                }
            }
        }
        fout << endl;

        fout.close();

        srand(time(0));
        return file_name;
    }
    int summnation() const {
        float summnation = 0;
        for(unsigned i = 0; i < num_layers; ++i) {
            unsigned size = hidden_layer_size;
            if ( i == 0) {
                size = inputs;
            }
            else if (i == num_layers-1) {
                size = outputs;
            }
            for (unsigned j = 0; j < size; ++j) {
                summnation += biases[i][j];
            }
        }

        //copy weights
        for (unsigned index = 0; index < num_layers-1; ++index) {
            unsigned rows = hidden_layer_size;
            unsigned cols = hidden_layer_size;
            if (index == 0) {
                cols = inputs;
            }
            if (index == num_layers-2) {
                rows = outputs;
            }
            for (unsigned i = 0; i < rows; ++i) {
                for (unsigned j = 0; j < cols; ++j) {
                    summnation += adjacency_matrices[index][i][j];
                }
            }
        }

        return summnation;
    }

    float* get_inputs() {
        return activations[0];
    }
    float* get_outputs() {
        return activations[num_layers-1];
    }
    unsigned num_inputs() {
        return inputs;
    }

    void forward_propagation() {
        for (unsigned index = 1; index < num_layers; ++index) {
            float* solution;
            unsigned size;
            if (index == 1) { //input layer -> hidden layer
                size = hidden_layer_size;
                delete[] activations[index]; //delete before reassigning
                activations[index] = Matrix::multiply(adjacency_matrices[index-1], activations[index-1], hidden_layer_size, inputs);
            }
            else if (index == num_layers-1) { //hidden layer -> output layer
                size = outputs;
                delete[] activations[index];
                activations[index] = Matrix::multiply(adjacency_matrices[index-1], activations[index-1], outputs, hidden_layer_size);
            }
            else { //hidden layer -> hidden layer
                size = hidden_layer_size;
                delete[] activations[index];
                activations[index] = Matrix::multiply(adjacency_matrices[index-1], activations[index-1], hidden_layer_size, hidden_layer_size);
            }

            for (unsigned i = 0; i < size; ++i) {
                activations[index][i] += biases[index][i];
                activations[index][i] = Matrix::ReLU(activations[index][i]);
            }
        }
    }

    bool operator==(const NeuralNetwork & nn) const {
        for(unsigned i = 0; i < num_layers; ++i) {
            unsigned size = hidden_layer_size;
            if ( i == 0) {
                size = inputs;
            }
            else if (i == num_layers-1) {
                size = outputs;
            }
            for (unsigned j = 0; j < size; ++j) {
                if (biases[i][j] != nn.get_biases()[i][j]) return false;
            }
        }

        //copy weights
        for (unsigned index = 0; index < num_layers-1; ++index) {
            unsigned rows = hidden_layer_size;
            unsigned cols = hidden_layer_size;
            if (index == 0) {
                cols = inputs;
            }
            if (index == num_layers-2) {
                rows = outputs;
            }
            for (unsigned i = 0; i < rows; ++i) {
                for (unsigned j = 0; j < cols; ++j) {
                    if (adjacency_matrices[index][i][j] != nn.get_weights()[index][i][j]) return false;
                }
            }
        }

        return true;
    }

    float*** get_weights() const {
        return adjacency_matrices;
    }
    const float*** get_weights() {
        return (const float***)(adjacency_matrices);
    }
    float** get_biases() const {
        return biases;
    }
    const float** get_biases() {
        return (const float**)(biases);
    }
    NetworkParams get_params() {
        NetworkParams params(inputs, outputs, num_layers-2, hidden_layer_size);
        return params;
    }

    void print_activations() {
        std::cout << "activations:\n";
        for(unsigned i = 0; i < num_layers; ++i) {
            unsigned size = hidden_layer_size;
            if ( i == 0) {
                size = inputs;
            }
            else if (i == num_layers-1) {
                size = outputs;
            }
            for (unsigned j = 0; j < size; ++j) {
                std::cout<< activations[i][j] << ' ';
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
    void print_biases() {
        std::cout << "biases:\n";
        for(unsigned i = 0; i < num_layers; ++i) {
            unsigned size = hidden_layer_size;
            if ( i == 0) {
                size = inputs;
            }
            else if (i == num_layers-1) {
                size = outputs;
            }
            for (unsigned j = 0; j < size; ++j) {
                std::cout<< biases[i][j] << ' ';
            }
            std::cout << '\n';
        }
        std::cout << '\n';
    }
    void print_weights() {
        std::cout << "weights:-----------------------------------------------------------------------------------------------------------------------------------\n";
        for (unsigned index = 0; index < num_layers-1; ++index) {
            unsigned rows = hidden_layer_size;
            unsigned cols = hidden_layer_size;
            if (index == 0) {
                cols = inputs;
            }
            if (index == num_layers-2) {
                rows = outputs;
            }
            for (unsigned i = 0; i < rows; ++i) {
                for (unsigned j = 0; j < cols; ++j) {
                    std::cout << adjacency_matrices[index][i][j] << ' ';
                }
                std::cout << '\n';
            }
            std::cout << '\n';
        }
        std::cout << "----------------------------------------------------------------------------------------------------------------------------------------------\n";
    }

    ~NeuralNetwork() {
        //std::cout << "destructor called" << std::endl;
        for (unsigned index = 0; index < num_layers-1; ++index) {
            unsigned rows = hidden_layer_size;
            unsigned cols = hidden_layer_size;
            if (index == 0) {
                cols = inputs;
            }
            if (index == num_layers-2) {
                rows = outputs;
            }
            for (unsigned i = 0; i < rows; ++i) {
                delete[] adjacency_matrices[index][i];
            }
            delete[] adjacency_matrices[index];
        }
        delete[] adjacency_matrices;

        for(unsigned i = 0; i < num_layers; ++i) {
            delete[] biases[i];
            delete[] activations[i];
        }
        delete[] biases;
        delete[] activations;
    }
private:
    float fRand(float fMin, float fMax) {
        float f = (float)rand() / RAND_MAX;
        return fMin + f * (fMax - fMin);
    }
    float choose(float x, float y) {
        if (fRand(-1,1) > 0) {
            return x;
        }
        return y;
    }

    float mutate(float x, float mutation_rate) {
        float identifier = fRand(0,1);

        if (identifier <= mutation_rate) {
            x += fRand(-1,1);
        }

        return x;

    }

    void init_layer(unsigned index, unsigned rows, unsigned cols) {
        for (unsigned i = 0; i < rows; ++i) {
            adjacency_matrices[index][i] = new float[cols];
            for (unsigned j = 0; j < cols; ++j) {
                adjacency_matrices[index][i][j] = fRand(-1,1);
            }
        }
    }

    void init_nodes(unsigned index, unsigned layer_size) {
        for (unsigned i = 0; i < layer_size; ++i) {
            if (index == 0) {
                biases[index][i] = 0.0;
            }
            else {
                biases[index][i] = fRand(-1,1);
            }
            activations[index][i] = 0.0;
            //std::cout << biases[index][i] << " | ";
        }
        //std::cout << "\n";
    }

};

#endif
