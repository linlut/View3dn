#ifndef BASEGENGRAPHFORM_H
#define BASEGENGRAPHFORM_H

#include <qvariant.h>


#include <Qt3Support/Q3ButtonGroup>
#include <Qt3Support/Q3GroupBox>
#include <QtCore/QVariant>
#include <QtGui/QAction>
#include <QtGui/QApplication>
#include <QtGui/QButtonGroup>
#include <QtGui/QCheckBox>
#include <QtGui/QComboBox>
#include <QtGui/QDialog>
#include <QtGui/QGridLayout>
#include <QtGui/QHBoxLayout>
#include <QtGui/QLabel>
#include <QtGui/QLineEdit>
#include <QtGui/QPushButton>
#include <QtGui/QRadioButton>
#include <QtGui/QSpacerItem>
#include <QtGui/QVBoxLayout>


class Ui_BaseGenGraphForm
{
public:
    QVBoxLayout *vboxLayout;
    QGridLayout *gridLayout;
    QLabel *filenameLabel;
    QPushButton *browseButton;
    QLineEdit *filename;
    QHBoxLayout *hboxLayout;
    Q3ButtonGroup *layoutGroup;
    QVBoxLayout *vboxLayout1;
    QRadioButton *layoutDirV;
    QRadioButton *layoutDirH;
    QRadioButton *layoutSpring;
    QRadioButton *layoutRadial;
    Q3GroupBox *outputsBox;
    QVBoxLayout *vboxLayout2;
    QCheckBox *genPNG;
    QCheckBox *genPS;
    QCheckBox *genFIG;
    QCheckBox *genSVG;
    QHBoxLayout *hboxLayout1;
    QLabel *presetLabel;
    QComboBox *presetFilter;
    Q3GroupBox *filterBox;
    QGridLayout *gridLayout1;
    QCheckBox *showSolvers;
    QCheckBox *showMechanicalStates;
    QCheckBox *showForceFields;
    QCheckBox *showInteractionForceFields;
    QCheckBox *showConstraints;
    QCheckBox *showMass;
    QCheckBox *showMechanicalMappings;
    QCheckBox *showTopology;
    QCheckBox *showMappings;
    QCheckBox *showCollisionModels;
    QCheckBox *showVisualModels;
    QCheckBox *showCollisionPipeline;
    QCheckBox *showBehaviorModels;
    QCheckBox *showContext;
    QCheckBox *showObjects;
    QCheckBox *showNodes;
    QHBoxLayout *hboxLayout2;
    Q3GroupBox *groupNodeLabel;
    QVBoxLayout *vboxLayout3;
    QCheckBox *labelNodeName;
    QCheckBox *labelNodeClass;
    Q3GroupBox *groupObjectLabel;
    QVBoxLayout *vboxLayout4;
    QCheckBox *labelObjectName;
    QCheckBox *labelObjectClass;
    QHBoxLayout *hboxLayout3;
    QSpacerItem *spacerItem;
    QPushButton *displayButton;
    QPushButton *exportButton;
    QPushButton *closeButton;

