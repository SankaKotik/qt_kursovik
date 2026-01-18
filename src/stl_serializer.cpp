#include "stl_serializer.h"

void StlSerializer::read(const std::string& filename) {
    std::ifstream file(filename);
    if (!file) {
        throw std::runtime_error("Ошибка открытия файла: " + filename);
    }

    std::array<vec3<float>, 3> currentFacet;
    vec3<float> currentNormal;
    bool inFacet = false;
    bool inLoop = false;
    int vertexCount = 0;

    std::string line;
    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string token;
        while (iss >> token) {
            if (token == "solid") {
                // Начало файла
            } else if (token == "facet") {
                inFacet = true;
                vertexCount = 0;
                inLoop = false;
            } else if (token == "normal") {
                if (inFacet) {
                    if (!(iss >> currentNormal.x >> currentNormal.y >> currentNormal.z)) {
                        throw std::runtime_error("Некорректный формат normal в файле STL");
                    }
                }
            } else if (token == "loop") {
                inLoop = true;
            } else if (token == "endloop") {
                inLoop = false;
            } else if (token == "vertex") {
                if (inFacet && inLoop) {
                    if (!(iss >> currentFacet[vertexCount].x >> currentFacet[vertexCount].y >> currentFacet[vertexCount].z)) {
                        throw std::runtime_error("Некорректный формат vertex в файле STL");
                    }
                    vertexCount++;
                }
            } else if (token == "endfacet") {
                if (inFacet && vertexCount == 3) {
                    vertex->push_back(currentFacet);
                    normals->push_back(currentNormal);
                    normals->push_back(currentNormal);
                    normals->push_back(currentNormal);
                } else {
                    throw std::runtime_error("Некорректные вершины в face");
                }
                inFacet = false;
                vertexCount = 0;
                inLoop = false;
            } else if (token == "endsolid") {
                // Конец файла
            }
        }
    }
}

void StlSerializer::write(const std::string& filename) const {
    std::ofstream file(filename);
    if (!file) {
        throw std::runtime_error("Ошибка открытия файла для записи: " + filename);
    }

    file << "solid\n";
    for (int i = 0; i < vertex->size(); i++) {
        auto& facet = vertex->at(i);
        auto& normal = normals->at(i / 3);
        file << "  facet normal "
                << normal.x << " " << normal.y << " " << normal.z << "\n";
        file << "    outer loop\n";
        for (int i = 0; i < 3; ++i) {
            const auto& v = facet[i];
            file << "      vertex "
                    << v.x << " " << v.y << " " << v.z << "\n";
        }
        file << "    endloop\n";
        file << "  endfacet\n";
    }
    file << "endsolid\n";
}
