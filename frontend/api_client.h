#pragma once
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <string>

using json = nlohmann::json;

struct ProductDTO {
    std::string name;
    double price;
    std::string article;
    int quantity;
};

class ApiClient {
private:
    httplib::Client cli{"http://localhost:8080"};

public:

    std::vector<std::pair<int, std::string>> getCategories() {
        std::vector<std::pair<int, std::string>> result;

        auto res = cli.Get("/api/categories");
        if (res && res->status == 200) {
            auto j = json::parse(res->body);
            for (auto& item : j) {
                result.emplace_back(item["id"], item["name"]);
            }
        }
        return result;
    }

    std::vector<ProductDTO> getProducts(int categoryId) {
        std::vector<ProductDTO> result;

        auto res = cli.Get(("/api/products?categoryId=" + std::to_string(categoryId)).c_str());
        if (res && res->status == 200) {
            auto j = json::parse(res->body);
            for (auto& item : j) {
                result.push_back({
                    item["name"],
                    item["price"],
                    item["article"],
                    item["quantity"]
                });
            }
        }
        return result;
    }

    bool addProduct(int catId, const ProductDTO& p) {
        json j = {
            {"categoryId", catId},
            {"name", p.name},
            {"price", p.price},
            {"article", p.article},
            {"quantity", p.quantity}
        };

        auto res = cli.Post("/api/products", j.dump(), "application/json");
        return res && res->status == 201;
    }
};