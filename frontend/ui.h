#ifndef UI_H
#define UI_H

#include "imgui.h"
#include "api_client.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#ifdef __APPLE__
  #include <OpenGL/gl3.h>
#else
  #include <GL/gl.h>
#endif

#include <string>
#include <sstream>
#include <map>
#include <vector>

struct Texture {
    GLuint id = 0;
    int width = 0;
    int height = 0;
};

static Texture loadTextureFromMemory(const std::vector<unsigned char>& data) {
    Texture tex;
    if (data.empty()) return tex;

    int channels;
    unsigned char* pixels = stbi_load_from_memory(
        data.data(), (int)data.size(),
        &tex.width, &tex.height, &channels, 0);

    if (!pixels) return tex;

    GLenum format = GL_RGB;
    if (channels == 1) format = GL_RED;
    else if (channels == 2) format = GL_RG;
    else if (channels == 3) format = GL_RGB;
    else if (channels == 4) format = GL_RGBA;

    glGenTextures(1, &tex.id);
    glBindTexture(GL_TEXTURE_2D, tex.id);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexImage2D(GL_TEXTURE_2D, 0, format,
                 tex.width, tex.height, 0,
                 format, GL_UNSIGNED_BYTE, pixels);
    stbi_image_free(pixels);
    return tex;
}

static void freeTexture(Texture& tex) {
    if (tex.id) { glDeleteTextures(1, &tex.id); tex.id = 0; }
}

class UI {
private:
    ApiClient* api;
    int selectedCategoryId = -1;
    std::vector<ProductDTO> products;
    std::vector<std::pair<int, std::string>> categories;
    int sortMode = 0;

    bool openViewPopup = false;
    bool openEditPopup = false;
    bool openSearchPopup = false;

    char searchArticleBuf[64] = "";
    ProductDTO foundProduct;
    bool hasSearchResult = false;

    char nameBuf[128] = "";
    char brandBuf[128] = "";
    float priceBuf = 0.0f;
    int quantityBuf = 0;
    char addressBuf[128] = "";
    char articleBuf[64] = "";
    char specsBuf[256] = "";
    bool availableBuf = true;

    char editName[128] = "";
    char editBrand[128] = "";
    float editPrice = 0.0f;
    int editQuantity = 0;
    char editAddress[128] = "";
    char editArticle[64] = "";
    char editSpecs[256] = "";
    bool editAvailable = true;

    int selectedIndex = -1;

    std::vector<Texture> currentTextures;
    int currentImageIndex = 0;
    std::string currentCategoryFolder;

    void clearTextures() {
        for (auto& t : currentTextures) freeTexture(t);
        currentTextures.clear();
        currentImageIndex = 0;
    }

    std::string getSortParam() {
        if (sortMode == 1) return "price";
        if (sortMode == 2) return "complex";
        return "";
    }

    void loadProductImages(const ProductDTO& p) {
        clearTextures();
        for (const auto& imgName : p.images) {
            auto bytes = api->getImage(currentCategoryFolder, imgName);
            Texture tex = loadTextureFromMemory(bytes);
            currentTextures.push_back(tex);
        }
    }

    static std::string categoryToFolder(const std::string& catName) {
        std::string folder = catName;
        std::replace(folder.begin(), folder.end(), ' ', '_');
        for (auto& c : folder) c = tolower((unsigned char)c);
        return folder;
    }

    std::string getCategoryName(int id) {
        for (auto& [cid, cname] : categories)
            if (cid == id) return cname;
        return "";
    }

public:
    UI(ApiClient* apiClient) : api(apiClient) {}

    ~UI() { clearTextures(); }

    void update() {
        ImGuiViewport* viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove |
                                 ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoSavedSettings;

        ImGui::Begin("AutoParts Manager", NULL, flags);
        drawCategories();
        ImGui::SameLine();
        drawProducts();
        ImGui::End();
    }

private:
    std::map<std::string, std::string> parseSpecs(const std::string& specs) {
        std::map<std::string, std::string> result;
        std::stringstream ss(specs);
        std::string pair;
        while (ss >> pair) {
            size_t pos = pair.find(':');
            if (pos != std::string::npos)
                result[pair.substr(0, pos)] = pair.substr(pos + 1);
        }
        return result;
    }

    void drawCategories() {
        ImGui::BeginChild("Categories", ImVec2(200, 0), true);
        ImGui::Text("Категории");
        ImGui::Separator();

        static bool loadedOnce = false;
        if (!loadedOnce) { categories = api->getCategories(); loadedOnce = true; }

        for (auto& [id, name] : categories) {
            if (ImGui::Selectable(name.c_str(), selectedCategoryId == id)) {
                selectedCategoryId = id;
                products = api->getProducts(id, getSortParam());
                selectedIndex = -1;
                currentCategoryFolder = categoryToFolder(name);
            }
        }
        ImGui::EndChild();
    }

