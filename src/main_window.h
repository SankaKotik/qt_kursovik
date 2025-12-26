#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include "gl_widget.h"
#include "sketch_widget.h"
#include "tools/clickabletreewidget.h"
#include "overlay_widget.h"
#include "parameter_selector.h"
#include "model.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    void createToolBars();
    void setupUi();

    void buildParamSelector();
    void buildSketch();

    ClickableTreeWidget *tree;
    QStackedWidget *stackedWidget;
    GLWidget3D *glWidget;
    SketchWidget *sketchWidget;
    Cube cubeModel;
};
