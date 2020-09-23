/*==============================================================================

  Program: 3D Slicer

  Portions (c) Copyright Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

==============================================================================*/

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerLookingGlassModuleWidget.h"
#include "ui_qSlicerLookingGlassModuleWidget.h"

// LookingGlass Logic includes
#include <vtkSlicerLookingGlassLogic.h>

// LookingGlass MRML includes
#include <vtkMRMLLookingGlassViewNode.h>

// LookingGlass Widget includes
#include <qMRMLLookingGlassView.h>

// LookingGlass includes
#include "qSlicerLookingGlassModule.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLookingGlassModuleWidgetPrivate: public Ui_qSlicerLookingGlassModuleWidget
{
public:
  qSlicerLookingGlassModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerLookingGlassModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerLookingGlassModuleWidgetPrivate::qSlicerLookingGlassModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerLookingGlassModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerLookingGlassModuleWidget::qSlicerLookingGlassModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerLookingGlassModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerLookingGlassModuleWidget::~qSlicerLookingGlassModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::setup()
{
  Q_D(qSlicerLookingGlassModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();

  // Connection
  connect(d->ConnectCheckBox, SIGNAL(toggled(bool)), this, SLOT(setLookingGlassConnected(bool)));
  connect(d->RenderingEnabledCheckBox, SIGNAL(toggled(bool)), this, SLOT(setLookingGlassActive(bool)));

  // Display
  connect(d->FocalPlanePushBackButton, SIGNAL(clicked()), this, SLOT(pushFocalPlaneBack()));
  connect(d->FocalPlanePullForwardButton, SIGNAL(clicked()), this, SLOT(pullFocalPlaneForward()));
  connect(d->DesiredUpdateRateSlider, SIGNAL(valueChanged(double)), this, SLOT(onDesiredUpdateRateChanged(double)));
  connect(d->UpdateViewFromReferenceViewCameraButton, SIGNAL(clicked()), this, SLOT(updateViewFromReferenceViewCamera()));

  // Advanced
  connect(d->ReferenceViewNodeComboBox, SIGNAL(currentNodeChanged(vtkMRMLNode*)), this, SLOT(setReferenceViewNode(vtkMRMLNode*)));
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::setLookingGlassConnected(bool connect)
{
  Q_D(qSlicerLookingGlassModuleWidget);
  vtkSlicerLookingGlassLogic* lgLogic = vtkSlicerLookingGlassLogic::SafeDownCast(this->logic());
  lgLogic->SetLookingGlassConnected(connect);
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::setLookingGlassActive(bool activate)
{
  Q_D(qSlicerLookingGlassModuleWidget);
  vtkSlicerLookingGlassLogic* lgLogic = vtkSlicerLookingGlassLogic::SafeDownCast(this->logic());
  lgLogic->SetLookingGlassActive(activate);
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::setReferenceViewNode(vtkMRMLNode* node)
{
  Q_D(qSlicerLookingGlassModuleWidget);
  vtkSlicerLookingGlassLogic* lgLogic = vtkSlicerLookingGlassLogic::SafeDownCast(this->logic());
  vtkMRMLLookingGlassViewNode* lgViewNode = lgLogic->GetLookingGlassViewNode();
  if (lgViewNode)
    {
    vtkMRMLViewNode* referenceViewNode = vtkMRMLViewNode::SafeDownCast(node);
    lgViewNode->SetAndObserveReferenceViewNode(referenceViewNode);
    }
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::updateViewFromReferenceViewCamera()
{
  Q_D(qSlicerLookingGlassModuleWidget);
  qSlicerLookingGlassModule* lgModule = dynamic_cast<qSlicerLookingGlassModule*>(this->module());
  if (!lgModule)
    {
    return;
    }
  qMRMLLookingGlassView* lgView = lgModule->viewWidget();
  if (!lgView)
    {
    return;
    }
  lgView->updateViewFromReferenceViewCamera();
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::onDesiredUpdateRateChanged(double fps)
{
  Q_D(qSlicerLookingGlassModuleWidget);
  vtkSlicerLookingGlassLogic* lgLogic = vtkSlicerLookingGlassLogic::SafeDownCast(this->logic());
  vtkMRMLLookingGlassViewNode* lgViewNode = lgLogic->GetLookingGlassViewNode();
  if (lgViewNode)
    {
    lgViewNode->SetDesiredUpdateRate(fps);
    }
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::pullFocalPlaneForward()
{
  qSlicerLookingGlassModule* lgModule = dynamic_cast<qSlicerLookingGlassModule*>(this->module());
  if (!lgModule)
    {
    return;
    }
  qMRMLLookingGlassView* lgView = lgModule->viewWidget();
  if (!lgView)
    {
    return;
    }
  lgView->pullFocalPlaneForward();
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::pushFocalPlaneBack()
{
  qSlicerLookingGlassModule* lgModule = dynamic_cast<qSlicerLookingGlassModule*>(this->module());
  if (!lgModule)
    {
    return;
    }
  qMRMLLookingGlassView* lgView = lgModule->viewWidget();
  if (!lgView)
    {
    return;
    }
  lgView->pushFocalPlaneBack();
}
