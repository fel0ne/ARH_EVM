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
    std::vector<std::string> images; // <-- добавили
    std::string specs;
};

class ApiClient {
private:
    httplib::Client cli{"http://localhost:8080"};

    // Хелпер для парсинга ProductDTO из json-объекта
    ProductDTO parseProduct(const json& item) {
        ProductDTO p;
        p.name = item.value("name", "");
        p.brand = item.value("brand", "-");
        p.price = item.value("price", 0.0);
        p.available = item.value("available", true);
        p.address = item.value("address", "-");
        p.quantity = item.value("quantity", 0);
        p.article = item.value("article", "");
        p.specs = item.value("specs", "");
        if (item.contains("images") && item["images"].is_array()) {
            for (auto& img : item["images"]) p.images.push_back(img.get<std::string>());
        }
        return p;
    }

public:
    std::vector<std::pair<int, std::string>> getCategories() {
        std::vector<std::pair<int, std::string>> result;
        auto res = cli.Get("/api/categories");
        if (res && res->status == 200) {
            auto j = json::parse(res->body);
            for (auto& item : j)
                result.emplace_back(item["id"], item["name"]);
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
            for (auto& item : j) result.push_back(parseProduct(item));
        }
        return result;
    }

    ProductDTO getProductByArticle(const std::string& article) {
        auto res = cli.Get(("/api/products/" + article).c_str());
        if (res && res->status == 200)
            return parseProduct(json::parse(res->body));
        return {};
    }

    bool addProduct(int catId, const ProductDTO& p) {
        json j = {
            {"categoryId", catId}, {"name", p.name}, {"brand", p.brand},
            {"price", p.price}, {"available", p.available}, {"address", p.address},
            {"quantity", p.quantity}, {"article", p.article}, {"specs", p.specs}
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
            {"name", p.name}, {"brand", p.brand}, {"price", p.price},
            {"available", p.available}, {"address", p.address},
            {"quantity", p.quantity}, {"article", p.article}, {"specs", p.specs}
        };
        auto res = cli.Put("/api/products", j.dump(), "application/json");
        return res && res->status == 200;
    }

    // Загружает байты картинки с сервера
    // categoryFolder — например "глушитель_задний", imageName — "00101"
    std::vector<unsigned char> getImage(const std::string& categoryFolder,
                                        const std::string& imageName) {
        std::string url = "/api/images/" + categoryFolder + "/" + imageName + ".jpg";
        auto res = cli.Get(url.c_str());
        if (res && res->status == 200) {
            return std::vector<unsigned char>(res->body.begin(), res->body.end());
        }
        return {};
    }
};