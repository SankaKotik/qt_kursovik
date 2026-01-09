#include "parameter_selector.h"
#include <QHeaderView>

ParameterSelectorDialog::ParameterSelectorDialog(const QVector<QStringList>& data,
                                                int &selectedParameters, bool &modal,
                                                QWidget* parent)
    : QDialog(parent),
    selectedParameters(selectedParameters),
    modal(modal)
    , tableWidget(new QTableWidget(this))
{
    selectedBefore = selectedParameters;
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

    tableWidget->selectRow(selectedParameters);

    QLayout *buttonBoxLayout = new QHBoxLayout();
    modality = new QCheckBox("Предпросмотр");
    modality->setChecked(modal);

    // Кнопки OK/Cancel
    QDialogButtonBox* buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
    connect(buttonBox, &QDialogButtonBox::accepted, this, &ParameterSelectorDialog::onAccept);
    connect(buttonBox, &QDialogButtonBox::rejected, this, &ParameterSelectorDialog::onReject);
    buttonBoxLayout->addWidget(modality);
    buttonBoxLayout->addWidget(buttonBox);

    connect(tableWidget, &QTableWidget::itemSelectionChanged, this, [this](){
        if (modality->isChecked()) {
            this->selectedParameters = tableWidget->currentRow();
            emit modelUpdated();
        }
    });

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tableWidget);
    layout->addLayout(buttonBoxLayout);

    setLayout(layout);
    resize(600, 400);
}

void ParameterSelectorDialog::onAccept()
{
    QModelIndexList selected = tableWidget->selectionModel()->selectedRows();
    if (!selected.isEmpty() && !modality->isChecked()) {
        selectedParameters = tableWidget->currentRow();
        emit modelUpdated();
    }
    modal = modality->isChecked();
    accept();
}

void ParameterSelectorDialog::onReject() {
    selectedParameters = selectedBefore;
    emit modelUpdated();
    reject();
}

void ParameterSelectorDialog::setHeadings(const QStringList& headings) {
    if (tableWidget->columnCount() == headings.count()) {
        tableWidget->setHorizontalHeaderLabels(headings);
    }
}
