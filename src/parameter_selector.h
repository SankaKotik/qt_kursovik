#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QVector>
#include <QStringList>
#include <QCheckBox>

class ParameterSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterSelectorDialog(const QVector<QStringList>& data,
                                    int &selectedParameters, bool &modal,
                                    QWidget* parent = nullptr);

    int &selectedParameters;
    int selectedBefore;
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
    QCheckBox* modality;
};
