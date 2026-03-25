#pragma once
#include "httplib.h"
#include "nlohmann/json.hpp"
#include <vector>
#include <string>

using json = nlohmann::json;

struct ProductDTO {
    std::string name;
    std::string brand;
    double price;
    bool available;
    std::string address;
    int quantity;
    std::string article;
    std::string specs;
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

    std::vector<ProductDTO> getProducts(int categoryId, const std::string& sort = "") {
        std::vector<ProductDTO> result;

        std::string url = "/api/products?categoryId=" + std::to_string(categoryId);
        if (!sort.empty()) url += "&sort=" + sort;
        auto res = cli.Get(url.c_str());
        if (res && res->status == 200) {
            auto j = json::parse(res->body);
            for (auto& item : j) {
                ProductDTO p;
                p.name = item.value("name", "");
                p.brand = item.value("brand", "-");
                p.price = item.value("price", 0.0);
                p.available = item.value("available", true);
                p.address = item.value("address", "-");
                p.quantity = item.value("quantity", 0);
                p.article = item.value("article", "");
                p.specs = item.value("specs", "");
                result.push_back(p);
            }
        }
        return result;
    }

    bool addProduct(int catId, const ProductDTO& p) {
        json j = {
            {"categoryId", catId},
            {"name", p.name},
            {"brand", p.brand},
            {"price", p.price},
            {"available", p.available},
            {"address", p.address},
            {"quantity", p.quantity},
            {"article", p.article},
            {"specs", p.specs}
        };

        auto res = cli.Post("/api/products", j.dump(), "application/json");
        return res && res->status == 201;
    }

    bool deleteProduct(const std::string& article) {
        auto res = cli.Delete(("/api/products/" + article).c_str());
        return res && res->status == 204;
    }

    bool updateProduct(const ProductDTO& p) {
        json j = {
            {"name", p.name},
            {"brand", p.brand},
            {"price", p.price},
            {"available", p.available},
            {"address", p.address},
            {"quantity", p.quantity},
            {"article", p.article},
            {"specs", p.specs}
        };

        auto res = cli.Put("/api/products", j.dump(), "application/json");
        return res && res->status == 200;
    }

    ProductDTO getProductByArticle(const std::string& article) {
        ProductDTO p;
        auto res = cli.Get(("/api/products/" + article).c_str());
        if (res && res->status == 200) {
            auto j = json::parse(res->body);
            p.name = j.value("name", "");
            p.brand = j.value("brand", "-");
            p.price = j.value("price", 0.0);
            p.available = j.value("available", true);
            p.address = j.value("address", "-");
            p.quantity = j.value("quantity", 0);
            p.article = j.value("article", "");
            p.specs = j.value("specs", "");
        }
        return p;
    }

};