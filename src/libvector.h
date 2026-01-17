#pragma once

// Реализация двухкомпонентного вектора
#include <algorithm>
#include <cmath>
#include <cstdint>
template <typename T>
struct vec2 {
    T x, y;

    // Конструктор по умолчанию
    vec2() : x(0.0f), y(0.0f) {}

    // Конструктор с одним параметром
    vec2(T x) : x(x), y(x) {}

    // Конструктор с параметрами
    vec2(T x, T y) : x(x), y(y) {}

    // Конструктор для конвертации
    template <typename U>
    vec2(const vec2<U>& other) : x(static_cast<T>(other.x)), y(static_cast<T>(other.y)) {}

    // Перегрузка оператора сложения
    vec2 operator+(const vec2& other) const {
        return vec2(x + other.x, y + other.y);
    }

    // Перегрузка оператора вычитания
    vec2 operator-(const vec2& other) const {
        return vec2(x - other.x, y - other.y);
    }

    // Перегрузка оператора умножения на скаляр
    vec2 operator*(T scalar) const {
        return vec2(x * scalar, y * scalar);
    }

    // Перегрузка оператора деления на скаляр
    vec2 operator/(T scalar) const {
        return vec2(x / scalar, y / scalar);
    }

    // Поэлементное умножение
    vec2 operator*(const vec2& other) const {
        return vec2(x * other.x, y * other.y);
    }

    // Поэлементное деление
    vec2 operator/(const vec2& other) const {
        return vec2(x / other.x, y / other.y);
    }

    // Перегрузка оператора сравнения
    auto operator<=>(vec2 const& other) const = default;

    bool operator>(vec2 const& other) const {
        return x > other.x || y > other.y;
    }

    bool operator>=(vec2 const& other) const {
        return x >= other.x || y >= other.y;
    }

    bool operator<(vec2 const& other) const {
        return x < other.x || y < other.y;
    }

    bool operator<=(vec2 const& other) const {
        return x <= other.x || y <= other.y;
    }

    // Вычисление длины вектора
    float length() const {
        return std::sqrt(x * x + y * y);
    }

    // Нормализация вектора
    vec2 normalize() const {
        float len = length();
        return len != 0.0f ? vec2(x / len, y / len) : *this;
    }
};

// Вспомогательные функции
template <typename T>
vec2<T> abs(vec2<T> a) { return vec2(abs(a.x), abs(a.y)); }
template <typename T>
vec2<T> max(vec2<T> a, vec2<T> b) { return vec2<T>(std::max(a.x, b.x), std::max(a.y, b.y)); }
template <typename T>
vec2<T> min(vec2<T> a, vec2<T> b) { return vec2<T>(std::min(a.x, b.x), std::min(a.y, b.y)); }
template <typename T>
T dot(vec2<T> a, vec2<T> b) { return a.x * b.x + a.y * b.y; }


// Реализация трехкомпонентного вектора
template <typename T>
struct vec3 {
    union {
        struct {
            T x, y, z;
        };
        struct {
            T r, g, b;
        };
    };

    // Конструктор по умолчанию
    vec3() : x(0.0f), y(0.0f), z(0.0f) {}

    // Конструктор с одним параметром
    vec3(T x) : x(x), y(x), z(x) {}

    // Конструктор с параметрами
    vec3(T x, T y, T z) : x(x), y(y), z(z) {}

    // Перегрузка оператора сложения
    vec3 operator+(const vec3& other) const {
        return vec3(x + other.x, y + other.y, z + other.z);
    }

    // Перегрузка оператора вычитания
    vec3 operator-(const vec3& other) const {
        return vec3(x - other.x, y - other.y, z - other.z);
    }

    // Перегрузка оператора умножения на скаляр
    vec3 operator*(T scalar) const {
        return vec3(x * scalar, y * scalar, z * scalar);
    }

    // Перегрузка оператора деления на скаляр
    vec3 operator/(T scalar) const {
        if (scalar != 0.0f) {
            return vec3(x / scalar, y / scalar, z / scalar);
        } else {
            // Избегаем деления на ноль, возвращаем исходный вектор
            return *this;
        }
    }

    // Вычисление длины вектора
    float length() const {
        return std::sqrt(x * x + y * y + z * z);
    }

    // Нормализация вектора
    vec3 normalize() const {
        float len = length();
        if (len != 0.0f) {
            return vec3(x / len, y / len, z / len);
        } else {
            return *this;  // Возвращаем нулевой вектор, если длина нулевая
        }
    }
};

