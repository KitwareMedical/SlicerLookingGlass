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
#include <QAction>
#include <QApplication>
#include <QDebug>
#include <QMainWindow>
#include <QMenu>
#include <QSettings>
#include <QToolBar>
#include <QtPlugin>
#include <QSpacerItem>

// LookingGlass Logic includes
#include <vtkSlicerLookingGlassLogic.h>

// LookingGlass MRML includes
#include <vtkMRMLLookingGlassViewNode.h>

// LookingGlass Widget includes
#include <qMRMLLookingGlassView.h>

// LookingGlass includes
#include "qSlicerLookingGlassModule.h"
#include "qSlicerLookingGlassModuleWidget.h"

// Slicer includes
#include <qSlicerApplication.h>
#include <qSlicerCoreApplication.h>
#include <qSlicerModuleManager.h>
#include <vtkSlicerCamerasModuleLogic.h>
#include <vtkSlicerVolumeRenderingLogic.h>

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLookingGlassModulePrivate
{
  Q_DECLARE_PUBLIC(qSlicerLookingGlassModule)
protected:
  qSlicerLookingGlassModule* const q_ptr;
public:
  qSlicerLookingGlassModulePrivate(qSlicerLookingGlassModule& object);
  virtual ~qSlicerLookingGlassModulePrivate();

  vtkSlicerLookingGlassLogic* logic();

  /// Adds Looking Glass toolbar to the application GUI (toolbar and menu)
  void addToolBar();

  /// Updated toolbar button states.
  void updateToolBar();

  /// Adds Looking Glass view widget
  void addViewWidget();

  QToolBar* ToolBar;
  QAction* LookingGlassToggleAction;
  QAction* UpdateViewFromReferenceViewCameraAction;
  QAction* ConfigureAction;
  qMRMLLookingGlassView* LookingGlassViewWidget;
  QAction* Spacer;
};

