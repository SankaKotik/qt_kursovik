#include "parameter_selector.h"
#include <QHeaderView>

ParameterSelectorDialog::ParameterSelectorDialog(const QVector<QStringList>& data,
                                                 QWidget* parent)
    : QDialog(parent)
    , tableWidget(new QTableWidget(this))
{
    setWindowTitle("Выбор параметров");

    // Настройка таблицы
    tableWidget->setRowCount(static_cast<int>(data.size()));
    if (!data.isEmpty())
        tableWidget->setColumnCount(data.first().size());

    for (int row = 0; row < data.size(); ++row) {
        const QStringList& rowData = data[row];
        for (int col = 0; col < rowData.size(); ++col) {
            tableWidget->setItem(row, col, new QTableWidgetItem(rowData[col]));
        }
    }

    tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // Автоматическая подгонка ширины столбцов
    tableWidget->horizontalHeader()->setStretchLastSection(true);
    tableWidget->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);

    // Кнопки OK/Cancel
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ParameterSelectorDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &QDialog::reject);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tableWidget);
    layout->addWidget(buttonBox);

    setLayout(layout);
    resize(600, 400);
}

void ParameterSelectorDialog::onAccept()
{
    QModelIndexList selected = tableWidget->selectionModel()->selectedRows();
    if (!selected.isEmpty()) {
        int row = selected.first().row();
        selectedRowData.clear();
        for (int col = 0; col < tableWidget->columnCount(); ++col) {
            QTableWidgetItem* item = tableWidget->item(row, col);
            selectedRowData << (item ? item->text() : QString());
        }
        accept();
    } else {
        
    }
}

int ParameterSelectorDialog::selectedParameters() const
{
    return tableWidget->currentRow();
}

void ParameterSelectorDialog::setHeadings(const QStringList& headings) {
    if (tableWidget->columnCount() == headings.count()) {
        tableWidget->setHorizontalHeaderLabels(headings);
    }
}