    void setupUi(QDialog *BaseGenGraphForm)
    {
    BaseGenGraphForm->setObjectName(QString::fromUtf8("BaseGenGraphForm"));
    BaseGenGraphForm->resize(QSize(289, 589).expandedTo(BaseGenGraphForm->minimumSizeHint()));
    BaseGenGraphForm->setWindowIcon(icon(image0_ID));
    vboxLayout = new QVBoxLayout(BaseGenGraphForm);
    vboxLayout->setSpacing(6);
    vboxLayout->setMargin(11);
    vboxLayout->setObjectName(QString::fromUtf8("vboxLayout"));
    gridLayout = new QGridLayout();
    gridLayout->setSpacing(6);
    gridLayout->setMargin(0);
    gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
    filenameLabel = new QLabel(BaseGenGraphForm);
    filenameLabel->setObjectName(QString::fromUtf8("filenameLabel"));
    QSizePolicy sizePolicy(static_cast<QSizePolicy::Policy>(5), static_cast<QSizePolicy::Policy>(5));
    sizePolicy.setHorizontalStretch(1);
    sizePolicy.setVerticalStretch(0);
    sizePolicy.setHeightForWidth(filenameLabel->sizePolicy().hasHeightForWidth());
    filenameLabel->setSizePolicy(sizePolicy);

    gridLayout->addWidget(filenameLabel, 0, 0, 1, 1);

    browseButton = new QPushButton(BaseGenGraphForm);
    browseButton->setObjectName(QString::fromUtf8("browseButton"));

    gridLayout->addWidget(browseButton, 0, 1, 1, 1);

    filename = new QLineEdit(BaseGenGraphForm);
    filename->setObjectName(QString::fromUtf8("filename"));

    gridLayout->addWidget(filename, 1, 0, 1, 2);


    vboxLayout->addLayout(gridLayout);

    hboxLayout = new QHBoxLayout();
    hboxLayout->setSpacing(6);
    hboxLayout->setMargin(0);
    hboxLayout->setObjectName(QString::fromUtf8("hboxLayout"));
    layoutGroup = new Q3ButtonGroup(BaseGenGraphForm);
    layoutGroup->setObjectName(QString::fromUtf8("layoutGroup"));
    layoutGroup->setColumnLayout(0, Qt::Vertical);
    layoutGroup->layout()->setSpacing(6);
    layoutGroup->layout()->setMargin(11);
    vboxLayout1 = new QVBoxLayout((QWidget *)layoutGroup->layout());
    vboxLayout1->setAlignment(Qt::AlignTop);
    vboxLayout1->setObjectName(QString::fromUtf8("vboxLayout1"));
    layoutDirV = new QRadioButton(layoutGroup);
    layoutDirV->setObjectName(QString::fromUtf8("layoutDirV"));
    layoutDirV->setChecked(true);

    vboxLayout1->addWidget(layoutDirV);

    layoutDirH = new QRadioButton(layoutGroup);
    layoutDirH->setObjectName(QString::fromUtf8("layoutDirH"));

    vboxLayout1->addWidget(layoutDirH);

    layoutSpring = new QRadioButton(layoutGroup);
    layoutSpring->setObjectName(QString::fromUtf8("layoutSpring"));

    vboxLayout1->addWidget(layoutSpring);

    layoutRadial = new QRadioButton(layoutGroup);
    layoutRadial->setObjectName(QString::fromUtf8("layoutRadial"));

    vboxLayout1->addWidget(layoutRadial);


    hboxLayout->addWidget(layoutGroup);

    outputsBox = new Q3GroupBox(BaseGenGraphForm);
    outputsBox->setObjectName(QString::fromUtf8("outputsBox"));
    outputsBox->setColumnLayout(0, Qt::Vertical);
    outputsBox->layout()->setSpacing(6);
    outputsBox->layout()->setMargin(11);
    vboxLayout2 = new QVBoxLayout((QWidget *)outputsBox->layout());
    vboxLayout2->setAlignment(Qt::AlignTop);
    vboxLayout2->setObjectName(QString::fromUtf8("vboxLayout2"));
    genPNG = new QCheckBox(outputsBox);
    genPNG->setObjectName(QString::fromUtf8("genPNG"));
    genPNG->setChecked(true);

    vboxLayout2->addWidget(genPNG);

    genPS = new QCheckBox(outputsBox);
    genPS->setObjectName(QString::fromUtf8("genPS"));

    vboxLayout2->addWidget(genPS);

    genFIG = new QCheckBox(outputsBox);
    genFIG->setObjectName(QString::fromUtf8("genFIG"));

    vboxLayout2->addWidget(genFIG);

    genSVG = new QCheckBox(outputsBox);
    genSVG->setObjectName(QString::fromUtf8("genSVG"));

    vboxLayout2->addWidget(genSVG);


    hboxLayout->addWidget(outputsBox);


    vboxLayout->addLayout(hboxLayout);

    hboxLayout1 = new QHBoxLayout();
    hboxLayout1->setSpacing(6);
    hboxLayout1->setMargin(0);
    hboxLayout1->setObjectName(QString::fromUtf8("hboxLayout1"));
    presetLabel = new QLabel(BaseGenGraphForm);
    presetLabel->setObjectName(QString::fromUtf8("presetLabel"));

    hboxLayout1->addWidget(presetLabel);

    presetFilter = new QComboBox(BaseGenGraphForm);
    presetFilter->setObjectName(QString::fromUtf8("presetFilter"));
    QSizePolicy sizePolicy1(static_cast<QSizePolicy::Policy>(1), static_cast<QSizePolicy::Policy>(0));
    sizePolicy1.setHorizontalStretch(1);
    sizePolicy1.setVerticalStretch(0);
    sizePolicy1.setHeightForWidth(presetFilter->sizePolicy().hasHeightForWidth());
    presetFilter->setSizePolicy(sizePolicy1);
    presetFilter->setEditable(true);
    presetFilter->setInsertPolicy(QComboBox::InsertAtTop);
    presetFilter->setAutoCompletion(true);
    presetFilter->setDuplicatesEnabled(false);

    hboxLayout1->addWidget(presetFilter);


    vboxLayout->addLayout(hboxLayout1);

    filterBox = new Q3GroupBox(BaseGenGraphForm);
    filterBox->setObjectName(QString::fromUtf8("filterBox"));
    filterBox->setColumnLayout(0, Qt::Vertical);
    filterBox->layout()->setSpacing(6);
    filterBox->layout()->setMargin(11);
    gridLayout1 = new QGridLayout((QWidget *)filterBox->layout());
    gridLayout1->setAlignment(Qt::AlignTop);
    gridLayout1->setObjectName(QString::fromUtf8("gridLayout1"));
    showSolvers = new QCheckBox(filterBox);
    showSolvers->setObjectName(QString::fromUtf8("showSolvers"));
    showSolvers->setChecked(true);

    gridLayout1->addWidget(showSolvers, 0, 1, 1, 1);

    showMechanicalStates = new QCheckBox(filterBox);
    showMechanicalStates->setObjectName(QString::fromUtf8("showMechanicalStates"));
    showMechanicalStates->setChecked(true);

    gridLayout1->addWidget(showMechanicalStates, 1, 1, 1, 1);

    showForceFields = new QCheckBox(filterBox);
    showForceFields->setObjectName(QString::fromUtf8("showForceFields"));
    showForceFields->setChecked(true);

    gridLayout1->addWidget(showForceFields, 2, 1, 1, 1);

    showInteractionForceFields = new QCheckBox(filterBox);
    showInteractionForceFields->setObjectName(QString::fromUtf8("showInteractionForceFields"));
    showInteractionForceFields->setChecked(true);

    gridLayout1->addWidget(showInteractionForceFields, 3, 1, 1, 1);

    showConstraints = new QCheckBox(filterBox);
    showConstraints->setObjectName(QString::fromUtf8("showConstraints"));
    showConstraints->setChecked(true);

    gridLayout1->addWidget(showConstraints, 4, 1, 1, 1);

    showMass = new QCheckBox(filterBox);
    showMass->setObjectName(QString::fromUtf8("showMass"));
    showMass->setChecked(true);

    gridLayout1->addWidget(showMass, 5, 1, 1, 1);

    showMechanicalMappings = new QCheckBox(filterBox);
    showMechanicalMappings->setObjectName(QString::fromUtf8("showMechanicalMappings"));
    showMechanicalMappings->setChecked(true);

    gridLayout1->addWidget(showMechanicalMappings, 7, 1, 1, 1);

    showTopology = new QCheckBox(filterBox);
    showTopology->setObjectName(QString::fromUtf8("showTopology"));
    showTopology->setChecked(true);

    gridLayout1->addWidget(showTopology, 6, 1, 1, 1);

    showMappings = new QCheckBox(filterBox);
    showMappings->setObjectName(QString::fromUtf8("showMappings"));
    showMappings->setChecked(true);

    gridLayout1->addWidget(showMappings, 5, 0, 1, 1);

    showCollisionModels = new QCheckBox(filterBox);
    showCollisionModels->setObjectName(QString::fromUtf8("showCollisionModels"));
    showCollisionModels->setChecked(true);

    gridLayout1->addWidget(showCollisionModels, 3, 0, 1, 1);

    showVisualModels = new QCheckBox(filterBox);
    showVisualModels->setObjectName(QString::fromUtf8("showVisualModels"));
    showVisualModels->setChecked(true);

    gridLayout1->addWidget(showVisualModels, 4, 0, 1, 1);

    showCollisionPipeline = new QCheckBox(filterBox);
    showCollisionPipeline->setObjectName(QString::fromUtf8("showCollisionPipeline"));
    showCollisionPipeline->setChecked(true);

    gridLayout1->addWidget(showCollisionPipeline, 7, 0, 1, 1);

    showBehaviorModels = new QCheckBox(filterBox);
    showBehaviorModels->setObjectName(QString::fromUtf8("showBehaviorModels"));
    showBehaviorModels->setChecked(true);

    gridLayout1->addWidget(showBehaviorModels, 2, 0, 1, 1);

    showContext = new QCheckBox(filterBox);
    showContext->setObjectName(QString::fromUtf8("showContext"));
    showContext->setChecked(true);

    gridLayout1->addWidget(showContext, 6, 0, 1, 1);

    showObjects = new QCheckBox(filterBox);
    showObjects->setObjectName(QString::fromUtf8("showObjects"));
    showObjects->setChecked(true);

    gridLayout1->addWidget(showObjects, 1, 0, 1, 1);

    showNodes = new QCheckBox(filterBox);
    showNodes->setObjectName(QString::fromUtf8("showNodes"));
    showNodes->setChecked(true);

    gridLayout1->addWidget(showNodes, 0, 0, 1, 1);


    vboxLayout->addWidget(filterBox);

    hboxLayout2 = new QHBoxLayout();
    hboxLayout2->setSpacing(6);
    hboxLayout2->setMargin(0);
    hboxLayout2->setObjectName(QString::fromUtf8("hboxLayout2"));
    groupNodeLabel = new Q3GroupBox(BaseGenGraphForm);
    groupNodeLabel->setObjectName(QString::fromUtf8("groupNodeLabel"));
    groupNodeLabel->setColumnLayout(0, Qt::Vertical);
    groupNodeLabel->layout()->setSpacing(6);
    groupNodeLabel->layout()->setMargin(11);
    vboxLayout3 = new QVBoxLayout((QWidget *)groupNodeLabel->layout());
    vboxLayout3->setAlignment(Qt::AlignTop);
    vboxLayout3->setObjectName(QString::fromUtf8("vboxLayout3"));
    labelNodeName = new QCheckBox(groupNodeLabel);
    labelNodeName->setObjectName(QString::fromUtf8("labelNodeName"));
    labelNodeName->setChecked(true);

    vboxLayout3->addWidget(labelNodeName);

    labelNodeClass = new QCheckBox(groupNodeLabel);
    labelNodeClass->setObjectName(QString::fromUtf8("labelNodeClass"));

    vboxLayout3->addWidget(labelNodeClass);


    hboxLayout2->addWidget(groupNodeLabel);

    groupObjectLabel = new Q3GroupBox(BaseGenGraphForm);
    groupObjectLabel->setObjectName(QString::fromUtf8("groupObjectLabel"));
    groupObjectLabel->setColumnLayout(0, Qt::Vertical);
    groupObjectLabel->layout()->setSpacing(6);
    groupObjectLabel->layout()->setMargin(11);
    vboxLayout4 = new QVBoxLayout((QWidget *)groupObjectLabel->layout());
    vboxLayout4->setAlignment(Qt::AlignTop);
    vboxLayout4->setObjectName(QString::fromUtf8("vboxLayout4"));
    labelObjectName = new QCheckBox(groupObjectLabel);
    labelObjectName->setObjectName(QString::fromUtf8("labelObjectName"));
    labelObjectName->setChecked(true);

    vboxLayout4->addWidget(labelObjectName);

    labelObjectClass = new QCheckBox(groupObjectLabel);
    labelObjectClass->setObjectName(QString::fromUtf8("labelObjectClass"));
    labelObjectClass->setChecked(true);

    vboxLayout4->addWidget(labelObjectClass);


    hboxLayout2->addWidget(groupObjectLabel);


    vboxLayout->addLayout(hboxLayout2);

    hboxLayout3 = new QHBoxLayout();
    hboxLayout3->setSpacing(6);
    hboxLayout3->setMargin(0);
    hboxLayout3->setObjectName(QString::fromUtf8("hboxLayout3"));
    spacerItem = new QSpacerItem(16, 20, QSizePolicy::Expanding, QSizePolicy::Minimum);

    hboxLayout3->addItem(spacerItem);

    displayButton = new QPushButton(BaseGenGraphForm);
    displayButton->setObjectName(QString::fromUtf8("displayButton"));
    displayButton->setEnabled(false);

    hboxLayout3->addWidget(displayButton);

    exportButton = new QPushButton(BaseGenGraphForm);
    exportButton->setObjectName(QString::fromUtf8("exportButton"));
    exportButton->setDefault(true);

    hboxLayout3->addWidget(exportButton);

    closeButton = new QPushButton(BaseGenGraphForm);
    closeButton->setObjectName(QString::fromUtf8("closeButton"));

    hboxLayout3->addWidget(closeButton);


    vboxLayout->addLayout(hboxLayout3);

    filenameLabel->setBuddy(filename);
    QWidget::setTabOrder(filename, browseButton);
    QWidget::setTabOrder(browseButton, layoutDirV);
    QWidget::setTabOrder(layoutDirV, layoutDirH);
    QWidget::setTabOrder(layoutDirH, layoutSpring);
    QWidget::setTabOrder(layoutSpring, layoutRadial);
    QWidget::setTabOrder(layoutRadial, genPNG);
    QWidget::setTabOrder(genPNG, genPS);
    QWidget::setTabOrder(genPS, genFIG);
    QWidget::setTabOrder(genFIG, genSVG);
    QWidget::setTabOrder(genSVG, showSolvers);
    QWidget::setTabOrder(showSolvers, showMechanicalStates);
    QWidget::setTabOrder(showMechanicalStates, showForceFields);
    QWidget::setTabOrder(showForceFields, showInteractionForceFields);
    QWidget::setTabOrder(showInteractionForceFields, showConstraints);
    QWidget::setTabOrder(showConstraints, showMass);
    QWidget::setTabOrder(showMass, showMechanicalMappings);
    QWidget::setTabOrder(showMechanicalMappings, showTopology);
    QWidget::setTabOrder(showTopology, showMappings);
    QWidget::setTabOrder(showMappings, showCollisionModels);
    QWidget::setTabOrder(showCollisionModels, showVisualModels);
    QWidget::setTabOrder(showVisualModels, showCollisionPipeline);
    QWidget::setTabOrder(showCollisionPipeline, showBehaviorModels);
    QWidget::setTabOrder(showBehaviorModels, showContext);
    QWidget::setTabOrder(showContext, showObjects);
    QWidget::setTabOrder(showObjects, showNodes);
    QWidget::setTabOrder(showNodes, labelNodeName);
    QWidget::setTabOrder(labelNodeName, labelNodeClass);
    QWidget::setTabOrder(labelNodeClass, labelObjectName);
    QWidget::setTabOrder(labelObjectName, labelObjectClass);
    QWidget::setTabOrder(labelObjectClass, displayButton);
    QWidget::setTabOrder(displayButton, exportButton);
    retranslateUi(BaseGenGraphForm);

    QMetaObject::connectSlotsByName(BaseGenGraphForm);
    } // setupUi

