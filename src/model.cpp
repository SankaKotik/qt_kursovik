#include "model.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS.hxx>
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
#include <gp_Ax3.hxx>
#include <gp_Pln.hxx>
#include <gp_Vec.hxx>
#include <ElSLib.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <GC_MakeArcOfCircle.hxx>

void Model::generateMesh()
{
    // if (!vertex) return;
    vertex.clear();
    if (shape.IsNull()) return;

    // Триангуляция
    BRepMesh_IncrementalMesh mesh(shape, 0.1);
    if (mesh.IsDone()) {
        std::cout << ("Сетка успешно создана и сохранена внутри shape\n");
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

void Cube::initModel3D() {
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

void Cube::drawSketch(SketchWidget *sketch) {
    
}

void HalfCoupling::initModel3D() {
    double l = params_table[selectedParameters][6];
    double d1 = params_table[selectedParameters][4];
    double D = params_table[selectedParameters][5];
    double d = params_table[selectedParameters][1];
    double dt1 = params_table[selectedParameters][2];
    double b = params_table[selectedParameters][3];
    
    // Вращение
    TopoDS_Shape revolvedSolid;
    {
        gp_Pnt p1(0, 0, 0),
        p2(l, 0, 0),
        p3(l, d1 / 2., 0),
        p4(16. - 10.5 - 1., d1 / 2., 0),
        p5(16. - 10.5 - 1., D / 2., 0),
        p6(0, D / 2., 0);
        
        // Полилиния
        BRepBuilderAPI_MakePolygon polyMaker;
        polyMaker.Add(p1);
        polyMaker.Add(p2);
        polyMaker.Add(p3);
        polyMaker.Add(p4);
        polyMaker.Add(p5);
        polyMaker.Add(p6);
        polyMaker.Close();
        
        TopoDS_Wire polyline = polyMaker.Wire();
        
        // Ось вращения (Z)
        gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
        
        // Создаем тело вращения на 360 градусов (2*PI)
        BRepPrimAPI_MakeRevol revolMaker(polyline, axis);
        revolvedSolid = revolMaker.Shape();
    }
    
    // Выдавливание
    TopoDS_Shape extrusionSolid;
    {
        // Определение системы координат эскиза
        gp_Pnt origin(0, 0, 0);
        gp_Dir normal(1, 0, 0);
        gp_Ax3 sketchSystem(origin, normal);
        
        // точки 2d
        double circlePointY = sqrt(pow(d / 2, 2) - pow(b / 2, 2));
        gp_Pnt2d sp1(-b / 2, circlePointY),
        sp2(-b / 2., dt1 - d / 2),
        sp3(b / 2., dt1 - d / 2),
        sp4(b / 2., circlePointY);
        
        // Поворот систему координат эскиза на 45 градусов относительно нормали
        gp_Trsf rotation;
        rotation.SetRotation(gp_Ax1(sketchSystem.Location(), sketchSystem.Direction()), 45.0 * M_PI / 180.0);
        sketchSystem.Transform(rotation);
        
        // Создаем плоскость эскиза
        gp_Pln sketchPlane(sketchSystem);
        
        // Преобразуем в 3D (вычисляет 3D-координату на основе UV-параметров поверхности)
        gp_Pnt p1 = ElSLib::Value(sp1.X(), sp1.Y(), sketchPlane);
        gp_Pnt p2 = ElSLib::Value(sp2.X(), sp2.Y(), sketchPlane);
        gp_Pnt p3 = ElSLib::Value(sp3.X(), sp3.Y(), sketchPlane);
        gp_Pnt p4 = ElSLib::Value(sp4.X(), sp4.Y(), sketchPlane);
        
        TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
        TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
        TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4);
        
        // Окружность для дуги
        gp_Pnt center(0, 0, 0);
        gp_Ax2 circleAx(center, normal);
        gp_Circ circle(circleAx, d / 2);
        
        // Дуга по окружности и двум точкам
        GC_MakeArcOfCircle arcMaker(circle, p1, p4, true); 
        TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(arcMaker.Value());
        
        BRepBuilderAPI_MakeWire wireMaker;
        wireMaker.Add(e1); wireMaker.Add(e2); 
        wireMaker.Add(e3); wireMaker.Add(e4);
        TopoDS_Wire outerWire = wireMaker.Wire();
        
        TopoDS_Face face = BRepBuilderAPI_MakeFace(outerWire);
        
        gp_Vec extrusionVec(l, 0, 0);
        BRepPrimAPI_MakePrism prism(face, extrusionVec);
        
        extrusionSolid = prism.Shape();
    }
    
    // Булева: вычитание
    TopoDS_Shape booleanShape;
    {
        BRepAlgoAPI_Cut cutMaker(revolvedSolid, extrusionSolid);
        booleanShape = cutMaker.Shape();
    }
    
    shape = booleanShape;
}

void HalfCoupling::drawSketch(SketchWidget *sketch) {
    double l = params_table[selectedParameters][6];
    double d1 = params_table[selectedParameters][4];
    double D = params_table[selectedParameters][5];
    double d = params_table[selectedParameters][1];
    double dt1 = params_table[selectedParameters][2];
    double b = params_table[selectedParameters][3];
    
    
    QPointF p1(0, 0),
    p2(l, 0),
    p3(l, d1 / 2.),
    p4(16. - 10.5 - 1., d1 / 2.),
    p5(16. - 10.5 - 1., D / 2.),
    p6(0, D / 2.);
    
    sketch->addLine({p1, p2});
    sketch->addLine({p2, p3});
    sketch->addLine({p3, p4});
    sketch->addLine({p4, p5});
    sketch->addLine({p5, p6});
    sketch->addLine({p6, p1});
}

void Detail1::initModel3D() {
    gp_Pnt p1(0, 0, 0), p2(100, 0, 0), p3(100, 50, 0), p4(10, 50, 0), p5(0, 40, 0), p6(3, 47, 0);

    TopoDS_Edge e1 = BRepBuilderAPI_MakeEdge(p1, p2);
    TopoDS_Edge e2 = BRepBuilderAPI_MakeEdge(p2, p3);
    TopoDS_Edge e3 = BRepBuilderAPI_MakeEdge(p3, p4);

    GC_MakeArcOfCircle arcMaker(p4, p6, p5); 
    TopoDS_Edge e4 = BRepBuilderAPI_MakeEdge(arcMaker.Value());
    TopoDS_Edge e5 = BRepBuilderAPI_MakeEdge(p5, p1);

    BRepBuilderAPI_MakeWire wireMaker;
    wireMaker.Add(e1); wireMaker.Add(e2); wireMaker.Add(e3);
    wireMaker.Add(e4); wireMaker.Add(e5);
    TopoDS_Wire outerWire = wireMaker.Wire();

    gp_Pnt center(50, 25, 0);
    gp_Dir normal(0, 0, -1);
    gp_Ax2 circleAx(center, normal);
    gp_Circ circle(circleAx, 10.0); // Радиус 10
    TopoDS_Edge holeEdge = BRepBuilderAPI_MakeEdge(circle);
    TopoDS_Wire innerWire = BRepBuilderAPI_MakeWire(holeEdge);

    BRepBuilderAPI_MakeFace faceMaker(outerWire);
    faceMaker.Add(innerWire); // Добавляем отверстие в основную грань
    TopoDS_Face myFace = faceMaker.Face();

    gp_Vec extrusionVec(0, 0, 20); // Выдавливаем на 20 единиц по оси Z
    BRepPrimAPI_MakePrism prism(myFace, extrusionVec);

    shape = prism.Shape();
}

void Detail1::drawSketch(SketchWidget *sketch) {
    sketch->clear();
    sketch->addLine({{200, 200}, {200, 400}});
    sketch->addLine({{200, 400}, {400, 400}});
    sketch->addLine({{400, 400}, {400, 200}});
    sketch->addLine({{400, 200}, {300, 300}});
    sketch->addLine({{300, 300}, {200, 200}});

    sketch->addDimensionLine({{300, 300}, {200, 200}});
}
