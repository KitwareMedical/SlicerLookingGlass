/*==============================================================================

  Program: 3D Slicer

  Copyright (c) Kitware Inc.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Jean-Christophe Fillion-Robin, Kitware Inc.
  and was partially funded by NIH grant 3P41RR013218-12S1

==============================================================================*/

#include "qMRMLLookingGlassView_p.h"

// Qt includes
#include <QCoreApplication>
#include <QDebug>
#include <QEvent>
#include <QFileInfo>
#include <QHBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QToolButton>
#include <QTimer>

// CTK includes
#include <ctkAxesWidget.h>
#include <ctkPimpl.h>

// qMRML includes
#include "qMRMLColors.h"
#include "qMRMLThreeDView.h"
#include "qMRMLThreeDWidget.h"

// Slicer includes
#include "qSlicerApplication.h"
#include "qSlicerLayoutManager.h"
#include "vtkSlicerConfigure.h" // For Slicer_USE_OpenVR
#include "vtkSlicerCamerasModuleLogic.h"
#include <vtkSlicerVersionConfigure.h> // For Slicer_VERSION_MAJOR, Slicer_VERSION_MINOR 

// Slicer LookingGlass includes
#include "vtkMRMLLookingGlassViewNode.h"

// MRMLDisplayableManager includes
#include <vtkMRMLAbstractDisplayableManager.h>
#include <vtkMRMLDisplayableManagerGroup.h>
#include <vtkMRMLLookingGlassViewDisplayableManagerFactory.h>
#include <vtkMRMLThreeDViewInteractorStyle.h>

// MRML includes
#include <vtkMRMLCameraNode.h>
#include <vtkMRMLLinearTransformNode.h>
#include <vtkMRMLScene.h>

// VTK LookingGlass includes
#include <vtkLookingGlassInterface.h>

// VTK includes
#include <vtkCamera.h>
#include <vtkCollection.h>
#include <vtkCullerCollection.h>
#include <vtkMathUtilities.h>
#include <vtkNew.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkRenderingOpenGLConfigure.h> // For VTK_USE_X, VTK_USE_COCOA
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>
#if defined(VTK_USE_X)
# include <vtkXLookingGlassRenderWindow.h>
#elif defined(Q_OS_WIN)
# include <vtkWin32LookingGlassRenderWindow.h>
#elif defined(VTK_USE_COCOA)
# include <vtkCocoaLookingGlassRenderWindow.h>
#endif

//--------------------------------------------------------------------------
// qMRMLLookingGlassViewPrivate methods

//---------------------------------------------------------------------------
qMRMLLookingGlassViewPrivate::qMRMLLookingGlassViewPrivate(qMRMLLookingGlassView& object)
  : q_ptr(&object)
  , CamerasLogic(nullptr)
  , ReferenceViewInteractive(false)
  , RequestTimer(nullptr)
{
  this->MRMLLookingGlassViewNode = nullptr;
}

//---------------------------------------------------------------------------
qMRMLLookingGlassViewPrivate::~qMRMLLookingGlassViewPrivate()
{
}

//---------------------------------------------------------------------------
void qMRMLLookingGlassViewPrivate::init()
{
  Q_Q(qMRMLLookingGlassView);

  QObject::connect(&this->LookingGlassLoopTimer, SIGNAL(timeout()),
    q, SLOT(scheduleRender()));

  this->RequestTimer = new QTimer(q);
  this->RequestTimer->setSingleShot(true);
  QObject::connect(this->RequestTimer, SIGNAL(timeout()),
                   q, SLOT(requestRender()));
}

//----------------------------------------------------------------------------
CTK_SET_CPP(qMRMLLookingGlassView, vtkSlicerCamerasModuleLogic*, setCamerasLogic, CamerasLogic);
CTK_GET_CPP(qMRMLLookingGlassView, vtkSlicerCamerasModuleLogic*, camerasLogic, CamerasLogic);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLookingGlassView, vtkRenderer*, renderer, Renderer);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLookingGlassView, vtkOpenGLRenderWindow*, renderWindow, RenderWindow);

