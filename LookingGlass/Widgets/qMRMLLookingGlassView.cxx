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
#include <vtkNew.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkOpenGLRenderWindow.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

//--------------------------------------------------------------------------
// qMRMLLookingGlassViewPrivate methods

//---------------------------------------------------------------------------
qMRMLLookingGlassViewPrivate::qMRMLLookingGlassViewPrivate(qMRMLLookingGlassView& object)
  : q_ptr(&object)
  , CamerasLogic(nullptr)
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
//  QObject::connect(&this->LookingGlassLoopTimer, SIGNAL(timeout()), this, SLOT(doLookingGlass()));
}

//----------------------------------------------------------------------------
CTK_SET_CPP(qMRMLLookingGlassView, vtkSlicerCamerasModuleLogic*, setCamerasLogic, CamerasLogic);
CTK_GET_CPP(qMRMLLookingGlassView, vtkSlicerCamerasModuleLogic*, camerasLogic, CamerasLogic);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLookingGlassView, vtkRenderer*, renderer, Renderer);

//----------------------------------------------------------------------------
CTK_GET_CPP(qMRMLLookingGlassView, vtkOpenGLRenderWindow*, renderWindow, RenderWindow);

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
//  this->Interactor->SetInteractorStyle(this->InteractorStyle);
//  this->InteractorStyle->SetInteractor(this->Interactor);
//  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->Camera = vtkSmartPointer<vtkCamera>::New();
  this->Renderer->SetActiveCamera(this->Camera);
  this->RenderWindow->SetMultiSamples(0);
  this->RenderWindow->AddRenderer(this->Renderer);
  this->RenderWindow->SetInteractor(this->Interactor);

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
      //<< "vtkMRMLRulerDisplayableManager" // Not supported in LookingGlass ?
      //<< "vtkMRMLAnnotationDisplayableManager" // Not supported in LookingGlass ?
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
//  this->LookingGlassLoopTimer.stop();
  // Must break the connection between interactor and render window,
  // otherwise they would circularly refer to each other and would not
  // be deleted.
//  this->Interactor->SetRenderWindow(nullptr);
//  this->Interactor = nullptr;
//  this->InteractorStyle = nullptr;
//  this->DisplayableManagerGroup = nullptr;
//  this->Renderer = nullptr;
//  this->Camera = nullptr;
//  this->RenderWindow = nullptr;
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
  }

//  if (this->MRMLLookingGlassViewNode->GetActive())
//  {
//    this->LookingGlassLoopTimer.start(0);
//  }
//  else
//  {
//    this->LookingGlassLoopTimer.stop();
//  }
}

//---------------------------------------------------------------------------
double qMRMLLookingGlassViewPrivate::desiredUpdateRate()
{
  Q_Q(qMRMLLookingGlassView);
  double rate = this->MRMLLookingGlassViewNode->GetDesiredUpdateRate();

  // enforce non-zero frame rate to avoid division by zero errors
  const double defaultStaticViewUpdateRate = 0.0001;
  if (rate < defaultStaticViewUpdateRate)
  {
    rate = defaultStaticViewUpdateRate;
  }

  return rate;
}

//---------------------------------------------------------------------------
double qMRMLLookingGlassViewPrivate::stillUpdateRate()
{
  Q_Q(qMRMLLookingGlassView);
  return 0.0001;
}

// --------------------------------------------------------------------------
//void qMRMLLookingGlassViewPrivate::doLookingGlass()
//{
//  if (this->Interactor && this->RenderWindow /*&& this->RenderWindow->GetHMD()*/ && this->Renderer)
//  {
////    this->Interactor->DoOneEvent(this->RenderWindow, this->Renderer);

//    this->LastViewUpdateTime->StopTimer();
//    if (this->LastViewUpdateTime->GetElapsedTime() > 0.0)
//    {
////      bool quickViewMotion = true;