    void retranslateUi(QDialog *BaseGenGraphForm)
    {
    BaseGenGraphForm->setWindowTitle(QApplication::translate("BaseGenGraphForm", "Sofa Graph Export", 0, QApplication::UnicodeUTF8));
    filenameLabel->setText(QApplication::translate("BaseGenGraphForm", "Output DOT &File", 0, QApplication::UnicodeUTF8));
    browseButton->setText(QApplication::translate("BaseGenGraphForm", "Browse...", 0, QApplication::UnicodeUTF8));
    filename->setText(QApplication::translate("BaseGenGraphForm", "untitled", 0, QApplication::UnicodeUTF8));
    layoutGroup->setTitle(QApplication::translate("BaseGenGraphForm", "Layout", 0, QApplication::UnicodeUTF8));
    layoutDirV->setText(QApplication::translate("BaseGenGraphForm", "Directed Vertically", 0, QApplication::UnicodeUTF8));
    layoutDirH->setText(QApplication::translate("BaseGenGraphForm", "Directed Horizontally", 0, QApplication::UnicodeUTF8));
    layoutSpring->setText(QApplication::translate("BaseGenGraphForm", "Undirected Spring", 0, QApplication::UnicodeUTF8));
    layoutRadial->setText(QApplication::translate("BaseGenGraphForm", "Undirected Radial", 0, QApplication::UnicodeUTF8));
    outputsBox->setTitle(QApplication::translate("BaseGenGraphForm", "Outputs", 0, QApplication::UnicodeUTF8));
    genPNG->setText(QApplication::translate("BaseGenGraphForm", "Generate PNG", 0, QApplication::UnicodeUTF8));
    genPS->setText(QApplication::translate("BaseGenGraphForm", "Generate PS", 0, QApplication::UnicodeUTF8));
    genFIG->setText(QApplication::translate("BaseGenGraphForm", "Generate FIG", 0, QApplication::UnicodeUTF8));
    genSVG->setText(QApplication::translate("BaseGenGraphForm", "Generate SVG", 0, QApplication::UnicodeUTF8));
    presetLabel->setText(QApplication::translate("BaseGenGraphForm", "Presets", 0, QApplication::UnicodeUTF8));
    presetFilter->clear();
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Full Graph", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "All Objects", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "All Nodes", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Mechanical Graph", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Mechanical Objects", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Visual Graph", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Visual Objects", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Collision Graph", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Collision Objects", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Collision Response Graph", 0, QApplication::UnicodeUTF8));
    presetFilter->addItem(QApplication::translate("BaseGenGraphForm", "Collision Response Objects", 0, QApplication::UnicodeUTF8));
    filterBox->setTitle(QApplication::translate("BaseGenGraphForm", "Filter", 0, QApplication::UnicodeUTF8));
    showSolvers->setText(QApplication::translate("BaseGenGraphForm", "Solvers", 0, QApplication::UnicodeUTF8));
    showMechanicalStates->setText(QApplication::translate("BaseGenGraphForm", "Mechanical Models", 0, QApplication::UnicodeUTF8));
    showForceFields->setText(QApplication::translate("BaseGenGraphForm", "Force Fields", 0, QApplication::UnicodeUTF8));
    showInteractionForceFields->setText(QApplication::translate("BaseGenGraphForm", "Interaction Force Fields", 0, QApplication::UnicodeUTF8));
    showConstraints->setText(QApplication::translate("BaseGenGraphForm", "Constraints", 0, QApplication::UnicodeUTF8));
    showMass->setText(QApplication::translate("BaseGenGraphForm", "Mass", 0, QApplication::UnicodeUTF8));
    showMechanicalMappings->setText(QApplication::translate("BaseGenGraphForm", "Mechanical Mappings", 0, QApplication::UnicodeUTF8));
    showTopology->setText(QApplication::translate("BaseGenGraphForm", "Topology", 0, QApplication::UnicodeUTF8));
    showMappings->setText(QApplication::translate("BaseGenGraphForm", "Mappings", 0, QApplication::UnicodeUTF8));
    showCollisionModels->setText(QApplication::translate("BaseGenGraphForm", "Collision Models", 0, QApplication::UnicodeUTF8));
    showVisualModels->setText(QApplication::translate("BaseGenGraphForm", "Visual Models", 0, QApplication::UnicodeUTF8));
    showCollisionPipeline->setText(QApplication::translate("BaseGenGraphForm", "Collision Pipeline", 0, QApplication::UnicodeUTF8));
    showBehaviorModels->setText(QApplication::translate("BaseGenGraphForm", "Behavior Models", 0, QApplication::UnicodeUTF8));
    showContext->setText(QApplication::translate("BaseGenGraphForm", "Context", 0, QApplication::UnicodeUTF8));
    showObjects->setText(QApplication::translate("BaseGenGraphForm", "Objects", 0, QApplication::UnicodeUTF8));
    showNodes->setText(QApplication::translate("BaseGenGraphForm", "Nodes", 0, QApplication::UnicodeUTF8));
    groupNodeLabel->setTitle(QApplication::translate("BaseGenGraphForm", "Nodes Labels", 0, QApplication::UnicodeUTF8));
    labelNodeName->setText(QApplication::translate("BaseGenGraphForm", "Name", 0, QApplication::UnicodeUTF8));
    labelNodeClass->setText(QApplication::translate("BaseGenGraphForm", "Class", 0, QApplication::UnicodeUTF8));
    groupObjectLabel->setTitle(QApplication::translate("BaseGenGraphForm", "Objects Labels", 0, QApplication::UnicodeUTF8));
    labelObjectName->setText(QApplication::translate("BaseGenGraphForm", "Name", 0, QApplication::UnicodeUTF8));
    labelObjectClass->setText(QApplication::translate("BaseGenGraphForm", "Class", 0, QApplication::UnicodeUTF8));
    displayButton->setText(QApplication::translate("BaseGenGraphForm", "&Display", 0, QApplication::UnicodeUTF8));
    displayButton->setShortcut(QApplication::translate("BaseGenGraphForm", "Alt+D", 0, QApplication::UnicodeUTF8));
    exportButton->setText(QApplication::translate("BaseGenGraphForm", "&Export", 0, QApplication::UnicodeUTF8));
    exportButton->setShortcut(QApplication::translate("BaseGenGraphForm", "Alt+E", 0, QApplication::UnicodeUTF8));
    closeButton->setText(QApplication::translate("BaseGenGraphForm", "&Close", 0, QApplication::UnicodeUTF8));
    closeButton->setShortcut(QApplication::translate("BaseGenGraphForm", "Alt+C", 0, QApplication::UnicodeUTF8));
    Q_UNUSED(BaseGenGraphForm);
    } // retranslateUi