//----------------------------------------------------------------------------
vtkLookingGlassInterface* qMRMLLookingGlassView::lookingGlassTnterface()const
{
  Q_D(const qMRMLLookingGlassView);
#if defined(VTK_USE_X)
  vtkXLookingGlassRenderWindow* renderWindow = vtkXLookingGlassRenderWindow::SafeDownCast(d->RenderWindow);
  return renderWindow->GetInterface();
#elif defined(Q_OS_WIN)
  vtkWin32LookingGlassRenderWindow* renderWindow = vtkWin32LookingGlassRenderWindow::SafeDownCast(d->RenderWindow);
  return renderWindow->GetInterface();
#elif defined(VTK_USE_COCOA)
  vtkCocoaLookingGlassRenderWindow* renderWindow = vtkCocoaLookingGlassRenderWindow::SafeDownCast(d->RenderWindow);
  return renderWindow->GetInterface();
#else
  return nullptr;
#endif
}

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLookingGlassView, vtkRenderWindowInteractor*, interactor, Interactor);

//---------------------------------------------------------------------------
void qMRMLLookingGlassViewPrivate::createRenderWindow()
{
  Q_Q(qMRMLLookingGlassView);

  this->LastViewUpdateTime = vtkSmartPointer<vtkTimerLog>::New();
  this->LastViewUpdateTime->StartTimer();
  this->LastViewUpdateTime->StopTimer();
  this->LastViewDirection[0] = 0.0;
  this->LastViewDirection[1] = 0.0;
  this->LastViewDirection[2] = 1.0;
  this->LastViewUp[0] = 0.0;
  this->LastViewUp[1] = 1.0;
  this->LastViewUp[2] = 0.0;
  this->LastViewPosition[0] = 0.0;
  this->LastViewPosition[1] = 0.0;
  this->LastViewPosition[2] = 0.0;

  this->RenderWindow = vtkSmartPointer<vtkOpenGLRenderWindow>::Take(
        vtkLookingGlassInterface::CreateLookingGlassRenderWindow());

  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
  this->Interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  this->InteractorStyle = vtkSmartPointer<vtkMRMLThreeDViewInteractorStyle>::New();

  this->Interactor->SetInteractorStyle(this->InteractorStyle);
  this->InteractorStyle->SetInteractor(this->Interactor);
  this->InteractorStyle->SetCurrentRenderer(this->Renderer);

  this->Camera = vtkSmartPointer<vtkCamera>::New();
  this->Renderer->SetActiveCamera(this->Camera);
  this->RenderWindow->SetMultiSamples(0);
  this->RenderWindow->AddRenderer(this->Renderer);
  this->RenderWindow->SetInteractor(this->Interactor);

  // The interactor never calls Render() on the render window.
  this->Interactor->SetEnableRender(false);

  // Ensure this view catches all render requests and ensure the desired framerate
  this->qvtkReconnect(this->RenderWindow->GetInteractor(), this->Interactor,
                vtkCommand::RenderEvent, q, SLOT(scheduleRender()));

  vtkMRMLLookingGlassViewDisplayableManagerFactory* factory
    = vtkMRMLLookingGlassViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers
      << "vtkMRMLCameraDisplayableManager"
  //<< "vtkMRMLViewDisplayableManager"
      << "vtkMRMLModelDisplayableManager"
      << "vtkMRMLThreeDReformatDisplayableManager"
      << "vtkMRMLCrosshairDisplayableManager3D"
      << "vtkMRMLOrientationMarkerDisplayableManager"
      << "vtkMRMLRulerDisplayableManager"
      << "vtkMRMLAnnotationDisplayableManager"
      << "vtkMRMLMarkupsDisplayableManager"
      << "vtkMRMLSegmentationsDisplayableManager3D"
      << "vtkMRMLTransformsDisplayableManager3D"
      << "vtkMRMLLinearTransformsDisplayableManager3D"
      << "vtkMRMLVolumeRenderingDisplayableManager"
      ;

  foreach (const QString& displayableManager, displayableManagers)
  {
    if (!factory->IsDisplayableManagerRegistered(displayableManager.toLatin1()))
    {
      factory->RegisterDisplayableManager(displayableManager.toLatin1());
    }
  }

  this->DisplayableManagerGroup = vtkSmartPointer<vtkMRMLDisplayableManagerGroup>::Take(
                                    factory->InstantiateDisplayableManagers(q->renderer()));
  this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLLookingGlassViewNode);

