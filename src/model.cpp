#include "model.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>  // ← нужен для TopoDS::DownCast
#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepMesh_IncrementalMesh.hxx>
#include <Poly_Triangulation.hxx>
#include <TopLoc_Location.hxx>
#include <BRep_Tool.hxx>
#include <TopExp_Explorer.hxx>
#include <TopAbs_ShapeEnum.hxx>
#include <gp_Pnt.hxx>
#include <gp_Trsf.hxx>

void Model::generateMesh()
{
    // if (!vertex) return;
    vertex.clear();
    if (shape.IsNull()) return;

    // Триангуляция
    BRepMesh_IncrementalMesh mesh(shape, 0.1);
    if (mesh.IsDone()) {
        std::cout << "Сетка успешно создана и сохранена внутри shape" <<std::endl;
    }

    for (TopExp_Explorer ex(shape, TopAbs_FACE); ex.More(); ex.Next()) {
        TopoDS_Face face = TopoDS::Face(ex.Current());
        TopLoc_Location location;
        
        // Получаем триангуляцию грани
        Handle(Poly_Triangulation) tri = BRep_Tool::Triangulation(face, location);
        
        if (!tri.IsNull()) {
            // Получаем матрицу трансформации грани
            gp_Trsf trsf = location.Transformation();

            int iterator = 0;
            // Проход по треугольникам
            for (Standard_Integer triN = 0; triN <= tri->NbNodes() - 3; triN++) {
                std::array<vec3<float>, 3> trianglePoints;
                // Проход по точкам треугольника
                int i = 0;
                for (Standard_Integer index = 1 + triN; index <= 3 + triN; index++, i++) {
                    // 1. Получаем точку
                    gp_Pnt p = tri->Node(index);
                    
                    // 2. Применяем трансформацию, чтобы получить мировые координаты
                    p.Transform(trsf);

                    // 3. Записываем в массив как float
                    float px = static_cast<float>(p.X());
                    float py = static_cast<float>(p.Y());
                    float pz = static_cast<float>(p.Z());

                    trianglePoints[i].x = px;
                    trianglePoints[i].y = py;
                    trianglePoints[i].z = pz;
                    std::cout << "треуг " << iterator << " тчк " << i << " x=" << px << " y=" << py << " z=" << pz << std::endl;
                }

                vertex.push_back(trianglePoints);
            }
        }
    }
}

void Cube::initModel() {
    shape = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
}
