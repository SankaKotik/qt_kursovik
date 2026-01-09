#pragma once
#include <TopoDS_Shape.hxx>
#include "libvector.h"
#include "sketch_widget.h"

struct Model
{
    TopoDS_Shape shape;
    std::vector<std::array<vec3<float>, 3>> vertex;
    std::vector<std::vector<float>> params_table;
    QStringList params_table_headings;
    int selectedParameters = 0;

    virtual void initModel3D() = 0;
    virtual void drawSketch(SketchWidget *sketch) = 0;
    void generateMesh();
};

struct Cube : Model
{
    void initModel3D() override;
    void drawSketch(SketchWidget *sketch) override;
};

struct HalfCoupling : Model {
    HalfCoupling() {
        params_table = {
            // Mкр | d | d+t1 | b | d1 | D | l (исп1) | l (исп2) | l1 (исп1) | l1 (исп2) | B | B1 | r | масса исп1 | масса исп2
            {2.5f, 6.0f, 7.0f, 2.0f, 20.0f, 32.0f, 16.0f, 0.0f, 28.0f, 0.0f, 4.0f, 16.0f, 0.1f, 0.08f, 0.0f},   // строка 1
            {2.5f, 7.0f, 8.0f, 2.0f, 20.0f, 32.0f, 16.0f, 0.0f, 28.0f, 0.0f, 4.0f, 16.0f, 0.1f, 0.07f, 0.0f},   // строка 2
            {6.3f, 10.0f, 11.4f, 3.0f, 22.0f, 45.0f, 23.0f, 20.0f, 35.0f, 32.0f, 5.0f, 20.0f, 0.1f, 0.12f, 0.11f}, // строка 3
            {6.3f, 11.0f, 12.8f, 3.0f, 22.0f, 45.0f, 23.0f, 20.0f, 35.0f, 32.0f, 5.0f, 20.0f, 0.1f, 0.11f, 0.10f}, // строка 4
            {6.3f, 12.0f, 13.8f, 4.0f, 24.0f, 45.0f, 30.0f, 25.0f, 42.0f, 37.0f, 5.0f, 20.0f, 0.2f, 0.13f, 0.12f}, // строка 5
            {6.3f, 14.0f, 16.3f, 5.0f, 26.0f, 45.0f, 30.0f, 25.0f, 42.0f, 37.0f, 5.0f, 20.0f, 0.2f, 0.15f, 0.13f}  // строка 6
        };
        params_table_headings = {
            "Номинальный крутящий момент Mкр, Н·м",
            "d (пред. откл. по H7)",
            "d + t₁",
            "b",
            "d₁",
            "D",
            "l (Исполнение 1)",
            "l (Исполнение 2)",
            "l₁ (Исполнение 1)",
            "l₁ (Исполнение 2)",
            "B (пред. откл. +0.1)",
            "B₁",
            "r",
            "Масса, кг (Исполнение 1)",
            "Масса, кг (Исполнение 2)"
        };
    }
    
    void initModel3D() override;
    void drawSketch(SketchWidget *sketch) override;
};

struct Detail1 : Model {
    void initModel3D() override;
    void drawSketch(SketchWidget *sketch) override;
};