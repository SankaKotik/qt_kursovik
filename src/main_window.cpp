#include "main_window.h"
#include <QToolBar>
#include <QStatusBar>
#include <QMenuBar>
#include <QSplitter>
#include <QMessageBox>
#include <QButtonGroup>
#include <QRadioButton>
#include <QHBoxLayout>
#include <vector>

MainWindow::MainWindow(QWidget *parent)
: QMainWindow(parent)
{
    setWindowTitle("Дерево Qt6");
    setGeometry(100, 100, 1200, 800);

    createToolBars();
    setupUi();
}

MainWindow::~MainWindow() {}

void MainWindow::createToolBars()
{
    auto menuBar = new QMenuBar(nullptr);
    auto menu_file = menuBar->addMenu(tr("Файл"));
    auto menu_settings = menuBar->addMenu(tr("Настроить"));
    auto menu_help = menuBar->addMenu(tr("Справка"));

    menu_file->addAction(
        QIcon::fromTheme("document-close"), 
        "Закрыть",
        [this](){ this->close(); },
        Qt::CTRL + Qt::Key_Q
    );

    menu_settings->addAction(
        "Показать дерево", 
        [this](bool checked){
            tree->setHidden(!checked);
        }
    )->setCheckable(true);
    
    menu_help->addAction(
        QIcon::fromTheme("help-about"), 
        "О программе",
        [this](){
            QMessageBox::about(this, "О программе", 
                      "\n"
                      "Qt6 C++\n"
                      "Использует ");
        }
    );
    setMenuBar(menuBar);
}

void MainWindow::setupUi()
{
    QSplitter *hsplitter = new QSplitter(Qt::Horizontal, this);

    tree = new ClickableTreeWidget(this);
    auto root = tree->addItem("Сборка");
    root->setExpanded(true);

    auto detail = root->addItem("Деталь");
    auto det3d = root->addItem("3d");
    auto params = root->addItem("Параметры");
    

    QWidget *container = new QWidget(this);
    QGridLayout *container_layout = new QGridLayout(container);
    container_layout->setContentsMargins(0, 0, 0, 0);

    stackedWidget = new QStackedWidget(this);
    
    sketchWidget = new SketchWidget(this);
    glWidget = new GLWidget3D(this);

    detail->setOnDoubleClickHandler([this]() { 
        sketchWidget->clear();
        sketchWidget->addLine({{200, 200}, {200, 400}});
        sketchWidget->addLine({{200, 400}, {400, 400}});
        sketchWidget->addLine({{400, 400}, {400, 200}});
        sketchWidget->addLine({{400, 200}, {300, 300}});
        sketchWidget->addLine({{300, 300}, {200, 200}});

        sketchWidget->addDimensionLine({{300, 300}, {200, 200}});

        // sketchWidget->setShowGrid(true);

        qDebug()<<sketchWidget->findClosedContours();
    });

    det3d->setOnDoubleClickHandler([this](){
        

        cubeModel.initModel();
        cubeModel.generateMesh();
        // model.exportData();

        // std::array<vec3<float>, 3> triang;
        // triang = {vec3<float>(0,0,0), vec3<float>(0,1,0), vec3<float>(0,0,1)};
        // vertex.push_back(triang);
        glWidget->loadModel(&cubeModel.vertex);
        // glWidget->loadModel(&model.vertex);
        // for (auto tri: model.vertex) {
        //     for (auto pnt : tri) {
        //         qDebug() << pnt.x << pnt.y << pnt.z;
        //     }
        // }
    });

    params->setOnDoubleClickHandler([this](){
        auto parameter_selector = new ParameterSelectorDialog({
            {"12.", "23.", "32."},
            {"45.", "56.", "78."}
        });
        parameter_selector->setHeadings({"param1", "param2", "param3"});
        parameter_selector->exec();
        qDebug()<<parameter_selector->selectedParameters();
    });

    stackedWidget->addWidget(sketchWidget);
    stackedWidget->addWidget(glWidget);

    auto *overlay = new OverlayWidget();
    connect(overlay->modeSwitch, &ViewModeSwitch::modeChanged, this, [&](ViewModeSwitch::Mode newMode){
        stackedWidget->setCurrentIndex(newMode == ViewModeSwitch::Mode3D);
    });

    container_layout->addWidget(stackedWidget, 0, 0);
    container_layout->addWidget(overlay, 0, 0);
    
    hsplitter->addWidget(tree);
    hsplitter->addWidget(container);
    hsplitter->setSizes({
      (int)(width() * ((float)1 / 4)),
      (int)(width() * ((float)3 / 4)),
    });

    QSplitter *vsplitter = new QSplitter(Qt::Vertical, this);
    vsplitter->addWidget(hsplitter);
    
    vsplitter->setSizes({
        (int)(height() * ((float)3 / 4)),
        // (int)(height() * ((float)1 / 4)),
    });

    setCentralWidget(vsplitter);
    statusBar()->showMessage("Программа готова к работе", 5000);
}

void MainWindow::buildParamSelector() {
    
}

void MainWindow::buildSketch() {
    
}
