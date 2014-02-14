#include "BaseGenGraphForm.h"

#include <qvariant.h>
/*
 *  Constructs a BaseGenGraphForm as a child of 'parent', with the
 *  name 'name' and widget flags set to 'f'.
 *
 *  The dialog will by default be modeless, unless you set 'modal' to
 *  true to construct a modal dialog.
 */
BaseGenGraphForm::BaseGenGraphForm(QWidget* parent, const char* name, bool modal, Qt::WFlags fl)
//    : QDialog(parent, name, modal, fl)
    : QDialog(parent, fl)
{
    setupUi(this);


    // signals and slots connections
    connect(showObjects, SIGNAL(toggled(bool)), showBehaviorModels, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showCollisionModels, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showVisualModels, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showMappings, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showContext, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showCollisionPipeline, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showSolvers, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showMechanicalStates, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showForceFields, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showInteractionForceFields, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showConstraints, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showMass, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showTopology, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), showMechanicalMappings, SLOT(setEnabled(bool)));
    connect(showObjects, SIGNAL(toggled(bool)), groupObjectLabel, SLOT(setEnabled(bool)));
    connect(showNodes, SIGNAL(toggled(bool)), groupNodeLabel, SLOT(setEnabled(bool)));
    connect(showNodes, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showObjects, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showBehaviorModels, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showCollisionModels, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showVisualModels, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showMappings, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showContext, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showCollisionPipeline, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showSolvers, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showMechanicalStates, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showForceFields, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showInteractionForceFields, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showConstraints, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showMass, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showTopology, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
    connect(showMechanicalMappings, SIGNAL(toggled(bool)), presetFilter, SLOT(clearEdit()));
}

/*
 *  Destroys the object and frees any allocated resources
 */
BaseGenGraphForm::~BaseGenGraphForm()
{
    // no need to delete child widgets, Qt does it all for us
}

/*
 *  Sets the strings of the subwidgets using the current
 *  language.
 */
void BaseGenGraphForm::languageChange()
{
    retranslateUi(this);
}