//  ///CONFIGURATION OF THE OPENVR ENVIRONEMENT

//  this->Renderer->RemoveCuller(this->Renderer->GetCullers()->GetLastItem());
//  this->Renderer->SetBackground(0.7, 0.7, 0.7);

  q->updateViewFromReferenceViewCamera();

  this->RenderWindow->Initialize();
  this->Renderer->ResetCamera();

  // Observe displayable manager group to catch RequestRender events
  this->qvtkConnect(this->DisplayableManagerGroup, vtkCommand::UpdateEvent,
    q, SLOT(scheduleRender()));
}

//---------------------------------------------------------------------------
void qMRMLLookingGlassViewPrivate::destroyRenderWindow()
{
  Q_Q(qMRMLLookingGlassView);
  this->LookingGlassLoopTimer.stop();
  // Must break the connection between interactor and render window,
  // otherwise they would circularly refer to each other and would not
  // be deleted.
  this->Interactor->SetRenderWindow(nullptr);
  this->Interactor = nullptr;
  this->InteractorStyle = nullptr;
  this->DisplayableManagerGroup = nullptr;
  this->Renderer = nullptr;
  this->Camera = nullptr;
  this->RenderWindow = nullptr;
}

// --------------------------------------------------------------------------
void qMRMLLookingGlassViewPrivate::updateWidgetFromMRML()
{
  Q_Q(qMRMLLookingGlassView);
  if (!this->MRMLLookingGlassViewNode || !this->MRMLLookingGlassViewNode->GetVisibility())
  {
    if (this->RenderWindow != nullptr)
    {
      this->destroyRenderWindow();
    }
    if (this->MRMLLookingGlassViewNode)
    {
      this->MRMLLookingGlassViewNode->ClearError();
    }
    return;
  }

  if (!this->RenderWindow)
  {
    QApplication::setOverrideCursor(QCursor(Qt::BusyCursor));
    this->createRenderWindow();
    QApplication::restoreOverrideCursor();
    if (!q->isHardwareConnected())
    {
      this->MRMLLookingGlassViewNode->SetError("Connection failed");
      return;
    }
    this->MRMLLookingGlassViewNode->ClearError();
  }

  if (this->DisplayableManagerGroup->GetMRMLDisplayableNode() != this->MRMLLookingGlassViewNode.GetPointer())
  {
    this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLLookingGlassViewNode);
  }

  // Renderer properties
  this->Renderer->SetGradientBackground(1);
  this->Renderer->SetBackground(this->MRMLLookingGlassViewNode->GetBackgroundColor());
  this->Renderer->SetBackground2(this->MRMLLookingGlassViewNode->GetBackgroundColor2());
  this->Renderer->SetUseDepthPeeling(this->MRMLLookingGlassViewNode->GetUseDepthPeeling() != 0);
  this->Renderer->SetUseDepthPeelingForVolumes(this->MRMLLookingGlassViewNode->GetUseDepthPeeling() != 0);

  // Render window properties
  if (this->RenderWindow)
  {
    // Desired update rate
    this->RenderWindow->SetDesiredUpdateRate(this->desiredUpdateRate());

    vtkMRMLCameraNode* cameraNode = this->CamerasLogic->GetViewActiveCameraNode(this->MRMLLookingGlassViewNode);
    if (!cameraNode || !cameraNode->GetCamera())
      {
      qWarning() << Q_FUNC_INFO << " failed: camera node is not found";
      return;
      }

    // Use clipping limits
    q->lookingGlassTnterface()->SetUseClippingLimits(this->MRMLLookingGlassViewNode->GetUseClippingLimits());

    // Near range limit
    double previousNearClippingLimit = q->lookingGlassTnterface()->GetNearClippingLimit();
    q->lookingGlassTnterface()->SetNearClippingLimit(this->MRMLLookingGlassViewNode->GetNearClippingLimit());
    if (this->MRMLLookingGlassViewNode->GetUseClippingLimits() &&
        !vtkMathUtilities::FuzzyCompare<double>(previousNearClippingLimit, this->MRMLLookingGlassViewNode->GetNearClippingLimit()))
      {
      // Trigger re-render
      cameraNode->Modified();
      }

    // Far range limit
    double previousFarClippingLimit = q->lookingGlassTnterface()->GetFarClippingLimit();
    q->lookingGlassTnterface()->SetFarClippingLimit(this->MRMLLookingGlassViewNode->GetFarClippingLimit());
    if (this->MRMLLookingGlassViewNode->GetUseClippingLimits() &&
        !vtkMathUtilities::FuzzyCompare<double>(previousFarClippingLimit, this->MRMLLookingGlassViewNode->GetFarClippingLimit()))
      {
      // Trigger re-render
      cameraNode->Modified();
      }
  }

  if (this->MRMLLookingGlassViewNode->GetActive())
    {
    this->LookingGlassLoopTimer.start(0);
    }
  else
    {
    this->LookingGlassLoopTimer.stop();
    }
}

