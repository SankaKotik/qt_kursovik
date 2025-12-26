// clickabletreewidget.h
#pragma once

#include <QTreeWidget>
#include <QTimer>
#include <QMap>
#include <functional>

class ClickableTreeWidget;

class TreeItem : public QTreeWidgetItem {
public:
    explicit TreeItem(ClickableTreeWidget* tree, const QString& text);
    explicit TreeItem(TreeItem* parent, const QString& text);
    
    void setOnClickHandler(std::function<void()> handler);
    void setOnDoubleClickHandler(std::function<void()> handler);
    template <typename... Args> TreeItem* addItem(Args... args);

private:
    ClickableTreeWidget* treeWidget() const;
};

class ClickableTreeWidget : public QTreeWidget {
    Q_OBJECT
public:
    explicit ClickableTreeWidget(QWidget* parent = nullptr);

    TreeItem* addItem(const QString& text, TreeItem* parent = nullptr);
    void registerClickHandler(QTreeWidgetItem* item, std::function<void()> handler);
    void registerDoubleClickHandler(QTreeWidgetItem* item, std::function<void()> handler);

    void clear() ;

protected:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;
    void mouseDoubleClickEvent(QMouseEvent* event) override;

private slots:
    void handleSingleClickTimeout();

private:
    QMap<QTreeWidgetItem*, std::function<void()>> clickHandlers;
    QMap<QTreeWidgetItem*, std::function<void()>> doubleClickHandlers;
    QTimer* singleClickTimer;
    QTreeWidgetItem* pendingSingleClickItem;
    bool isDoubleClick;
    QPoint lastPressPosition;

    void processClick(QTreeWidgetItem* item);
    void processDoubleClick(QTreeWidgetItem* item);
};

template <typename... Args>
TreeItem* TreeItem::addItem(Args... args) {
    return treeWidget()->addItem(args..., this);
}
