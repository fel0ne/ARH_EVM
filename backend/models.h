#ifndef MODELS_H
#define MODELS_H

#include <string>
#include <vector>

struct Product {
    std::string name;
    std::string brand;
    double price;
    bool available;
    std::string address;
    int quantity;
    std::string article;
    std::vector<std::string> images; // ["00101", "00102", "00103"]
    std::string specs;

    bool operator<(const Product& other) const {
        return price < other.price;
    }
};

struct Category {
    int id;
    std::string name;
    int some_val1;
    int some_val2;
    std::vector<Product> products;
};

#endif