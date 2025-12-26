#ifndef VIEWMODESWITCH_H
#define VIEWMODESWITCH_H

#include <QWidget>
#include <QPushButton>
#include <QHBoxLayout>
#include <QPropertyAnimation>

class ViewModeSwitch : public QWidget
{
    Q_OBJECT

public:
    explicit ViewModeSwitch(QWidget *parent = nullptr);
    ~ViewModeSwitch();

    enum Mode {
        Mode2D,
        Mode3D
    };

    Mode currentMode() const;

signals:
    void modeChanged(Mode newMode);

private slots:
    void onButtonClicked(int mode);

private:
    QPushButton *m_button2D;
    QPushButton *m_button3D;
    QHBoxLayout *m_layout;
    QWidget *m_activeBg;
    QPropertyAnimation *m_animation;
    Mode m_currentMode;

    void updateActiveBgPosition(bool animated = true);
    void resizeEvent(QResizeEvent *event);
};

#endif // VIEWMODESWITCH_H