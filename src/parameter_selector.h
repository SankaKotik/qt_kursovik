#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QVector>
#include <QStringList>
#include <QCheckBox>
#include <QRadioButton>
#include <QButtonGroup>
#include <QGroupBox>
#include "model.h"

class ParameterSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterSelectorDialog(const QVector<QStringList>& data,
                                    Model* &modelRef, bool &modal,
                                    QWidget* parent = nullptr);

    int selectedBefore;
    int selectedExecutionBefore;
    bool &modal;
    void setHeadings(const QStringList& headings);
    void setModality(bool &modal);

signals:
    void modelUpdated();

private slots:
    void onAccept();
    void onReject();

private:
    QTableWidget* tableWidget;
    QButtonGroup *executionSelection;
    QCheckBox* modality;
    Model* &m_modelRef;
};
