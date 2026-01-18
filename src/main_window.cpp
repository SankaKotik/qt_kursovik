#include "main_window.h"

#include <QToolBar>
#include <QMenuBar>
#include <QSplitter>
#include <QButtonGroup>
#include <QRadioButton>
#include <QHBoxLayout>
#include <QtConcurrent>
#include <QFuture>
#include <vector>

#include "Standard_ErrorHandler.hxx"

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
    auto menu_detail = menuBar->addMenu(tr("Деталь"));
    auto menu_settings = menuBar->addMenu(tr("Настроить"));
    auto menu_help = menuBar->addMenu(tr("Справка"));

    menu_file->addAction(
        QIcon::fromTheme("document-close"), 
        "Закрыть",
        [this](){ this->close(); },
        Qt::CTRL + Qt::Key_Q
    );
    
    menu_detail->addAction(
        QIcon::fromTheme("settings"),
        "Исполнение",
        this,
        &MainWindow::buildParamSelector
    );

    auto showTreeAction = menu_settings->addAction(
        "Показать дерево", 
        [this](bool checked){
            tree->setHidden(!checked);
        }
    );
    showTreeAction->setCheckable(true);
    showTreeAction->setChecked(true);
    
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

    auto detail = root->addItem("Полумуфта");
    detail->setOnClickHandler([this](){ selectModel<HalfCoupling>(); });

    auto detail2 = root->addItem("Звездочка");
    detail2->setOnClickHandler([this](){ selectModel<Sprocket>(); });

    QWidget *container = new QWidget(this);
    QGridLayout *container_layout = new QGridLayout(container);
    container_layout->setContentsMargins(0, 0, 0, 0);

    stackedWidget = new QStackedWidget(this);
    
    sketchWidget = new SketchWidget(this);
    glWidget = new GLWidget3D(this);

    stackedWidget->addWidget(sketchWidget);
    stackedWidget->addWidget(glWidget);

    overlay = new OverlayWidget();
    connect(overlay->modeSwitch, &ViewModeSwitch::modeChanged, this, [&](ViewModeSwitch::Mode newMode){
        stackedWidget->setCurrentIndex(newMode == ViewModeSwitch::Mode3D);
        updateView();
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
    if (currentModel) {
        QVector<QStringList> param_strings;
        std::transform(
            currentModel->params_table.begin(), 
            currentModel->params_table.end(), 
            std::back_inserter(param_strings), 
            [](const auto& v) { 
                QStringList sl; for(float f : v) sl << QString::number(f); return sl; 
            });

        auto parameter_selector = new ParameterSelectorDialog(
            param_strings, currentModel, paramSelectorPreview, this);
        parameter_selector->setHeadings(currentModel->params_table_headings);
        connect(parameter_selector, &ParameterSelectorDialog::modelUpdated, this, &MainWindow::updateView);
        parameter_selector->exec();
    }
}

void MainWindow::updateView() {
    if (currentModel) {
        setEnabled(false);
        if (overlay->modeSwitch->currentMode() == ViewModeSwitch::Mode2D) {
            sketchWidget->clear();
            currentModel->drawSketch(sketchWidget);
        } else {
            // Получаем фьючер
            auto future = QtConcurrent::run([this]() {
                // Обработка исключений при построении модели
                try {
                    OCC_CATCH_SIGNALS
                    currentModel->initModel3D();
                } catch (const Standard_Failure& theFailure) {
                    // Получаем текст ошибки и имя конкретного типа исключения
                    QMessageBox::critical(this, theFailure.DynamicType()->Name(), theFailure.GetMessageString());
                }
            });

            // Запускаем процесс в отдельном потоке
            future.then(this, [this]() {
                currentModel->generateMesh();
                glWidget->loadModel(&currentModel->vertex);
                setEnabled(true);
            });
        }
    }
}