    void drawProducts() {
        ImGui::BeginChild("Products", ImVec2(0, 0), true);

        if (selectedCategoryId == -1) {
            ImGui::Text("Выберите категорию");
            ImGui::EndChild();
            return;
        }

        ImGui::RadioButton("Без сортировки", &sortMode, 0);
        ImGui::SameLine();
        ImGui::RadioButton("По цене", &sortMode, 1);
        ImGui::SameLine();
        ImGui::RadioButton("По бренду + цене", &sortMode, 2);

        if (ImGui::Button("Обновить")) {
            products = api->getProducts(selectedCategoryId, getSortParam());
        }

        ImGui::SameLine();
        if (ImGui::Button("Поиск по артикулу")) openSearchPopup = true;

        if (openSearchPopup) { ImGui::OpenPopup("SearchPopup"); openSearchPopup = false; }

        if (ImGui::BeginPopupModal("SearchPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::InputText("Артикул", searchArticleBuf, IM_ARRAYSIZE(searchArticleBuf));
            if (ImGui::Button("Найти")) {
                foundProduct = api->getProductByArticle(searchArticleBuf);
                hasSearchResult = true;
            }
            if (hasSearchResult) {
                if (!foundProduct.article.empty()) {
                    ImGui::Separator();
                    ImGui::Text("Название: %s", foundProduct.name.c_str());
                    ImGui::Text("Бренд: %s", foundProduct.brand.c_str());
                    ImGui::Text("Цена: %.2f", foundProduct.price);
                    ImGui::Text("Наличие: %s", foundProduct.available ? "Да" : "Нет");
                    ImGui::Text("Адрес: %s", foundProduct.address.c_str());
                    ImGui::Text("Количество: %d", foundProduct.quantity);
                } else {
                    ImGui::Text("Товар не найден");
                }
            }
            if (ImGui::Button("Закрыть")) {
                hasSearchResult = false;
                searchArticleBuf[0] = 0;
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }

        ImGui::Separator();

        if (ImGui::BeginTable("ProductsTable", 7, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg)) {
            ImGui::TableSetupColumn("Название");
            ImGui::TableSetupColumn("Бренд");
            ImGui::TableSetupColumn("Цена");
            ImGui::TableSetupColumn("Наличие");
            ImGui::TableSetupColumn("Адрес");
            ImGui::TableSetupColumn("Кол-во");
            ImGui::TableSetupColumn("Артикул");
            ImGui::TableHeadersRow();

            for (size_t i = 0; i < products.size(); ++i) {
                auto& p = products[i];
                ImGui::PushID((int)i);
                ImGui::TableNextRow();
                ImGui::TableNextColumn();
                if (ImGui::Selectable(p.name.c_str())) {
                    selectedIndex = i;
                    strcpy(editName, p.name.c_str());
                    strcpy(editBrand, p.brand.c_str());
                    strcpy(editAddress, p.address.c_str());
                    strcpy(editArticle, p.article.c_str());
                    strcpy(editSpecs, p.specs.c_str());
                    editPrice = p.price;
                    editQuantity = p.quantity;
                    editAvailable = p.available;
                    openViewPopup = true;
                }
                ImGui::TableNextColumn(); ImGui::Text("%s", p.brand.c_str());
                ImGui::TableNextColumn(); ImGui::Text("%.2f", p.price);
                ImGui::TableNextColumn(); ImGui::Text(p.available ? "Да" : "Нет");
                ImGui::TableNextColumn(); ImGui::Text(p.available ? p.address.c_str() : "-");
                ImGui::TableNextColumn(); ImGui::Text("%d", p.quantity);
                ImGui::TableNextColumn(); ImGui::Text("%s", p.article.c_str());
                ImGui::PopID();
            }
            ImGui::EndTable();
        }

        // --- Popup просмотра ---
        if (openViewPopup) {
            ImGui::OpenPopup("ViewPopup");
            openViewPopup = false;
            if (selectedIndex >= 0 && selectedIndex < (int)products.size()) {
                loadProductImages(products[selectedIndex]);
                currentImageIndex = 0;
            }
        }

        if (selectedIndex >= 0 && ImGui::BeginPopupModal("ViewPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            auto& p = products[selectedIndex];

            ImGui::Text("%s", p.name.c_str());
            ImGui::Separator();
            ImGui::Text("Бренд: %s", p.brand.c_str());
            ImGui::Text("Цена: %.2f", p.price);
            ImGui::Text("Наличие: %s", p.available ? "Да" : "Нет");
            if (p.available) ImGui::Text("Адрес: %s", p.address.c_str());
            ImGui::Text("Количество: %d", p.quantity);
            ImGui::Separator();
            ImGui::Text("Характеристики:");
            for (auto& [k, v] : parseSpecs(p.specs))
                ImGui::BulletText("%s: %s", k.c_str(), v.c_str());

            //картинки в ряд
            if (!currentTextures.empty()) {
                ImGui::Separator();
                ImGui::Text("Фото (%d шт.):", (int)currentTextures.size());
                for (auto& tex : currentTextures) {
                    if (tex.id) {
                        float maxH = 200.0f;
                        float scale = maxH / tex.height;
                        if (scale > 1.0f) scale = 1.0f;
                        ImGui::Image((ImTextureID)(intptr_t)tex.id,
                                    ImVec2(tex.width * scale, tex.height * scale));
                        ImGui::SameLine();
                    }
                }
                ImGui::NewLine();
                ImGui::Separator();
            }

            if (ImGui::Button("Редактировать")) openEditPopup = true;
            ImGui::SameLine();
            if (ImGui::Button("Удалить")) {
                if (!p.article.empty()) api->deleteProduct(p.article);
                products = api->getProducts(selectedCategoryId, getSortParam());
                selectedIndex = -1;
                clearTextures();
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Закрыть")) {
                selectedIndex = -1;
                clearTextures();
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();

            if (openEditPopup) { ImGui::OpenPopup("EditPopup"); openEditPopup = false; }
        }

        if (ImGui::BeginPopupModal("EditPopup", NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Редактирование");
            ImGui::Separator();
            ImGui::InputText("Название", editName, IM_ARRAYSIZE(editName));
            ImGui::InputText("Бренд", editBrand, IM_ARRAYSIZE(editBrand));
            ImGui::InputFloat("Цена", &editPrice);
            ImGui::Checkbox("В наличии", &editAvailable);
            ImGui::InputText("Адрес", editAddress, IM_ARRAYSIZE(editAddress));
            ImGui::InputInt("Количество", &editQuantity);
            ImGui::InputText("Артикул", editArticle, IM_ARRAYSIZE(editArticle));
            ImGui::InputText("Характеристики", editSpecs, IM_ARRAYSIZE(editSpecs));

            if (ImGui::Button("Сохранить")) {
                ProductDTO dto;
                dto.name = editName; dto.brand = editBrand; dto.price = editPrice;
                dto.available = editAvailable; dto.address = editAddress;
                dto.quantity = editQuantity; dto.article = editArticle; dto.specs = editSpecs;
                api->updateProduct(dto);
                products = api->getProducts(selectedCategoryId);
                selectedIndex = -1;
                ImGui::CloseCurrentPopup();
            }
            ImGui::SameLine();
            if (ImGui::Button("Отмена")) ImGui::CloseCurrentPopup();
            ImGui::EndPopup();
        }

        ImGui::Separator();
        drawAddProduct();
        ImGui::EndChild();
    }

    void drawAddProduct() {
        if (ImGui::CollapsingHeader("Добавить товар")) {
            ImGui::InputText("Название", nameBuf, IM_ARRAYSIZE(nameBuf));
            ImGui::InputText("Бренд", brandBuf, IM_ARRAYSIZE(brandBuf));
            ImGui::InputFloat("Цена", &priceBuf);
            ImGui::Checkbox("В наличии", &availableBuf);
            ImGui::InputText("Адрес", addressBuf, IM_ARRAYSIZE(addressBuf));
            ImGui::InputInt("Количество", &quantityBuf);
            ImGui::InputText("Артикул", articleBuf, IM_ARRAYSIZE(articleBuf));
            ImGui::InputText("Характеристики", specsBuf, IM_ARRAYSIZE(specsBuf));

            if (ImGui::Button("Добавить")) {
                ProductDTO p;
                p.name = nameBuf; p.brand = brandBuf; p.price = priceBuf;
                p.available = availableBuf; p.address = addressBuf;
                p.quantity = quantityBuf; p.article = articleBuf; p.specs = specsBuf;
                api->addProduct(selectedCategoryId, p);
                products = api->getProducts(selectedCategoryId);
                selectedIndex = -1;
                clearForm();
            }
        }
    }

    void clearForm() {
        nameBuf[0] = brandBuf[0] = addressBuf[0] = articleBuf[0] = specsBuf[0] = 0;
        priceBuf = 0.0f; quantityBuf = 0; availableBuf = true;
    }
};

#endif