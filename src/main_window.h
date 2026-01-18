#pragma once

#include <QMainWindow>
#include <QStackedWidget>
#include <QStatusBar>
#include <QMessageBox>
#include <QProgressBar>
#include <QCoreApplication>
#include "gl_widget.h"
#include "sketch_widget.h"
#include "tools/clickabletreewidget.h"
#include "overlay_widget.h"
#include "stl_serializer.h"
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
    void updateView();

    template <typename T>
    void selectModel() {
        if (currentModel) {
            delete currentModel;
        }
        currentModel = new T();
        ModelNotifier* modelBridge = new ModelNotifier(this);
        currentModel->notifier = modelBridge;

        connect(modelBridge, &ModelNotifier::statusChanged, this, [this](const QString &msg) {
            statusBar()->showMessage(msg);
        });
        
        connect(modelBridge, &ModelNotifier::errorOccurred, this, [this](const QString &msg) {
            QMessageBox::critical(this, "Ошибка построения модели", msg);
        });

        connect(modelBridge, &ModelNotifier::warningIssued, this, [this](const QString &msg) {
            QMessageBox::information(this, "Обратите внимание", msg);
        });
        
        updateView();
    };

    ClickableTreeWidget *tree;
    QStackedWidget *stackedWidget;
    GLWidget3D *glWidget;
    SketchWidget *sketchWidget;
    OverlayWidget *overlay;
    QAction *saveStlAct;
    bool paramSelectorPreview = true;
    Model *currentModel = nullptr;
};