//---------------------------------------------------------------------------
double qMRMLLookingGlassViewPrivate::desiredUpdateRate()
{
  double rate = this->MRMLLookingGlassViewNode->GetDesiredUpdateRate();

  // enforce non-zero frame rate to avoid division by zero errors
  const double defaultStaticViewUpdateRate = 0.0001;
  if (rate < defaultStaticViewUpdateRate)
  {
    rate = defaultStaticViewUpdateRate;
  }

  return rate;
}

// --------------------------------------------------------------------------
// qMRMLLookingGlassView methods

// --------------------------------------------------------------------------
qMRMLLookingGlassView::qMRMLLookingGlassView(QWidget* _parent) : Superclass(_parent)
  , d_ptr(new qMRMLLookingGlassViewPrivate(*this))
{
  Q_D(qMRMLLookingGlassView);
  d->init();
}

// --------------------------------------------------------------------------
qMRMLLookingGlassView::~qMRMLLookingGlassView()
{
}

//------------------------------------------------------------------------------
void qMRMLLookingGlassView::addDisplayableManager(const QString& displayableManagerName)
{
  Q_D(qMRMLLookingGlassView);
  vtkSmartPointer<vtkMRMLAbstractDisplayableManager> displayableManager;
  displayableManager.TakeReference(
    vtkMRMLDisplayableManagerGroup::InstantiateDisplayableManager(
      displayableManagerName.toLatin1()));
  d->DisplayableManagerGroup->AddDisplayableManager(displayableManager);
}

//------------------------------------------------------------------------------
void qMRMLLookingGlassView::setMRMLLookingGlassViewNode(vtkMRMLLookingGlassViewNode* newViewNode)
{
  Q_D(qMRMLLookingGlassView);
  if (d->MRMLLookingGlassViewNode == newViewNode)
  {
    return;
  }

  d->qvtkReconnect(
    d->MRMLLookingGlassViewNode, newViewNode,
    vtkCommand::ModifiedEvent, d, SLOT(updateWidgetFromMRML()));

  d->MRMLLookingGlassViewNode = newViewNode;

  d->updateWidgetFromMRML();

  // Enable/disable widget
  this->setEnabled(newViewNode != nullptr);
}

//---------------------------------------------------------------------------
void qMRMLLookingGlassView::pushFocalPlaneBack()
{
  Q_D(qMRMLLookingGlassView);

  vtkMRMLCameraNode* cameraNode = d->CamerasLogic->GetViewActiveCameraNode(d->MRMLLookingGlassViewNode);
  if (!cameraNode || !cameraNode->GetCamera())
    {
    qWarning() << Q_FUNC_INFO << " failed: camera node is not found";
    return;
    }

  double fp[3] = {0., 0., 0.};
  cameraNode->GetFocalPoint(fp);

  double pos[3] = {0., 0., 0.};
  cameraNode->GetPosition(pos);

  double dx = fp[0] - pos[0];
  double dy = fp[1] - pos[1];
  double dz = fp[2] - pos[2];
  double distance = sqrt(dx * dx + dy * dy + dz * dz);

  double directionOfProjection[3];
  directionOfProjection[0] = dx / distance;
  directionOfProjection[1] = dy / distance;
  directionOfProjection[2] = dz / distance;

  // TODO Add slider widget (min: 0.05, max: 0.5)
  // Set how far to move the focal plane when pushing or pulling it.
  // It is a factor applied to the near or far clipping limits.
  double focalPlaneMovementFactor = 0.2;

  // Set a limit for the ratio of the far clipping plane to the focal
  // distance.
  double farClippingLimit = d->MRMLLookingGlassViewNode->GetFarClippingLimit();

  distance += focalPlaneMovementFactor * distance * (farClippingLimit - 1.0);

  fp[0] = pos[0] + directionOfProjection[0] * distance;
  fp[1] = pos[1] + directionOfProjection[1] * distance;
  fp[2] = pos[2] + directionOfProjection[2] * distance;
  cameraNode->SetFocalPoint(fp);

  cameraNode->Modified();
}