// Четырехкомпонентный вектор
template <typename T>
struct vec4 {
    union {
        struct {
            T x, y, z, w;
        };
        struct {
            T r, g, b, a;
        };
    };

    // Конструктор по умолчанию
    vec4() : x(0), y(0), z(0), w(0) {}

    // Конструктор с одним параметром
    vec4(T x) : x(x), y(x), z(x), w(x) {}

    // Конструктор с четырьмя параметрами
    vec4(T x, T y, T z, T w) : x(x), y(y), z(z), w(w) {}

    // Перегрузка оператора сложения
    vec4 operator+(const vec4& other) const {
        return vec4(x + other.x, y + other.y, z + other.z, w + other.w);
    }

    // Перегрузка оператора вычитания
    vec4 operator-(const vec4& other) const {
        return vec4(x - other.x, y - other.y, z - other.z, w - other.w);
    }

    // Перегрузка оператора умножения на скаляр
    vec4 operator*(T scalar) const {
        return vec4(x * scalar, y * scalar, z * scalar, w * scalar);
    }

    // Перегрузка оператора деления на скаляр
    vec4 operator/(T scalar) const {
        if (scalar != static_cast<T>(0)) {
            return vec4(x / scalar, y / scalar, z / scalar, w / scalar);
        } else {
            return *this;
        }
    }

    // Вычисление длины вектора
    T length() const {
        return std::sqrt(x * x + y * y + z * z + w * w);
    }

    // Нормализация вектора
    vec4 normalize() const {
        T len = length();
        if (len != static_cast<T>(0)) {
            return vec4(x / len, y / len, z / len, w / len);
        } else {
            return *this;
        }
    }
};

// // Интерполяция 
// float clamp(float x, float lowerlimit = 0.0f, float upperlimit = 1.0f) {
//   if (x < lowerlimit) return lowerlimit;
//   if (x > upperlimit) return upperlimit;
//   return x;
// }

// float smoothstep (float edge0, float edge1, float x) {
//    // Scale, and clamp x to 0..1 range
//    x = clamp((x - edge0) / (edge1 - edge0));

//    return x * x * (3.0f - 2.0f * x);
// }

// Цвет
struct color4u : vec4<uint8_t> {
    // Конструктор по умолчанию
    color4u() : vec4(0, 0, 0, 255) {}

    // Конструктор с одним параметром
    color4u(uint8_t x) : vec4(x, x, x, 255) {}

    // Конструктор с четырьмя параметрами
    color4u(uint8_t r, uint8_t g, uint8_t b, uint8_t a) : vec4(r, g, b, a) {}
};

// color4u mix(color4u src, color4u dest) {
//     float alpha = (float)src.a / 255;
//     return color4u(
//         // C = C1 * A1 + C2 * (1 - A1)
//         src.r * alpha + dest.r * (1 - alpha),
//         src.g * alpha + dest.g * (1 - alpha),
//         src.b * alpha + dest.b * (1 - alpha),
//         // A = A1 + A2 * (1 - A1)
//         src.a + int(dest.a * (1 - alpha))
//     );
// }

// Двухкомпонентная матрица
template <typename T>
struct mat2 {
    vec2<T> x, y;

    // Конструктор по умолчанию
    mat2() : x(1, 0), y(0, 1) {}
    mat2(T const* v) : 
    x(v[0], v[1]), 
    y(v[2], v[3]) {}
    mat2(std::initializer_list<T> v) : mat2(v.begin()) {}

    static mat2 scale(T x) {
        mat2 res;
        res.x.x = x;
        res.y.y = x;
        return res;
    }

    static mat2 scale(T x, T y, T z) {
        mat2 res;
        res.x.x = x;
        res.y.y = y;
        return res;
    }

    static mat2 translate(T x, T y, T z) {
        mat2 res;
        res.x.w = x;
        res.y.w = y;
        return res;
    }

    // Матрица поворота
    static mat2 rotate(T x) {
        return mat2{
            cos(x), -sin(x),
            sin(x), cos(x)
        };
    }

    // умножение на матрицу
    mat2 operator*(mat2 other) const {
        return mat2{
            x.x * other.x.x + x.y * other.y.x,
            x.x * other.x.y + x.y * other.y.y,

            y.x * other.x.x + y.y * other.y.x,
            y.x * other.x.y + y.y * other.y.y,
        };
    }

    mat2& operator*=(mat2 other) {
        *this = other * *this;
        return *this;
    }

