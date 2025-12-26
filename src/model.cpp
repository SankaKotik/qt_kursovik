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
#include <gp_Circ.hxx>
#include <gp_Ax2.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <gp_Vec.hxx>

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

            // Итерируемся по количеству треугольников (NbTriangles)
            for (Standard_Integer i = 1; i <= tri->NbTriangles(); i++) {
                // Получаем индексы вершин i-го треугольника
                Standard_Integer n1, n2, n3;
                tri->Triangle(i).Get(n1, n2, n3);

                // Получаем сами точки по этим индексам
                gp_Pnt p1 = tri->Node(n1).Transformed(trsf);
                gp_Pnt p2 = tri->Node(n2).Transformed(trsf);
                gp_Pnt p3 = tri->Node(n3).Transformed(trsf);

                // Записываем треугольник в ваш массив
                std::array<vec3<float>, 3> trianglePoints;
                trianglePoints[0] = { (float)p1.X(), (float)p1.Y(), (float)p1.Z() };
                trianglePoints[1] = { (float)p2.X(), (float)p2.Y(), (float)p2.Z() };
                trianglePoints[2] = { (float)p3.X(), (float)p3.Y(), (float)p3.Z() };

                vertex.push_back(trianglePoints);
            }
        }
    }
}

void Cube::initModel() {
    // Геометрия: Окружность в плоскости XY с радиусом 50
    gp_Circ circleGeom(gp_Ax2(gp_Pnt(0, 0, 0), gp_Dir(0, 0, 1)), 5.0);

    // Топология: Ребро из окружности
    TopoDS_Edge circleEdge = BRepBuilderAPI_MakeEdge(circleGeom);

    // Топология: Замкнутый контур (Wire)
    TopoDS_Wire circleWire = BRepBuilderAPI_MakeWire(circleEdge);

    // Создание плоской грани на основе контура
    TopoDS_Face circleFace = BRepBuilderAPI_MakeFace(circleWire);

    // Вектор выдавливания (например, по оси Z на 100 единиц)
    gp_Vec extrusionVec(0, 0, 10.0);

    // Выполнение операции выдавливания
    BRepPrimAPI_MakePrism prismMaker(circleFace, extrusionVec);

    // Получение итогового TopoDS_Shape
    shape = prismMaker.Shape();
    
    // shape = BRepPrimAPI_MakeBox(10.0, 10.0, 10.0).Shape();
}