//---------------------------------------------------------------------------
void qMRMLLookingGlassView::pullFocalPlaneForward()
{
  Q_D(qMRMLLookingGlassView);

  vtkMRMLCameraNode* cameraNode = d->CamerasLogic->GetViewActiveCameraNode(d->MRMLLookingGlassViewNode);
  if (!cameraNode || !cameraNode->GetCamera())
    {
    qWarning() << Q_FUNC_INFO << " failed: camera node is not found";
    return;
    }

  double fp[3] = {0., 0., 0.};
  cameraNode->GetFocalPoint(fp);

  double pos[3] = {0., 0., 0.};
  cameraNode->GetPosition(pos);

  double dx = fp[0] - pos[0];
  double dy = fp[1] - pos[1];
  double dz = fp[2] - pos[2];
  double distance = sqrt(dx * dx + dy * dy + dz * dz);

  double directionOfProjection[3];
  directionOfProjection[0] = dx / distance;
  directionOfProjection[1] = dy / distance;
  directionOfProjection[2] = dz / distance;

  // TODO Add slider widget (see above)
  double focalPlaneMovementFactor = 0.2;

  // Set a limit for the ratio of the near clipping plane to the focal
  // distance.
  double nearClippingLimit = d->MRMLLookingGlassViewNode->GetNearClippingLimit();

  distance -= focalPlaneMovementFactor * distance * (1.0 - nearClippingLimit);

  fp[0] = pos[0] + directionOfProjection[0] * distance;
  fp[1] = pos[1] + directionOfProjection[1] * distance;
  fp[2] = pos[2] + directionOfProjection[2] * distance;
  cameraNode->SetFocalPoint(fp);

  cameraNode->Modified();
}

//---------------------------------------------------------------------------
vtkMRMLLookingGlassViewNode* qMRMLLookingGlassView::mrmlLookingGlassViewNode()const
{
  Q_D(const qMRMLLookingGlassView);
  return d->MRMLLookingGlassViewNode;
}

//------------------------------------------------------------------------------
void qMRMLLookingGlassView::getDisplayableManagers(vtkCollection* displayableManagers)
{
  Q_D(qMRMLLookingGlassView);

  if (!displayableManagers || !d->DisplayableManagerGroup)
  {
    return;
  }
  int num = d->DisplayableManagerGroup->GetDisplayableManagerCount();
  for (int n = 0; n < num; n++)
  {
    displayableManagers->AddItem(d->DisplayableManagerGroup->GetNthDisplayableManager(n));
  }
}

//------------------------------------------------------------------------------
bool qMRMLLookingGlassView::isHardwareConnected()const
{
  vtkOpenGLRenderWindow* renWin = this->renderWindow();
  if (!renWin)
  {
    return false;
  }
  // connected successfully
  return true;
}

//---------------------------------------------------------------------------
bool qMRMLLookingGlassView::isReferenceViewInteractive() const
{
  Q_D(const qMRMLLookingGlassView);
  return d->ReferenceViewInteractive;
}

//---------------------------------------------------------------------------
void qMRMLLookingGlassView::setReferenceViewInteractive(bool interactive)
{
  Q_D(qMRMLLookingGlassView);
  d->ReferenceViewInteractive = interactive;
}