    // умножение на вектор
    vec2<T> operator*(vec2<T> other) const {
        return vec2{
            x.x * other.x + x.y * other.y,
            y.x * other.x + y.y * other.y,
        };
    }
    
};

// Четырехкомпонентная матрица
template <typename T>
struct mat4 {
    vec4<T> x, y, z, w;

    // Конструктор по умолчанию
    mat4() : x(1, 0, 0, 0), y(0, 1, 0, 0), z(0, 0, 1, 0), w(0, 0, 0, 1) {}
    mat4(T const* v) : 
    x(v[0], v[1], v[2], v[3]), 
    y(v[4], v[5], v[6], v[7]), 
    z(v[8], v[9], v[10], v[11]),
    w(v[12], v[13], v[14], v[15]) {}
    mat4(std::initializer_list<T> v) : mat4(v.begin()) {}

    static mat4 scale(T x) {
        mat4 res;
        res.x.x = x;
        res.y.y = x;
        res.z.z = x;
        return res;
    }

    static mat4 scale(T x, T y, T z) {
        mat4 res;
        res.x.x = x;
        res.y.y = y;
        res.z.z = z;
        return res;
    }

    static mat4 translate(T x, T y, T z) {
        mat4 res;
        res.x.w = x;
        res.y.w = y;
        res.z.w = z;
        return res;
    }

    // Матрица поворота
    static mat4 rotateX(T x) {
        return mat4{
            1, 0, 0, 0,
            0, cos(x), -sin(x), 0,
            0, sin(x), cos(x), 0,
            0, 0, 0, 1,
        };
    }

    static mat4 rotateY(T y) {
        return mat4{
            cos(y), 0, sin(y), 0,
            0, 1, 0, 0,
            -sin(y), 0, cos(y), 0,
            0, 0, 0, 1,
        };
    }

    static mat4 rotateZ(T z) {
        return mat4{
            cos(z), -sin(z), 0, 0,
            sin(z), cos(z), 0, 0,
            0, 0, 1, 0, 
            0, 0, 0, 1,
        };
    }

    static mat4 rotate(T x, T y, T z) {
        return rotateX(z) * rotateY(y) * rotateZ(x);
    }

    // умножение на матрицу
    mat4 operator*(mat4 other) const {
        return mat4{
            x.x * other.x.x + x.y * other.y.x + x.z * other.z.x + x.w * other.w.x,
            x.x * other.x.y + x.y * other.y.y + x.z * other.z.y + x.w * other.w.y,
            x.x * other.x.z + x.y * other.y.z + x.z * other.z.z + x.w * other.w.z,
            x.x * other.x.w + x.y * other.y.w + x.z * other.z.w + x.w * other.w.w,

            y.x * other.x.x + y.y * other.y.x + y.z * other.z.x + y.w * other.w.x,
            y.x * other.x.y + y.y * other.y.y + y.z * other.z.y + y.w * other.w.y,
            y.x * other.x.z + y.y * other.y.z + y.z * other.z.z + y.w * other.w.z,
            y.x * other.x.w + y.y * other.y.w + y.z * other.z.w + y.w * other.w.w,

            z.x * other.x.x + z.y * other.y.x + z.z * other.z.x + z.w * other.w.x,
            z.x * other.x.y + z.y * other.y.y + z.z * other.z.y + z.w * other.w.y,
            z.x * other.x.z + z.y * other.y.z + z.z * other.z.z + z.w * other.w.z,
            z.x * other.x.w + z.y * other.y.w + z.z * other.z.w + z.w * other.w.w,

            w.x * other.x.x + w.y * other.y.x + w.z * other.z.x + w.w * other.w.x,
            w.x * other.x.y + w.y * other.y.y + w.z * other.z.y + w.w * other.w.y,
            w.x * other.x.z + w.y * other.y.z + w.z * other.z.z + w.w * other.w.z,
            w.x * other.x.w + w.y * other.y.w + w.z * other.z.w + w.w * other.w.w
        };
    }

    mat4& operator*=(mat4 other) {
        *this = other * *this;
        return *this;
    }

    // умножение на вектор
    mat4 operator*(vec4<T> other) const {
        return mat4{
            x.x * other.x + x.y * other.y + x.z * other.z + x.w * other.w,
            y.x * other.x + y.y * other.y + y.z * other.z + y.w * other.w,
            z.x * other.x + z.y * other.y + z.z * other.z + z.w * other.w,
            w.x * other.x + w.y * other.y + w.z * other.z + w.w * other.w
        };
    }
    
};