////      if (this->MRMLLookingGlassViewNode->GetMotionSensitivity() > 0.999)
////      {
////        quickViewMotion = true;
////      }
////      else if (this->MRMLLookingGlassViewNode->GetMotionSensitivity() <= 0.001)
////      {
////        quickViewMotion = false;
////      }
////      else if (this->LastViewUpdateTime->GetElapsedTime() < 3.0) // don't consider stale measurements
////      {
////        // limit scale:
////        // sensitivity = 0    -> limit = 10.0x
////        // sensitivity = 50%  -> limit =  1.0x
////        // sensitivity = 100% -> limit =  0.1x
////        double limitScale = pow(100, 0.5 - this->MRMLLookingGlassViewNode->GetMotionSensitivity());

////        const double angularSpeedLimitRadiansPerSec = vtkMath::RadiansFromDegrees(5.0 * limitScale);
////        double viewDirectionChangeSpeed = vtkMath::AngleBetweenVectors(this->LastViewDirection,
////                                          this->Camera->GetViewPlaneNormal()) / this->LastViewUpdateTime->GetElapsedTime();
////        double viewUpDirectionChangeSpeed = vtkMath::AngleBetweenVectors(this->LastViewUp,
////                                            this->Camera->GetViewUp()) / this->LastViewUpdateTime->GetElapsedTime();

////        const double translationSpeedLimitMmPerSec = 100.0 * limitScale;
////        // Physical scale = 100 if virtual objects are real-world size; <100 if virtual objects are larger
////        double viewTranslationSpeedMmPerSec = this->RenderWindow->GetPhysicalScale() * 0.01 *
////                                              sqrt(vtkMath::Distance2BetweenPoints(this->LastViewPosition, this->Camera->GetPosition()))
////                                              / this->LastViewUpdateTime->GetElapsedTime();

////        if (viewDirectionChangeSpeed < angularSpeedLimitRadiansPerSec
////            && viewUpDirectionChangeSpeed < angularSpeedLimitRadiansPerSec
////            && viewTranslationSpeedMmPerSec  < translationSpeedLimitMmPerSec)
////        {
////          quickViewMotion = false;
////        }
////      }

////      double updateRate = quickViewMotion ? this->desiredUpdateRate() : this->stillUpdateRate();
////      this->RenderWindow->SetDesiredUpdateRate(updateRate);

////      this->Camera->GetViewPlaneNormal(this->LastViewDirection);
////      this->Camera->GetViewUp(this->LastViewUp);
////      this->Camera->GetPosition(this->LastViewPosition);

//      this->LastViewUpdateTime->StartTimer();
//    }
//  }
//}

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

  // TODO Add slider widget (min: 0.75, max: 1.5)
  // Set a limit for the ratio of the far clipping plane to the focal
  // distance. This is a mechanism to limit parallex and resulting
  // ghosting when using the looking glass display. The typical value
  // should be around 1.2.
  double farClippingLimit = 1.2;

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

  // TODO Add slider widget (min: 0.75, max: 1.5)
  // Set a limit for the ratio of the near clipping plane to the focal
  // distance. This is a mechanism to limit parallex and resulting
  // ghosting when using the looking glass display. The typical value
  // should be around 0.8.
  double nearClippingLimit = 0.8;

  distance -= focalPlaneMovementFactor * distance * (1.0 - nearClippingLimit);

  fp[0] = pos[0] + directionOfProjection[0] * distance;
  fp[1] = pos[1] + directionOfProjection[1] * distance;
  fp[2] = pos[2] + directionOfProjection[2] * distance;
  cameraNode->SetFocalPoint(fp);

  cameraNode->Modified();
}

//---------------------------------------------------------------------------
void qMRMLLookingGlassView::scheduleRender()
{
  Q_D(qMRMLLookingGlassView);
  d->RenderWindow->Render();
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
//  if (!renWin->GetHMD())
//  {
//    return false;
//  }
  // connected successfully
  return true;
}

////------------------------------------------------------------------------------
//void qMRMLLookingGlassView::onPhysicalToWorldMatrixModified()
//{
//  Q_D(qMRMLLookingGlassView);

//  d->MRMLLookingGlassViewNode->SetMagnification(d->InteractorStyle->GetMagnification());

//  emit physicalToWorldMatrixModified();
//}

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
