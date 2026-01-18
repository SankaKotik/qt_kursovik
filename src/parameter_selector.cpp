#include "parameter_selector.h"
#include <QHeaderView>

ParameterSelectorDialog::ParameterSelectorDialog(const QVector<QStringList>& data,
                                                Model* &modelRef, bool &modal,
                                                QWidget* parent)
    : QDialog(parent),
    m_modelRef(modelRef),
    modal(modal)
    , tableWidget(new QTableWidget(this))
{
    selectedBefore = m_modelRef->selectedParameters;
    selectedExecutionBefore = m_modelRef->selectedExecution;
    setWindowTitle("Выбор параметров");

    QGroupBox *groupBox = new QGroupBox("Исполнение", this);
    {
        QRadioButton *radio1 = new QRadioButton("Исполнение 1");
        QRadioButton *radio2 = new QRadioButton("Исполнение 2");

        executionSelection = new QButtonGroup(this);
        executionSelection->addButton(radio1, 1); // ID 1
        executionSelection->addButton(radio2, 2); // ID 2

        QHBoxLayout *hbox = new QHBoxLayout;
        hbox->addWidget(radio1);
        hbox->addWidget(radio2);
        groupBox->setLayout(hbox);

        if (m_modelRef->selectedExecution == 1) {
            radio1->setChecked(true);
        } else {
            radio2->setChecked(true);
        }

        connect(executionSelection, &QButtonGroup::idToggled, [this](int id, bool checked) {
            if (modality->isChecked() && checked) {
                m_modelRef->selectedExecution = id;
                emit modelUpdated();
            }
        });
    }

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

    tableWidget->selectRow(m_modelRef->selectedParameters);

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
            this->m_modelRef->selectedParameters = tableWidget->currentRow();
            emit modelUpdated();
        }
    });

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(tableWidget);
    layout->addWidget(groupBox);
    layout->addLayout(buttonBoxLayout);

    setLayout(layout);
    resize(600, 400);
}

void ParameterSelectorDialog::onAccept()
{
    bool modelIsDirty = false;
    QModelIndexList selected = tableWidget->selectionModel()->selectedRows();
    if (!selected.isEmpty() && m_modelRef->selectedParameters != tableWidget->currentRow()) {
        m_modelRef->selectedParameters = tableWidget->currentRow();
        modelIsDirty = true;
    }
    if (m_modelRef->selectedExecution != executionSelection->checkedId()) {
        m_modelRef->selectedExecution = executionSelection->checkedId();
        modelIsDirty = true;
    }
    if (modelIsDirty) {
        emit modelUpdated();
    }
    modal = modality->isChecked();
    accept();
}

void ParameterSelectorDialog::onReject() {
    bool modelIsDirty = false;
    if (m_modelRef->selectedParameters != selectedBefore) {
        m_modelRef->selectedParameters = selectedBefore;
        modelIsDirty = true;
    }
    if (m_modelRef->selectedExecution != selectedExecutionBefore) {
        m_modelRef->selectedExecution = selectedExecutionBefore;
        modelIsDirty = true;
    }
    if (modelIsDirty) {
        emit modelUpdated();
    }
    reject();
}

void ParameterSelectorDialog::setHeadings(const QStringList& headings) {
    if (tableWidget->columnCount() == headings.count()) {
        tableWidget->setHorizontalHeaderLabels(headings);
    }
}
