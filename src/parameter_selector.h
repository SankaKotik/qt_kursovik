#pragma once

#include <QDialog>
#include <QTableWidget>
#include <QVBoxLayout>
#include <QDialogButtonBox>
#include <QVector>
#include <QStringList>

class ParameterSelectorDialog : public QDialog
{
    Q_OBJECT

public:
    explicit ParameterSelectorDialog(const QVector<QStringList>& data,
                                    QWidget* parent = nullptr);

    int selectedParameters() const;
    void setHeadings(const QStringList& headings);

private slots:
    void onAccept();

private:
    QTableWidget* tableWidget;
    QStringList selectedRowData;
};
