#include "model.h"

#include <TopoDS_Shape.hxx>
#include <TopoDS_Face.hxx>
#include <TopoDS_Edge.hxx>
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
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>
#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>
#include "TopTools_ListOfShape.hxx"
#include <BRepAdaptor_Curve.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>
#include <GC_MakeArcOfCircle.hxx>

ModelNotifier::ModelNotifier(QObject *parent)
    : QObject()
{
    
}

void Model::generateMesh()
{
    vertex.clear();
    if (shape.IsNull()) return;

    // Триангуляция
    BRepMesh_IncrementalMesh mesh(shape, 0.1);
    if (mesh.IsDone()) {
        if (notifier) {
            emit notifier->statusChanged("Сетка успешно создана и сохранена внутри shape");
        }
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
    if (notifier) {
        emit notifier->statusChanged("Обновление модели...");
    }

    double mkr = params_table[selectedParameters][0];
    double l3, l2, chamferDistance;
    if (mkr <= 6.3f) {
        l2 = 16.0f;
        l3 = 10.5f;
        chamferDistance = 1.0f;
    } else {
        l2 = params_table[selectedParameters][12];
        l3 = params_table[selectedParameters][13];
        chamferDistance = 1.6f;
    }

    double l = params_table[selectedParameters][selectedExecution == 1 ? 8 : 9];
    double d1 = params_table[selectedParameters][6];
    double D = params_table[selectedParameters][7];
    double d = params_table[selectedParameters][1];
    if (d == 0) {
        if (notifier) { notifier->warningIssued("1-й ряд является предпочтительным."); }
        d = params_table[selectedParameters][2];
    }
    double dt1 = params_table[selectedParameters][selectedExecution == 1 || mkr <= 6.3f ? 3 : 4];
    double b = params_table[selectedParameters][5];
    double B1 = params_table[selectedParameters][15];
    double B = params_table[selectedParameters][14];
    double r = params_table[selectedParameters][16];
    
    if (!(l2 && l3 && l && d1 && D && d && dt1 && b && B1 && B && r)) {
        if (notifier) { notifier->errorOccurred("Выбранного исполнения не существует!"); }
        return;
    }
    
    // Вращение
    TopoDS_Shape revolvedSolid;
    {
        gp_Pnt p1(0, 0, 0),
        p2(l, 0, 0),
        p3(l, d1 / 2., 0),
        p4(l2 - l3 - 1., d1 / 2., 0),
        p5(l2 - l3 - 1., D / 2., 0),
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
        
        TopoDS_Wire outerWire = polyMaker.Wire();

        // Превращаем Wire в Face, чтобы получить Solid
        TopoDS_Face face = BRepBuilderAPI_MakeFace(outerWire);
        
        // Ось вращения (Z)
        gp_Ax1 axis(gp_Pnt(0, 0, 0), gp_Dir(1, 0, 0));
        
        // Создаем тело вращения на 360 градусов (2*PI)
        BRepPrimAPI_MakeRevol revolMaker(face, axis);
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
        rotation.SetRotation(gp_Ax1(sketchSystem.Location(), sketchSystem.Direction()), (mkr <= 6.3f ? 45.0 : 30.0) * M_PI / 180.0);
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

    // Булева: Вращение - Выдавливание
    TopoDS_Shape cutShape;
    {
        BRepAlgoAPI_Cut cutMaker(revolvedSolid, extrusionSolid);
        cutMaker.Build();
        cutShape = cutMaker.Shape();
    }

    // Фаска
    TopoDS_Shape chamferSolid;
    {
        BRepFilletAPI_MakeChamfer mkChamfer(cutShape);

        // Перебор всех ребер в объекте
        for (TopExp_Explorer ex(cutShape, TopAbs_EDGE); ex.More(); ex.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(ex.Current());

            // Адаптор позволяет "заглянуть" внутрь геометрии ребра
            BRepAdaptor_Curve adaptor(edge);

            // Проверяем тип кривой
            if (adaptor.GetType() == GeomAbs_Circle) {
                // Это ребро — окружность или дуга окружности
                
                // Теперь можно получить геометрические параметры окружности
                gp_Circ circle = adaptor.Circle();
                Standard_Real radius = circle.Radius();
                gp_Pnt center = circle.Location();

                // Если окружность не на лицевой части муфты
                if (center.X() > 0) {
                    mkChamfer.Add(chamferDistance, edge);
                }
            }
        }

        mkChamfer.Build();
        chamferSolid = mkChamfer.Shape();
    }

    // Скругление
    TopoDS_Shape filletSolid;
    {
        BRepFilletAPI_MakeFillet mkFillet(chamferSolid);

        // Проходимся по ребрам
        for (TopExp_Explorer ex(chamferSolid, TopAbs_EDGE); ex.More(); ex.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(ex.Current());

            BRepAdaptor_Curve adaptor(edge);

            // Кривая должна быть прямой линией
            if (adaptor.GetType() == GeomAbs_Line) {                
                gp_Lin line = adaptor.Line();
                Standard_Real f = adaptor.FirstParameter();
                Standard_Real l = adaptor.LastParameter();

                gp_Pnt startPoint = adaptor.Value(f); // Точка начала
                gp_Pnt endPoint = adaptor.Value(l);   // Точка конца

                // Оставляем только прямые, параллельные оси X
                if (abs(startPoint.Z() - endPoint.Z()) < 0.001 && 
                abs(startPoint.Y() - endPoint.Y()) < 0.001) {
                    double distance = vec2(startPoint.Z(), startPoint.Y()).length();
                    double neededDistance = vec2(b / 2., dt1 - d / 2).length();
                    if (abs(distance - neededDistance) < 0.001) {
                        mkFillet.Add(r, edge);
                    }
                }
            }
        }

        mkFillet.Build();
        filletSolid = mkFillet.Shape();
    }

    // Выдавливание 2
    TopoDS_Shape couplingTooth, toothMount;
    {
        bool isTriangleTooth = false; // зуб является треугольным, а не трапециевидным
        gp_Pnt outer_origin(0, 0, 0);
        gp_Dir normal(-1, 0, 0);
        gp_Ax3 outer_sketchSystem(outer_origin, normal);
        
        // точки 2d
        gp_Pnt2d outer_sp1, outer_sp2, outer_sp3, outer_sp4;
        gp_Pnt2d inner_sp1, inner_sp2, inner_sp3, inner_sp4;
        if (mkr <= 6.3f) {
            double outer_p23_oneofXY = (B1 / 2) * sqrt(2);
            outer_sp1 = gp_Pnt2d(0, D / 2),
            outer_sp2 = gp_Pnt2d(0, outer_p23_oneofXY),
            outer_sp3 = gp_Pnt2d(outer_p23_oneofXY, 0),
            outer_sp4 = gp_Pnt2d(D / 2, 0);

            double inner_circlePointY = sqrt(pow(D / 2, 2) - pow(B, 2));
            double inner_p23_oneofXY = outer_p23_oneofXY - B;
            inner_sp1 = gp_Pnt2d(B, inner_circlePointY),
            inner_sp2 = gp_Pnt2d(B, inner_p23_oneofXY),
            inner_sp3 = gp_Pnt2d(inner_p23_oneofXY, B),
            inner_sp4 = gp_Pnt2d(inner_circlePointY, B);
        } else {
            gp_Pnt2d center(0.0, 0.0);
            outer_sp1 = gp_Pnt2d(0, D / 2),
            outer_sp2 = gp_Pnt2d(0, B1 * cos(M_PI / 6)),
            outer_sp3 = outer_sp2.Rotated(center, -M_PI / 3),
            outer_sp4 = outer_sp1.Rotated(center, -M_PI / 3);

            inner_sp1 = gp_Pnt2d(B, sqrt(pow(D / 2, 2) - pow(B, 2))),
            inner_sp2 = gp_Pnt2d(B, (B-outer_sp2.X()) * (outer_sp3.Y()-outer_sp2.Y()) / (outer_sp3.X()-outer_sp2.X()) + outer_sp2.Y());

            gp_Dir2d direction(cos(M_PI / 3), sin(M_PI / 3));
            gp_Ax2d mirrorAxis(gp_Pnt2d(0, 0), direction);

            inner_sp3 = inner_sp2.Mirrored(mirrorAxis),
            inner_sp4 = inner_sp1.Mirrored(mirrorAxis);

            // Некоторые исполнения заставляют точки p2 и p3 слиться в одну
            // (p2 лежит на оси зеркалирования). Обработаем такой случай отдельно
            if (inner_sp2.IsEqual(inner_sp3, 0.001)) {
                isTriangleTooth = true;
            }
        }
        
        {
            gp_Trsf rotation;
            rotation.SetRotation(gp_Ax1(outer_sketchSystem.Location(), outer_sketchSystem.Direction()), mkr <= 6.3f ? -M_PI / 2 : M_PI / 3);
            outer_sketchSystem.Transform(rotation);
        }

        // Создаем плоскость эскиза
        gp_Pln outer_sketchPlane(outer_sketchSystem);
        
        // Преобразуем в 3D (вычисляет 3D-координату на основе UV-параметров поверхности)
        gp_Pnt outer_p1 = ElSLib::Value(outer_sp1.X(), outer_sp1.Y(), outer_sketchPlane);
        gp_Pnt outer_p2 = ElSLib::Value(outer_sp2.X(), outer_sp2.Y(), outer_sketchPlane);
        gp_Pnt outer_p3 = ElSLib::Value(outer_sp3.X(), outer_sp3.Y(), outer_sketchPlane);
        gp_Pnt outer_p4 = ElSLib::Value(outer_sp4.X(), outer_sp4.Y(), outer_sketchPlane);

        gp_Pnt inner_p1 = ElSLib::Value(inner_sp1.X(), inner_sp1.Y(), outer_sketchPlane);
        gp_Pnt inner_p2 = ElSLib::Value(inner_sp2.X(), inner_sp2.Y(), outer_sketchPlane);
        gp_Pnt inner_p3 = ElSLib::Value(inner_sp3.X(), inner_sp3.Y(), outer_sketchPlane);
        gp_Pnt inner_p4 = ElSLib::Value(inner_sp4.X(), inner_sp4.Y(), outer_sketchPlane);
        
        TopoDS_Edge outer_e1 = BRepBuilderAPI_MakeEdge(outer_p1, outer_p2);
        TopoDS_Edge outer_e2 = BRepBuilderAPI_MakeEdge(outer_p2, outer_p3);
        TopoDS_Edge outer_e3 = BRepBuilderAPI_MakeEdge(outer_p3, outer_p4);

        TopoDS_Edge inner_e1 = BRepBuilderAPI_MakeEdge(inner_p1, inner_p2);
        TopoDS_Edge inner_e2 = BRepBuilderAPI_MakeEdge(inner_p2, inner_p3);
        TopoDS_Edge inner_e3 = BRepBuilderAPI_MakeEdge(inner_p3, inner_p4);
        
        // Окружность для дуги
        gp_Pnt center(0, 0, 0);
        gp_Ax2 circleAx(center, gp_Dir(1, 0, 0));
        gp_Circ circle(circleAx, D / 2);
        
        // Дуга по окружности и двум точкам
        GC_MakeArcOfCircle outer_arcMaker(circle, outer_p1, outer_p4, true); 
        TopoDS_Edge outer_e4 = BRepBuilderAPI_MakeEdge(outer_arcMaker.Value());

        GC_MakeArcOfCircle inner_arcMaker(circle, inner_p1, inner_p4, true); 
        TopoDS_Edge inner_e4 = BRepBuilderAPI_MakeEdge(inner_arcMaker.Value());
        
        BRepBuilderAPI_MakeWire outer_wireMaker;
        outer_wireMaker.Add(outer_e1); outer_wireMaker.Add(outer_e2); 
        outer_wireMaker.Add(outer_e3); outer_wireMaker.Add(outer_e4);
        TopoDS_Wire outerWire = outer_wireMaker.Wire();

        BRepBuilderAPI_MakeWire inner_wireMaker;
        inner_wireMaker.Add(inner_e1);
        if (!isTriangleTooth) {
            inner_wireMaker.Add(inner_e2);
        }
        inner_wireMaker.Add(inner_e3);
        inner_wireMaker.Add(inner_e4);
        TopoDS_Wire innerWire = inner_wireMaker.Wire();
        
        TopoDS_Face outer_face = BRepBuilderAPI_MakeFace(outerWire);
        TopoDS_Face inner_face = BRepBuilderAPI_MakeFace(innerWire);
        
        gp_Vec extrusionVec(-1, 0, 0);
        gp_Vec extrusionVec2(-l3, 0, 0);
        BRepPrimAPI_MakePrism outer_prism(outer_face, extrusionVec);
        BRepPrimAPI_MakePrism inner_prism(inner_face, extrusionVec2);
        
        toothMount = outer_prism.Shape();
        couplingTooth = inner_prism.Shape();
    }

    // Булева
    TopoDS_Shape booleanShape;
    {
        // Добавление
        TopTools_ListOfShape arguments;
        TopTools_ListOfShape tools;
        arguments.Append(filletSolid);
        tools.Append(toothMount);
        tools.Append(couplingTooth);

        // Копирование вращением
        {
            // сколько раз вращаем
            int rotCount = mkr <= 6.3f ? 2 : 3;
            // ось вращения
            gp_Ax1 rotationAxis(gp_Pnt(0, 0, 0), gp_Dir(-1, 0, 0));

            double angle = 2 * M_PI / rotCount;

            for (int i = 1; i <= rotCount; ++i) {
                gp_Trsf trans;
                trans.SetRotation(rotationAxis, angle * i);
                BRepBuilderAPI_Transform mount_transformer(toothMount, trans);
                BRepBuilderAPI_Transform tooth_transformer(couplingTooth, trans);
                tools.Append(mount_transformer.Shape());
                tools.Append(tooth_transformer.Shape());
            }
        }
        
        BRepAlgoAPI_Fuse fuseMaker;
        fuseMaker.SetArguments(arguments);
        fuseMaker.SetTools(tools);
        fuseMaker.Build();
        
        booleanShape = fuseMaker.Shape();
    }

    // Фаска
    TopoDS_Shape toothChamfer;
    {
        BRepFilletAPI_MakeChamfer mkChamfer(booleanShape);

        // Перебор всех ребер в объекте
        for (TopExp_Explorer ex(booleanShape, TopAbs_EDGE); ex.More(); ex.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(ex.Current());
            BRepAdaptor_Curve adaptor(edge);

            if (adaptor.GetType() == GeomAbs_Circle) {
                gp_Circ circle = adaptor.Circle();
                Standard_Real radius = circle.Radius();
                gp_Pnt center = circle.Location();

                if (center.X() < -1.) {
                    mkChamfer.Add(chamferDistance, edge);
                }
            }
        }

        mkChamfer.Build();
        chamferSolid = mkChamfer.Shape();
    }
    
    shape = chamferSolid;
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
    
    // sketch->addLine({p1, p2});
    // sketch->addLine({p2, p3});
    // sketch->addLine({p3, p4});
    // sketch->addLine({p4, p5});
    // sketch->addLine({p5, p6});
    // sketch->addLine({p6, p1});

    sketch->addLine({{5, 52.7636}, {5, 9.2376}});
    sketch->addLine({{5, 9.2376}, {-5.5, -8.94893}});
    sketch->addLine({{-5.5, -8.94893}, {-43.1946, -30.7119}});
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

void Sprocket::initModel3D() {
    double mkr = params_table[selectedParameters][0];
    double D = params_table[selectedParameters][1];
    double d = params_table[selectedParameters][2];
    double B = params_table[selectedParameters][3];
    double H = mkr <= 6.3f ? 10.5f : params_table[selectedParameters][4];
    double r = params_table[selectedParameters][5];

    // Количество лапок
    double n = mkr <= 6.3f ? 4 : 6;

    // Выдавливание
    TopoDS_Shape extrusionSolid;
    {
        double angleStep = 2 * M_PI / n;
        double halfWidth = B / 2;
        // Полилиния
        BRepBuilderAPI_MakePolygon polyMaker;
        for (int i = 0; i < n; ++i) {
            double angle = i * angleStep;  // текущий угол луча
            mat2<double> rotation = mat2<double>::rotate(angle);
            
            vec2<double> p1 = rotation * vec2(B / 2, D);
            vec2<double> p2 = rotation * vec2(-B / 2, D);

            // с учетом будущего скругления
            vec2<double> p3 = rotation * vec2(
                -B / 2,
                mkr <= 6.3f ? B / 2 :
                (B / 2) / tan(M_PI / 6)
            );

            polyMaker.Add(gp_Pnt(p1.x, p1.y, 0));
            polyMaker.Add(gp_Pnt(p2.x, p2.y, 0));
            polyMaker.Add(gp_Pnt(p3.x, p3.y, 0));
        }
        polyMaker.Close();
        
        TopoDS_Wire outerWire = polyMaker.Wire();

        // Превращаем Wire в Face, чтобы получить Solid
        TopoDS_Face face = BRepBuilderAPI_MakeFace(outerWire);
        
        gp_Vec extrusionVec(0, 0, H); 
        BRepPrimAPI_MakePrism prismMaker(face, extrusionVec);
        extrusionSolid = prismMaker.Shape();
    }

    // Выдавливание окружности
    TopoDS_Shape extrusionSolid2;
    {
        gp_Circ circleGeom(
            gp_Ax2(
                gp_Pnt(0, 0, 0), 
                gp_Dir(0, 0, 1)
            ),
            D / 2
        );

        TopoDS_Edge circleEdge = BRepBuilderAPI_MakeEdge(circleGeom);
        TopoDS_Wire circleWire = BRepBuilderAPI_MakeWire(circleEdge);
        TopoDS_Face circleFace = BRepBuilderAPI_MakeFace(circleWire);

        gp_Vec extrusionVec(0, 0, H);
        BRepPrimAPI_MakePrism prismMaker(circleFace, extrusionVec);
        extrusionSolid2 = prismMaker.Shape();
    }

    // Булева: пересечение
    TopoDS_Shape booleanSolid;
    {
        BRepAlgoAPI_Common commonMaker(extrusionSolid, extrusionSolid2);
        booleanSolid = commonMaker.Shape();
    }

    // Скругление
    TopoDS_Shape filletSolid;
    {
        BRepFilletAPI_MakeFillet mkFillet(booleanSolid);

        // Проходимся по ребрам
        for (TopExp_Explorer ex(booleanSolid, TopAbs_EDGE); ex.More(); ex.Next()) {
            TopoDS_Edge edge = TopoDS::Edge(ex.Current());

            BRepAdaptor_Curve adaptor(edge);

            // Кривая должна быть прямой линией
            if (adaptor.GetType() == GeomAbs_Line) {                
                gp_Lin line = adaptor.Line();
                Standard_Real f = adaptor.FirstParameter();
                Standard_Real l = adaptor.LastParameter();

                gp_Pnt startPoint = adaptor.Value(f); // Точка начала
                gp_Pnt endPoint = adaptor.Value(l);   // Точка конца

                // Оставляем только прямые, параллельные оси Z
                if (abs(startPoint.X() - endPoint.X()) < 0.001 && 
                abs(startPoint.Y() - endPoint.Y()) < 0.001) {
                    double distance = vec2(startPoint.X(), startPoint.Y()).length();
                    if (mkr <= 6.3f) {
                        // 1е исполнение: проверяем координаты по отдельности
                        if (abs(abs(startPoint.X()) - B / 2) < 0.001 && abs(abs(startPoint.Y()) - B / 2) < 0.001) {
                            mkFillet.Add(r, edge);
                        }
                    } else {
                        // 2е исполнение: проверяем расстояние до точки
                        if (distance < d / 2) {
                            mkFillet.Add(r, edge);
                        }
                    }
                }
            }
        }

        mkFillet.Build();
        filletSolid = mkFillet.Shape();
    }

    shape = filletSolid;
}

void Sprocket::drawSketch(SketchWidget *sketch) {
    double mkr = params_table[selectedParameters][0];
    double D = params_table[selectedParameters][1];
    double d = params_table[selectedParameters][2];
    double B = params_table[selectedParameters][3];
    double H = params_table[selectedParameters][4];
    double r = params_table[selectedParameters][5];

    // Количество лапок
    double n = mkr <= 6.3f ? 4 : 6;
    {
        double angleStep = 2 * M_PI / n;
        double halfWidth = B / 2;

        vec2<double> first_p, old_p;
        for (int i = 0; i < n; ++i) {
            double angle = i * angleStep;  // текущий угол луча
            mat2<double> rotation = mat2<double>::rotate(angle);
            
            vec2<double> p1 = rotation * vec2(B / 2, D);
            vec2<double> p2 = rotation * vec2(-B / 2, D);

            // с учетом будущего скругления
            vec2<double> p3 = rotation * vec2(
                -B / 2,
                mkr <= 6.3f ? B / 2 :
                (B / 2) / tan(M_PI / 6)
            );

            sketch->addLine({{p1.x, p1.y}, {p2.x, p2.y}});
            sketch->addLine({{p2.x, p2.y}, {p3.x, p3.y}});
            if (i != 0) {
                sketch->addLine({{old_p.x, old_p.y}, {p1.x, p1.y}});
            } 
            if (i == 0) {
                first_p = p1;
            } else if (i == n - 1) {
                sketch->addLine({{first_p.x, first_p.y}, {p3.x, p3.y}});
            }
            old_p = p3;
        }
    }
}

void Assembly::initModel3D() {
    // Строим модели полумуфты и звездочки
    auto coupling = new HalfCoupling;
    coupling->initModel3D();
    auto sprocket = new Sprocket;
    sprocket->initModel3D();

    TopoDS_Compound assembly_res;
    BRep_Builder builder;
    builder.MakeCompound(assembly_res);
    builder.Add(assembly_res, coupling->shape);
    builder.Add(assembly_res, sprocket->shape);

    shape = assembly_res;
}

void Assembly::drawSketch(SketchWidget *sketch) {
    
}
