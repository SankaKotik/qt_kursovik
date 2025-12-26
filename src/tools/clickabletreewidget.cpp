// clickabletreewidget.cpp
#include "clickabletreewidget.h"
#include <QApplication>
#include <QDebug>
#include <QMouseEvent>

TreeItem::TreeItem(ClickableTreeWidget* tree, const QString& text)
    : QTreeWidgetItem(tree, QStringList(text)) {}

TreeItem::TreeItem(TreeItem* parent, const QString& text)
    : QTreeWidgetItem(parent, QStringList(text)) {}

void TreeItem::setOnClickHandler(std::function<void()> handler) {
    if (auto tree = treeWidget()) {
        tree->registerClickHandler(this, handler);
    }
}

void TreeItem::setOnDoubleClickHandler(std::function<void()> handler) {
    if (auto tree = treeWidget()) {
        tree->registerDoubleClickHandler(this, handler);
    }
}

ClickableTreeWidget* TreeItem::treeWidget() const {
    return qobject_cast<ClickableTreeWidget*>(QTreeWidgetItem::treeWidget());
}

ClickableTreeWidget::ClickableTreeWidget(QWidget* parent)
    : QTreeWidget(parent),
      singleClickTimer(new QTimer(this)),
      pendingSingleClickItem(nullptr),
      isDoubleClick(false) {
    setColumnCount(1);
    setHeaderHidden(true);
    setEditTriggers(QAbstractItemView::NoEditTriggers);
    setSelectionMode(QAbstractItemView::NoSelection);
    setRootIsDecorated(true);
    setItemsExpandable(true);
    setFrameShape(QFrame::NoFrame);

    singleClickTimer->setSingleShot(true);
    connect(singleClickTimer, &QTimer::timeout, this, &ClickableTreeWidget::handleSingleClickTimeout);
}

TreeItem* ClickableTreeWidget::addItem(const QString& text, TreeItem* parent) {
    TreeItem* item;
    if (parent) {
        item = new TreeItem(parent, text);
    } else {
        item = new TreeItem(this, text);
    }
    return item;
}

void ClickableTreeWidget::registerClickHandler(QTreeWidgetItem* item, std::function<void()> handler) {
    if (item) {
        clickHandlers[item] = handler;
    }
}

void ClickableTreeWidget::registerDoubleClickHandler(QTreeWidgetItem* item, std::function<void()> handler) {
    if (item) {
        doubleClickHandlers[item] = handler;
    }
}

void ClickableTreeWidget::clear() {
    clickHandlers.clear();
    doubleClickHandlers.clear();
    QTreeWidget::clear();
}

void ClickableTreeWidget::mousePressEvent(QMouseEvent* event) {
    lastPressPosition = event->pos();
    QTreeWidget::mousePressEvent(event);
}

void ClickableTreeWidget::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton && 
        (event->pos() - lastPressPosition).manhattanLength() < QApplication::startDragDistance()) {
        QTreeWidgetItem* item = itemAt(event->pos());
        if (item && !isDoubleClick) {
            pendingSingleClickItem = item;
            singleClickTimer->start(QApplication::doubleClickInterval());
        }
    }
    isDoubleClick = false;
    QTreeWidget::mouseReleaseEvent(event);
}

void ClickableTreeWidget::mouseDoubleClickEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        isDoubleClick = true;
        singleClickTimer->stop();
        pendingSingleClickItem = nullptr;

        QTreeWidgetItem* item = itemAt(event->pos());
        if (item) {
            processDoubleClick(item);
        }
    }
    QTreeWidget::mouseDoubleClickEvent(event);
}

void ClickableTreeWidget::handleSingleClickTimeout() {
    if (pendingSingleClickItem) {
        processClick(pendingSingleClickItem);
        pendingSingleClickItem = nullptr;
    }
}

void ClickableTreeWidget::processClick(QTreeWidgetItem* item) {
    if (clickHandlers.contains(item)) {
        clickHandlers[item]();
    }
}

void ClickableTreeWidget::processDoubleClick(QTreeWidgetItem* item) {
    if (doubleClickHandlers.contains(item)) {
        doubleClickHandlers[item]();
    }
}