protected:
    enum IconID
    {
        image0_ID,
        unknown_ID
    };
    static QPixmap icon(IconID id)
    {
    static const unsigned char image0_data[] = { 
    0x89, 0x50, 0x4e, 0x47, 0x0d, 0x0a, 0x1a, 0x0a, 0x00, 0x00, 0x00, 0x0d,
    0x49, 0x48, 0x44, 0x52, 0x00, 0x00, 0x00, 0x10, 0x00, 0x00, 0x00, 0x10,
    0x08, 0x06, 0x00, 0x00, 0x00, 0x1f, 0xf3, 0xff, 0x61, 0x00, 0x00, 0x01,
    0xe7, 0x49, 0x44, 0x41, 0x54, 0x78, 0x9c, 0x8d, 0x52, 0xa1, 0xef, 0xaa,
    0x60, 0x14, 0xbd, 0xb0, 0x27, 0x9b, 0xfb, 0x18, 0xc1, 0x19, 0x1d, 0xc9,
    0x64, 0x62, 0xd3, 0xca, 0x06, 0xcd, 0xe2, 0x46, 0x24, 0x99, 0x0c, 0x26,
    0x92, 0xc1, 0x3f, 0xc1, 0xff, 0x81, 0x80, 0x13, 0x82, 0x33, 0xfa, 0x17,
    0x50, 0x29, 0x34, 0x1a, 0x23, 0x60, 0xa0, 0x19, 0x64, 0x6e, 0xc2, 0xbe,
    0x71, 0x5e, 0x70, 0xbf, 0x6f, 0xef, 0x3d, 0x79, 0x9b, 0xa7, 0x7d, 0xf7,
    0x9e, 0xef, 0xec, 0xdc, 0x73, 0x2f, 0x51, 0x0f, 0x9e, 0xcf, 0x27, 0x36,
    0x9b, 0x0d, 0x26, 0x93, 0x09, 0x26, 0x93, 0x09, 0x36, 0x9b, 0x0d, 0x9e,
    0xcf, 0x27, 0xfa, 0xb8, 0xbf, 0xfa, 0x8a, 0xbb, 0xdd, 0x8e, 0xae, 0xd7,
    0x2b, 0xed, 0xf7, 0x7b, 0x22, 0x22, 0x3a, 0x1c, 0x0e, 0x34, 0x18, 0x0c,
    0xfa, 0xa8, 0x9f, 0x78, 0xbd, 0x5e, 0x50, 0x14, 0x05, 0x61, 0x18, 0xe2,
    0x07, 0x61, 0x18, 0x42, 0x51, 0x14, 0xbc, 0x5e, 0xaf, 0x0f, 0x17, 0xf2,
    0xbf, 0x85, 0xb6, 0x6d, 0x89, 0x73, 0x4e, 0xaa, 0xaa, 0x8a, 0x9a, 0xaa,
    0xaa, 0xc4, 0x39, 0xa7, 0xb6, 0x6d, 0xbf, 0x73, 0xb1, 0x5c, 0x2e, 0xb1,
    0x58, 0x2c, 0x90, 0x24, 0x09, 0x92, 0x24, 0xc1, 0x7c, 0x3e, 0xc7, 0x72,
    0xb9, 0xec, 0xcd, 0xa0, 0x17, 0x65, 0x59, 0x62, 0x34, 0x1a, 0x81, 0x88,
    0x40, 0x44, 0x18, 0x8d, 0x46, 0x28, 0xcb, 0xf2, 0xfb, 0x10, 0xab, 0xaa,
    0xa2, 0xfb, 0xfd, 0x4e, 0xa7, 0xd3, 0x89, 0x88, 0x88, 0xd6, 0xeb, 0x35,
    0x55, 0x55, 0xf5, 0xb5, 0x01, 0x72, 0x1c, 0x07, 0x86, 0x61, 0xa0, 0xeb,
    0x3a, 0x74, 0x5d, 0x07, 0xc3, 0x30, 0xe0, 0x38, 0xce, 0x77, 0x23, 0x64,
    0x59, 0x06, 0x49, 0x92, 0x70, 0x3e, 0x9f, 0xc5, 0x87, 0xf3, 0xf9, 0x0c,
    0x49, 0x92, 0x90, 0x65, 0xd9, 0xff, 0x45, 0x9a, 0xa6, 0x41, 0x51, 0x14,
    0x70, 0x5d, 0x17, 0xd3, 0xe9, 0x14, 0x9c, 0x73, 0x41, 0xe6, 0x9c, 0x63,
    0x3a, 0x9d, 0xc2, 0x75, 0x5d, 0x14, 0x45, 0x81, 0xa6, 0x69, 0xfe, 0x16,
    0xba, 0x5c, 0x2e, 0xd0, 0x34, 0x4d, 0x84, 0x66, 0xdb, 0x36, 0xba, 0xae,
    0x13, 0xa4, 0xae, 0xeb, 0x60, 0xdb, 0xb6, 0xe8, 0x6b, 0x9a, 0x86, 0xcb,
    0xe5, 0xf2, 0xee, 0xdf, 0x6e, 0x37, 0x30, 0xc6, 0xe0, 0x79, 0x1e, 0xf2,
    0x3c, 0x47, 0x14, 0x45, 0x60, 0x8c, 0xe1, 0x78, 0x3c, 0x0a, 0x81, 0xe3,
    0xf1, 0x08, 0xc6, 0x18, 0xa2, 0x28, 0x42, 0x9e, 0xe7, 0xf0, 0x3c, 0x0f,
    0x8c, 0x31, 0xdc, 0x6e, 0x37, 0x90, 0xef, 0xfb, 0x60, 0x8c, 0x81, 0x73,
    0x2e, 0x2e, 0xcf, 0x75, 0x5d, 0xac, 0x56, 0x2b, 0x21, 0xb0, 0x5a, 0xad,
    0xe0, 0xba, 0xae, 0xe8, 0x73, 0xce, 0xc1, 0x18, 0x83, 0xef, 0xfb, 0x90,
    0x55, 0x55, 0xa5, 0xb6, 0x6d, 0xa9, 0x69, 0x1a, 0x31, 0xd2, 0xe3, 0xf1,
    0xa0, 0xe1, 0x70, 0x28, 0xde, 0xc3, 0xe1, 0x90, 0x1e, 0x8f, 0xc7, 0x9f,
    0x79, 0x51, 0xdb, 0xb6, 0xef, 0x6b, 0xad, 0xeb, 0x1a, 0xba, 0xae, 0xc3,
    0x34, 0x4d, 0x84, 0x61, 0x88, 0xed, 0x76, 0x0b, 0x59, 0x96, 0x11, 0xc7,
    0xb1, 0x70, 0x10, 0xc7, 0x31, 0x64, 0x59, 0xc6, 0x76, 0xbb, 0x45, 0x18,
    0x86, 0x30, 0x4d, 0x13, 0xba, 0xae, 0xa3, 0xae, 0xeb, 0x37, 0x27, 0x4d,
    0x53, 0x58, 0x96, 0x85, 0xf1, 0x78, 0x8c, 0xd9, 0x6c, 0x86, 0x20, 0x08,
    0x3e, 0xd6, 0x15, 0x04, 0x01, 0x66, 0xb3, 0x19, 0xc6, 0xe3, 0x31, 0x2c,
    0xcb, 0x42, 0x9a, 0xa6, 0x20, 0x22, 0xfa, 0x0d, 0x2e, 0x6f, 0x43, 0x92,
    0x53, 0xd2, 0x56, 0x03, 0x00, 0x00, 0x00, 0x00, 0x49, 0x45, 0x4e, 0x44,
    0xae, 0x42, 0x60, 0x82
};

    switch (id) {
        case image0_ID:  { QImage img; img.loadFromData(image0_data, sizeof(image0_data), "PNG"); return QPixmap::fromImage(img); }
        default: return QPixmap();
    } // switch
    } // icon

};

namespace Ui {
    class BaseGenGraphForm: public Ui_BaseGenGraphForm {};
} // namespace Ui

class BaseGenGraphForm : public QDialog, public Ui::BaseGenGraphForm
{
    Q_OBJECT

public:
    BaseGenGraphForm(QWidget* parent = 0, const char* name = 0, bool modal = false, Qt::WFlags fl = 0);
    ~BaseGenGraphForm();

protected slots:
    virtual void languageChange();

};

#endif // BASEGENGRAPHFORM_H
