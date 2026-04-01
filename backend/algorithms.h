#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <vector>
#include <string>
#include <algorithm>
#include "models.h"

// --- QUICK SORT по цене ---
void quickSort(std::vector<Product>& A, int L, int R) {
    double x = A[(L + R) / 2].price;
    int i = L, j = R;
    while (i <= j) {
        while (A[i].price < x) i++;
        while (A[j].price > x) j--;
        if (i <= j) { std::swap(A[i], A[j]); i++; j--; }
    }
    if (L < j) quickSort(A, L, j);
    if (i < R) quickSort(A, i, R);
}

// --- QUICK SORT по сложному ключу: сначала бренд (A-Z), потом цена ---
bool complexLess(const Product& a, const Product& b) {
    if (a.brand != b.brand) return a.brand < b.brand;
    return a.price < b.price;
}

void quickSortComplex(std::vector<Product>& A, int L, int R) {
    int i = L, j = R;
    const Product& pivot = A[(L + R) / 2];
    while (i <= j) {
        while (complexLess(A[i], pivot)) i++;
        while (complexLess(pivot, A[j])) j--;
        if (i <= j) { std::swap(A[i], A[j]); i++; j--; }
    }
    if (L < j) quickSortComplex(A, L, j);
    if (i < R) quickSortComplex(A, i, R);
}

// --- ДЕРЕВО ОПТИМАЛЬНОГО ПОИСКА (Алгоритм А1) ---
struct NodeA1 {
    std::string article;
    Product data;
    int weight;
    NodeA1 *left, *right;

    NodeA1(Product p) : article(p.article), data(p), weight(1), left(nullptr), right(nullptr) {
        article.erase(0, article.find_first_not_of(" \t\r\n"));
        article.erase(article.find_last_not_of(" \t\r\n") + 1);
    }
};

class OptimalSearchTree {
private:
    NodeA1* root;

    void insert(NodeA1*& p, Product pData) {
        if (p == nullptr) p = new NodeA1(pData);
        else if (pData.article < p->article) insert(p->left, pData);
        else insert(p->right, pData);
    }

public:
    OptimalSearchTree() : root(nullptr) {}

    void buildA1(std::vector<Product>& products) {
        if (products.size() < 2) {
            for (const auto& p : products) insert(root, p);
            return;
        }
        for (size_t i = 0; i < products.size() - 1; i++)
            for (size_t j = products.size() - 1; j > i; j--)
                if (products[j].quantity > products[j - 1].quantity)
                    std::swap(products[j], products[j - 1]);
        for (const auto& p : products) insert(root, p);
    }

    Product* search(std::string article) {
        article.erase(0, article.find_first_not_of(" \t\r\n"));
        article.erase(article.find_last_not_of(" \t\r\n") + 1);
        NodeA1* curr = root;
        while (curr) {
            if (article == curr->article) return &curr->data;
            if (article < curr->article) curr = curr->left;
            else curr = curr->right;
        }
        return nullptr;
    }
};

#endif