//-----------------------------------------------------------------------------
vtkSlicerLookingGlassLogic* qSlicerLookingGlassModulePrivate::logic()
{
  Q_Q(qSlicerLookingGlassModule);
  return vtkSlicerLookingGlassLogic::SafeDownCast(q->logic());
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModulePrivate::addViewWidget()
{
  Q_Q(qSlicerLookingGlassModule);

  if (this->LookingGlassViewWidget != nullptr)
    {
    return;
    }

  this->LookingGlassViewWidget = new qMRMLLookingGlassView();
  this->LookingGlassViewWidget->setObjectName(QString("LookingGlassWidget"));

  qSlicerAbstractCoreModule* camerasModule =
    qSlicerCoreApplication::application()->moduleManager()->module("Cameras");
  if (camerasModule)
    {
    vtkSlicerCamerasModuleLogic* camerasLogic = vtkSlicerCamerasModuleLogic::SafeDownCast(camerasModule->logic());
    this->LookingGlassViewWidget->setCamerasLogic(camerasLogic);
    }
  else
    {
    qWarning() << "Cameras module is not found";
    }

//  qSlicerLookingGlassModuleWidget* moduleWidget =
//    dynamic_cast<qSlicerLookingGlassModuleWidget*>(q->widgetRepresentation());
//  if (moduleWidget)
//    {
//    QObject::connect(this->LookingGlassViewWidget, SIGNAL(physicalToWorldMatrixModified()),
//                     moduleWidget, SLOT(onPhysicalToWorldMatrixModified()));
//    }
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModulePrivate::addToolBar()
{
  Q_Q(qSlicerLookingGlassModule);

  QMainWindow* mainWindow = qSlicerApplication::application()->mainWindow();
  if (mainWindow == nullptr)
    {
    qDebug("qSlicerLookingGlassModulePrivate::addToolBar: no main window is available, toolbar is not added");
    return;
    }

  if (!this->ToolBar)
    {
    this->ToolBar = new QToolBar;
    this->ToolBar->setWindowTitle(QObject::tr("Looking Glass"));
    this->ToolBar->setObjectName("LookingGlassToolBar");

    this->LookingGlassToggleAction = this->ToolBar->addAction(QObject::tr("Show scene in looking glass."));
    this->LookingGlassToggleAction->setIcon(QIcon(":/Icons/LookingGlass.png"));
    this->LookingGlassToggleAction->setCheckable(true);
    QObject::connect(this->LookingGlassToggleAction, SIGNAL(toggled(bool)),
                     q, SLOT(enableLookingGlass(bool)));

    this->UpdateViewFromReferenceViewCameraAction = this->ToolBar->addAction(QObject::tr("Set looking glass view to match reference view."));
    this->UpdateViewFromReferenceViewCameraAction->setIcon(QIcon(":/Icons/ViewCenter.png"));
    QObject::connect(this->UpdateViewFromReferenceViewCameraAction, SIGNAL(triggered()),
                     q, SLOT(updateViewFromReferenceViewCamera()));

    this->ToolBar->addSeparator();

    this->ConfigureAction = this->ToolBar->addAction(QObject::tr("Configure looking glass settings."));
    this->ConfigureAction->setIcon(QIcon(":/Icons/Small/SlicerConfigure.png"));
    QObject::connect(this->ConfigureAction, SIGNAL(triggered()),
                     q, SLOT(switchToLookingGlassModule()));

    this->ToolBar->addSeparator();

    mainWindow->addToolBar(this->ToolBar);
    }

  // Add toolbar show/hide option to menu
  foreach (QMenu* toolBarMenu, mainWindow->findChildren<QMenu*>())
    {
    if (toolBarMenu->objectName() == QString("WindowToolBarsMenu"))
      {
      toolBarMenu->addAction(this->ToolBar->toggleViewAction());
      break;
      }
    }

  // Main window takes care of saving and restoring toolbar geometry and state.
  // However, when state is restored the virtual reality toolbar was not created yet.
  // We need to restore the main window state again, now, that the Sequences toolbar is available.
  QSettings settings;
  settings.beginGroup("MainWindow");
  bool restore = settings.value("RestoreGeometry", false).toBool();
  if (restore)
    {
    mainWindow->restoreState(settings.value("windowState").toByteArray());
    }
  settings.endGroup();
}


//-----------------------------------------------------------------------------
void qSlicerLookingGlassModulePrivate::updateToolBar()
{
  Q_Q(qSlicerLookingGlassModule);

  if (!this->LookingGlassToggleAction)
    {
    return;
    }

  vtkMRMLLookingGlassViewNode* lgViewNode = this->logic()->GetLookingGlassViewNode();

  bool wasBlocked = this->LookingGlassToggleAction->blockSignals(true);
  if (!lgViewNode)
    {
    this->LookingGlassToggleAction->setChecked(false);
    this->LookingGlassToggleAction->setIcon(QIcon(":/Icons/LookingGlassHeadset.png"));
    }
  else
    {
    this->LookingGlassToggleAction->setChecked(lgViewNode->GetVisibility() && lgViewNode->GetActive());
    if (lgViewNode->HasError())
      {
      this->LookingGlassToggleAction->setIcon(QIcon(":/Icons/LookingGlassHeadsetError.png"));
      }
    else
      {
      this->LookingGlassToggleAction->setIcon(QIcon(":/Icons/LookingGlassHeadset.png"));
      }
    }
  this->LookingGlassToggleAction->blockSignals(wasBlocked);

  this->UpdateViewFromReferenceViewCameraAction->setEnabled(this->LookingGlassToggleAction->isChecked());

  this->ToolBar->removeAction(this->Spacer);

  QWidget* widget = new QWidget();
  widget->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  this->Spacer = this->ToolBar->addWidget(widget);
}

//-----------------------------------------------------------------------------
// qSlicerLookingGlassModulePrivate methods

//-----------------------------------------------------------------------------
qSlicerLookingGlassModulePrivate::qSlicerLookingGlassModulePrivate(qSlicerLookingGlassModule& object)
  : q_ptr(&object)
  , ToolBar(nullptr)
  , LookingGlassToggleAction(nullptr)
  , UpdateViewFromReferenceViewCameraAction(nullptr)
  , ConfigureAction(nullptr)
  , LookingGlassViewWidget(nullptr)
  , Spacer(nullptr)
{
}

//-----------------------------------------------------------------------------
qSlicerLookingGlassModulePrivate::~qSlicerLookingGlassModulePrivate()
{
  if (this->LookingGlassViewWidget != nullptr)
    {
    delete this->LookingGlassViewWidget;
    }
}

//-----------------------------------------------------------------------------
// qSlicerLookingGlassModule methods

//-----------------------------------------------------------------------------
qSlicerLookingGlassModule::qSlicerLookingGlassModule(QObject* _parent)
  : Superclass(_parent)
  , d_ptr(new qSlicerLookingGlassModulePrivate(*this))
{
}

//-----------------------------------------------------------------------------
qSlicerLookingGlassModule::~qSlicerLookingGlassModule()
{
}

//-----------------------------------------------------------------------------
QString qSlicerLookingGlassModule::helpText() const
{
  return "This is a loadable module that can be bundled in an extension";
}

//-----------------------------------------------------------------------------
QString qSlicerLookingGlassModule::acknowledgementText() const
{
  return "This work was partially funded by NIH grant NXNNXXNNNNNN-NNXN";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLookingGlassModule::contributors() const
{
  QStringList moduleContributors;
  moduleContributors << QString("John Doe (AnyWare Corp.)");
  return moduleContributors;
}

//-----------------------------------------------------------------------------
QIcon qSlicerLookingGlassModule::icon() const
{
  return QIcon(":/Icons/LookingGlass.png");
}

//-----------------------------------------------------------------------------
QStringList qSlicerLookingGlassModule::categories() const
{
  return QStringList() << "Holographic Display";
}

//-----------------------------------------------------------------------------
QStringList qSlicerLookingGlassModule::dependencies() const
{
  return QStringList() << "Cameras" << "VolumeRendering";
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModule::setup()
{
  Q_D(qSlicerLookingGlassModule);

  this->Superclass::setup();

  d->addToolBar();
  d->addViewWidget();

  // Set volume rendering logic to LookingGlass logic
  vtkSlicerLookingGlassLogic* vrLogic = vtkSlicerLookingGlassLogic::SafeDownCast(this->logic());
  qSlicerAbstractCoreModule* volumeRenderingModule =
    qSlicerCoreApplication::application()->moduleManager()->module("VolumeRendering");
  if (volumeRenderingModule)
    {
    vtkSlicerVolumeRenderingLogic* volumeRenderingLogic = vtkSlicerVolumeRenderingLogic::SafeDownCast(volumeRenderingModule->logic());
    vrLogic->SetVolumeRenderingLogic(volumeRenderingLogic);
    }
  else
    {
    qWarning() << "Volume rendering module is not found";
    }

  // If LookingGlass logic is modified it indicates that the view node may changed
  qvtkConnect(vrLogic, vtkCommand::ModifiedEvent, this, SLOT(onViewNodeModified()));
}

//-----------------------------------------------------------------------------
qSlicerAbstractModuleRepresentation* qSlicerLookingGlassModule
::createWidgetRepresentation()
{
  return new qSlicerLookingGlassModuleWidget;
}

//-----------------------------------------------------------------------------
vtkMRMLAbstractLogic* qSlicerLookingGlassModule::createLogic()
{
  return vtkSlicerLookingGlassLogic::New();
}

//-----------------------------------------------------------------------------
QToolBar* qSlicerLookingGlassModule::toolBar()
{
  Q_D(qSlicerLookingGlassModule);
  return d->ToolBar;
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModule::setToolBarVisible(bool visible)
{
  Q_D(qSlicerLookingGlassModule);
  d->ToolBar->setVisible(visible);
}

//-----------------------------------------------------------------------------
bool qSlicerLookingGlassModule::isToolBarVisible()
{
  Q_D(qSlicerLookingGlassModule);
  return d->ToolBar->isVisible();
}

// --------------------------------------------------------------------------
qMRMLLookingGlassView* qSlicerLookingGlassModule::viewWidget()
{
  Q_D(qSlicerLookingGlassModule);
  return d->LookingGlassViewWidget;
}

// --------------------------------------------------------------------------
void qSlicerLookingGlassModule::enableLookingGlass(bool enable)
{
  Q_D(qSlicerLookingGlassModule);
  QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
  d->logic()->SetLookingGlassActive(enable);
  QApplication::restoreOverrideCursor();
}

// --------------------------------------------------------------------------
void qSlicerLookingGlassModule::updateViewFromReferenceViewCamera()
{
  Q_D(qSlicerLookingGlassModule);
  if (d->LookingGlassViewWidget == nullptr)
    {
    return;
    }
  d->LookingGlassViewWidget->updateViewFromReferenceViewCamera();
}

// --------------------------------------------------------------------------
void qSlicerLookingGlassModule::switchToLookingGlassModule()
{
  Q_D(qSlicerLookingGlassModule);
  if (!qSlicerApplication::application()
      || !qSlicerApplication::application()->moduleManager())
    {
    return;
    }
  qSlicerAbstractCoreModule* module = qSlicerApplication::application()->moduleManager()->module("LookingGlass");
  qSlicerAbstractModule* moduleWithAction = qobject_cast<qSlicerAbstractModule*>(module);
  if (!module)
    {
    return;
    }
  moduleWithAction->action()->trigger();
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModule::setMRMLScene(vtkMRMLScene* scene)
{
  this->Superclass::setMRMLScene(scene);

  vtkSlicerLookingGlassLogic* logic = vtkSlicerLookingGlassLogic::SafeDownCast(this->logic());
  if (!logic)
  {
    qCritical() << Q_FUNC_INFO << " failed: logic is invalid";
    return;
  }

  vtkMRMLLookingGlassViewNode* defaultViewNode = logic->GetDefaultLookingGlassViewNode();
  if (!defaultViewNode)
  {
    qCritical() << Q_FUNC_INFO << " failed: defaultViewNode is invalid";
    return;
  }
  QSettings settings;
  settings.beginGroup("Default3DView");
  if (settings.contains("UseDepthPeeling"))
  {
    defaultViewNode->SetUseDepthPeeling(settings.value("UseDepthPeeling").toBool());
  }
}

// --------------------------------------------------------------------------
void qSlicerLookingGlassModule::onViewNodeModified()
{
  Q_D(qSlicerLookingGlassModule);
  vtkMRMLLookingGlassViewNode* lgViewNode = d->logic()->GetLookingGlassViewNode();

  // Update view node in view widget
  if (d->LookingGlassViewWidget != nullptr)
    {
    vtkMRMLLookingGlassViewNode* oldVrViewNode = d->LookingGlassViewWidget->mrmlLookingGlassViewNode();
    if (oldVrViewNode != lgViewNode)
      {
      d->logic()->SetDefaultReferenceView();
      }
    d->LookingGlassViewWidget->setMRMLLookingGlassViewNode(lgViewNode);
  }

  // Update toolbar
  d->updateToolBar();
}