//---------------------------------------------------------------------------
void qMRMLLookingGlassView::updateViewFromReferenceViewCamera()
{
  Q_D(qMRMLLookingGlassView);
  if (!d->MRMLLookingGlassViewNode)
    {
    return;
    }
  vtkMRMLViewNode* referenceViewNode = d->MRMLLookingGlassViewNode->GetReferenceViewNode();
  if (!referenceViewNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: no reference view node is set";
    return;
    }
  if (!d->CamerasLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: cameras module logic is not set";
    return;
    }
  vtkMRMLCameraNode* cameraNode = d->CamerasLogic->GetViewActiveCameraNode(referenceViewNode);
  if (!cameraNode || !cameraNode->GetCamera())
    {
    qWarning() << Q_FUNC_INFO << " failed: reference view camera node is not found";
    return;
    }
  vtkMRMLCameraNode* lgCameraNode = d->CamerasLogic->GetViewActiveCameraNode(d->MRMLLookingGlassViewNode);
  if (!lgCameraNode || !lgCameraNode->GetCamera())
    {
    qWarning() << Q_FUNC_INFO << " failed: looking glass camera node is not found";
    return;
    }
  lgCameraNode->CopyContent(cameraNode);
}

//----------------------------------------------------------------------------
void qMRMLLookingGlassView::scheduleRender()
{
  Q_D(qMRMLLookingGlassView);

  //logger.trace(QString("scheduleRender - RenderEnabled: %1 - Request render elapsed: %2ms").
  //             arg(d->RenderEnabled ? "true" : "false")
  //             .arg(d->RequestTime.elapsed()));

  if (!d->MRMLLookingGlassViewNode->GetActive())
    {
    return;
    }

  if (d->MRMLLookingGlassViewNode->GetRenderingMode() == vtkMRMLLookingGlassViewNode::RenderingModeOnlyWhenRequested)
    {
    return;
    }

  if (d->MRMLLookingGlassViewNode->GetRenderingMode() == vtkMRMLLookingGlassViewNode::RenderingModeOnlyStillRenders
    && this->isReferenceViewInteractive())
    {
    return;
    }

  this->updateViewFromReferenceViewCamera();

  double msecsBeforeRender = 0;
  // If the MaximumUpdateRate is set to 0 then it indicates that rendering is done next time
  // the application is idle.
  if (d->MRMLLookingGlassViewNode->GetDesiredUpdateRate() > 0.0)
    {
    msecsBeforeRender = 1000. / d->MRMLLookingGlassViewNode->GetDesiredUpdateRate();
    }
//  if(d->VTKWidget->testAttribute(Qt::WA_WState_InPaintEvent))
//    {
//    // If the request comes from the system (widget exposed, resized...), the
//    // render must be done immediately.
//    this->requestRender();
//    }
//  else
  if (!d->RequestTime.isValid())
    {
    d->RequestTime.start();
    d->RequestTimer->start(static_cast<int>(msecsBeforeRender));
    }
  else if (d->RequestTime.elapsed() > msecsBeforeRender)
    {
    // The rendering hasn't still be done, but msecsBeforeRender milliseconds
    // have already been elapsed, it is likely that RequestTimer has already
    // timed out, but the event queue hasn't been processed yet, rendering is
    // done now to ensure the desired framerate is respected.
    this->requestRender();
    }
}

//----------------------------------------------------------------------------
void qMRMLLookingGlassView::requestRender()
{
  Q_D(const qMRMLLookingGlassView);

//  if (this->isRenderPaused())
//    {
//    return;
//    }
  this->forceRender();
}

//----------------------------------------------------------------------------
void qMRMLLookingGlassView::forceRender()
{
  Q_D(qMRMLLookingGlassView);

  if (this->sender() == d->RequestTimer  &&
      !d->RequestTime.isValid())
    {
    // The slot associated to the timeout signal is now called, however the
    // render has already been executed meanwhile. There is no need to do it
    // again.
    return;
    }

  // The timer can be stopped if it hasn't timed out yet.
  d->RequestTimer->stop();
  d->RequestTime = QTime();

  //logger.trace(QString("forceRender - RenderEnabled: %1")
  //             .arg(d->RenderEnabled ? "true" : "false"));

  if (!d->MRMLLookingGlassViewNode->GetActive() || !d->MRMLLookingGlassViewNode->GetVisibility())
    {
    return;
    }
  d->RenderWindow->Render();
}

////----------------------------------------------------------------------------
//double qMRMLLookingGlassView::maximumUpdateRate()const
//{
//  Q_D(const qMRMLLookingGlassView);
//  return d->MaximumUpdateRate;
//}

////----------------------------------------------------------------------------
//void qMRMLLookingGlassView::setMaximumUpdateRate(double fps)
//{
//  Q_D(qMRMLLookingGlassView);
//  d->MaximumUpdateRate = fps;
//}
