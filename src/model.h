#pragma once
#include <TopoDS_Shape.hxx>
#include "libvector.h"

struct Model 
{
    TopoDS_Shape shape;
    std::vector<std::array<vec3<float>, 3>> vertex;

    virtual void initModel() = 0;
    void generateMesh();
};

struct Cube : Model
{
    void initModel() override;
};