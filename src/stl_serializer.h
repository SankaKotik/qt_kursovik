#pragma once

#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <exception>
#include "libvector.h"

// Класс-парсер для STL-файлов
class StlSerializer {
public:
    StlSerializer() = default;

    StlSerializer(std::vector<std::array<vec3<float>, 3>> *vertex,
                  std::vector<vec3<float>> *normals)
        : vertex(vertex), normals(normals) {}
    
    std::vector<std::array<vec3<float>, 3>> *vertex;
    std::vector<vec3<float>> *normals;

    // Метод для чтения STL-файла
    void read(const std::string& filename);

    // Метод для записи STL-файла
    void write(const std::string& filename) const;
};
