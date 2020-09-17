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

// Need to be included before qMRMLLookingGlassView_p.h
#include <vtkCamera.h>
//#include <vtkLookingGlassViewInteractorStyle.h>
//#include <vtkOpenVRInteractorStyle.h> //TODO: For debugging the original interactor
//#include <vtkLookingGlassViewInteractor.h>
//#include <vtkOpenVRRenderWindowInteractor.h> //TODO: For debugging the original interactor
//#include <vtkOpenVRModel.h>
#include <vtkOpenGLRenderWindow.h>
//#include <vtkOpenVRRenderer.h>

#include <vtkLookingGlassInterface.h>

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

// LookingGlass includes
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

// VTK includes
#include <vtkCollection.h>
#include <vtkCullerCollection.h>
#include <vtkNew.h>
#include <vtkOpenGLFramebufferObject.h>
#include <vtkPolyDataMapper.h>
#include <vtkRenderer.h>
#include <vtkRendererCollection.h>
#include <vtkSmartPointer.h>
#include <vtkTimerLog.h>

//namespace
//{
//  //--------------------------------------------------------------------------
//  std::string PoseStatusToString(vr::ETrackingResult result)
//  {
//    switch (result)
//    {
//      case vr::TrackingResult_Calibrating_InProgress:
//        return "CalibratingInProgress";
//      case vr::TrackingResult_Calibrating_OutOfRange:
//        return "CalibratingOutOfRange";
//      case vr::TrackingResult_Running_OK:
//        return "RunningOk";
//      case vr::TrackingResult_Running_OutOfRange:
//        return "RunningOutOfRange";
//      case vr::TrackingResult_Uninitialized:
//      default:
//        return "Uninitialized";
//    }
//  }
//}

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

  qDebug() << "createRenderWindow";

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

  qDebug() <<  __LINE__;
  this->RenderWindow = vtkSmartPointer<vtkOpenGLRenderWindow>::Take(
        vtkLookingGlassInterface::CreateLookingGlassRenderWindow());
//  this->RenderWindow = vtkSmartPointer<vtkOpenVRRenderWindow>::New();

  qDebug() <<  __LINE__;
  this->Renderer = vtkSmartPointer<vtkRenderer>::New();
//  this->Renderer = vtkSmartPointer<vtkOpenVRRenderer>::New();

  qDebug() <<  __LINE__;
  this->Interactor = vtkSmartPointer<vtkRenderWindowInteractor>::New();
//  this->InteractorStyle = vtkSmartPointer<vtkLookingGlassViewInteractorStyle>::New();
//  //this->InteractorStyle = vtkSmartPointer<vtkOpenVRInteractorStyle>::New(); //TODO: For debugging the original interactor
//  this->Interactor->SetInteractorStyle(this->InteractorStyle);
//  this->InteractorStyle->SetInteractor(this->Interactor);
//  this->InteractorStyle->SetCurrentRenderer(this->Renderer);
  this->Camera = vtkSmartPointer<vtkCamera>::New();
  this->Renderer->SetActiveCamera(this->Camera);

  qDebug() <<  __LINE__;

  this->RenderWindow->SetMultiSamples(0);
  qDebug() <<  __LINE__;
  this->RenderWindow->AddRenderer(this->Renderer);
  qDebug() <<  __LINE__;
  this->RenderWindow->SetInteractor(this->Interactor);
  qDebug() <<  __LINE__;
//  // Set default 10x magnification (conversion: PhysicalScale = 1000 / Magnification)
//  this->RenderWindow->SetPhysicalScale(100.0);
//  // Observe VR render window event
//  qvtkReconnect(this->RenderWindow, vtkOpenVRRenderWindow::PhysicalToWorldMatrixModified,
//                q, SLOT(onPhysicalToWorldMatrixModified()));

  vtkMRMLLookingGlassViewDisplayableManagerFactory* factory
    = vtkMRMLLookingGlassViewDisplayableManagerFactory::GetInstance();

  QStringList displayableManagers;
  displayableManagers //<< "vtkMRMLCameraDisplayableManager"
  //<< "vtkMRMLViewDisplayableManager"
      << "vtkMRMLModelDisplayableManager"
      << "vtkMRMLThreeDReformatDisplayableManager"
      << "vtkMRMLCrosshairDisplayableManager3D"
      //<< "vtkMRMLOrientationMarkerDisplayableManager" // Not supported in VR
      //<< "vtkMRMLRulerDisplayableManager" // Not supported in VR
      //<< "vtkMRMLAnnotationDisplayableManager" // Not supported in VR
      << "vtkMRMLMarkupsDisplayableManager"
      << "vtkMRMLSegmentationsDisplayableManager3D"
      << "vtkMRMLTransformsDisplayableManager3D"
      << "vtkMRMLLinearTransformsDisplayableManager3D"
      << "vtkMRMLVolumeRenderingDisplayableManager"
      ;
  qDebug() <<  __LINE__;
  foreach (const QString& displayableManager, displayableManagers)
  {
    qDebug() <<  __LINE__ << displayableManager;
    if (!factory->IsDisplayableManagerRegistered(displayableManager.toLatin1()))
    {
      factory->RegisterDisplayableManager(displayableManager.toLatin1());
    }
  }

  qDebug() <<  __LINE__;
  this->DisplayableManagerGroup = vtkSmartPointer<vtkMRMLDisplayableManagerGroup>::Take(
                                    factory->InstantiateDisplayableManagers(q->renderer()));
  this->DisplayableManagerGroup->SetMRMLDisplayableNode(this->MRMLLookingGlassViewNode);
//  this->InteractorStyle->SetDisplayableManagerGroup(this->DisplayableManagerGroup);

  qDebug() << "this->DisplayableManagerGroup" << this->DisplayableManagerGroup->GetDisplayableManagerCount();

//  ///CONFIGURATION OF THE OPENVR ENVIRONEMENT

//  this->Renderer->RemoveCuller(this->Renderer->GetCullers()->GetLastItem());
//  this->Renderer->SetBackground(0.7, 0.7, 0.7);

  q->updateViewFromReferenceViewCamera();

  this->RenderWindow->Initialize();
  this->Renderer->ResetCamera();
//  this->Renderer->GetActiveCamera();
//  if (!this->RenderWindow->GetHMD())
//  {
//    qWarning() << "Failed to initialize OpenVR RenderWindow";
//    return;
//  }
}

//---------------------------------------------------------------------------
void qMRMLLookingGlassViewPrivate::destroyRenderWindow()
{
  Q_Q(qMRMLLookingGlassView);
//  this->LookingGlassLoopTimer.stop();
  // Must break the connection between interactor and render window,
  // otherwise they would circularly refer to each other and would not
  // be deleted.
  this->Interactor->SetRenderWindow(nullptr);
  this->Interactor = nullptr;
//  this->InteractorStyle = nullptr;
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

  this->Renderer->SetTwoSidedLighting(this->MRMLLookingGlassViewNode->GetTwoSidedLighting());

  this->Renderer->SetUseDepthPeeling(this->MRMLLookingGlassViewNode->GetUseDepthPeeling() != 0);
  this->Renderer->SetUseDepthPeelingForVolumes(this->MRMLLookingGlassViewNode->GetUseDepthPeeling() != 0);

  // Render window properties
  if (this->RenderWindow)
  {
    // Desired update rate
    this->RenderWindow->SetDesiredUpdateRate(this->desiredUpdateRate());

//    // Magnification
//    double magnification = this->MRMLLookingGlassViewNode->GetMagnification();
//    if (magnification < 0.01)
//    {
//      magnification = 0.01;
//    }
//    else if (magnification > 100.0)
//    {
//      magnification = 100.0;
//    }
//    this->InteractorStyle->SetMagnification(magnification);

//    // Dolly physical speed
//    double dollyPhysicalSpeedMps = this ->MRMLLookingGlassViewNode->GetMotionSpeed();

//    // 1.6666 m/s is walking speed (= 6 km/h), which is the default. We multiply it with the factor
//    this->InteractorStyle->SetDollyPhysicalSpeed(dollyPhysicalSpeedMps);

//    if (this->RenderWindow->GetHMD())
//    {
//      vtkEventDataDevice deviceIdsToUpdate[] = { vtkEventDataDevice::RightController, vtkEventDataDevice::LeftController, vtkEventDataDevice::Unknown };
//      for (int deviceIdIndex = 0; deviceIdsToUpdate[deviceIdIndex] != vtkEventDataDevice::Unknown; deviceIdIndex++)
//      {
//        vtkOpenVRModel* model = this->RenderWindow->GetTrackedDeviceModel(deviceIdsToUpdate[deviceIdIndex]);
//        if (!model)
//        {
//          continue;
//        }
//        model->SetVisibility(this->MRMLLookingGlassViewNode->GetControllerModelsVisible());
//      }

//      // Update tracking reference visibility
//      for (uint32_t deviceIdIndex = 0; deviceIdIndex < vr::k_unMaxTrackedDeviceCount; ++deviceIdIndex)
//      {
//        if (this->RenderWindow->GetHMD()->GetTrackedDeviceClass(deviceIdIndex) == vr::TrackedDeviceClass_TrackingReference)
//        {
//          vtkOpenVRModel* model = this->RenderWindow->GetTrackedDeviceModel(deviceIdIndex);
//          if (!model)
//          {
//            continue;
//          }
//          model->SetVisibility(this->MRMLLookingGlassViewNode->GetLighthouseModelsVisible());
//        }
//      }
//    }
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
  vtkMRMLViewNode* refrenceViewNode = d->MRMLLookingGlassViewNode->GetReferenceViewNode();
  if (!refrenceViewNode)
    {
    qWarning() << Q_FUNC_INFO << " failed: no reference view node is set";
    return;
    }
  if (!d->CamerasLogic)
    {
    qWarning() << Q_FUNC_INFO << " failed: cameras module logic is not set";
    return;
    }
  vtkMRMLCameraNode* cameraNode = d->CamerasLogic->GetViewActiveCameraNode(refrenceViewNode);
  if (!cameraNode || !cameraNode->GetCamera())
    {
    qWarning() << Q_FUNC_INFO << " failed: camera node is not found";
    return;
    }
  if (!d->RenderWindow)
    {
    qWarning() << Q_FUNC_INFO << " failed: RenderWindow has not been created";
    return;
    }

  // The following is based on d->RenderWindow->InitializeViewFromCamera(sourceCamera),
  // but that is not usable for us, as it puts the headset in the focal point (so we
  // need to step back to see the full content) and snaps view direction to the closest axis.

  vtkCamera* sourceCamera = cameraNode->GetCamera();

  vtkRenderer* ren = static_cast<vtkRenderer*>(d->RenderWindow->GetRenderers()->GetItemAsObject(0));
  if (!ren)
    {
    qWarning() << Q_FUNC_INFO << "The renderer must be set prior to calling InitializeViewFromCamera";
    return;
    }
//  vtkOpenVRCamera* cam = vtkOpenVRCamera::SafeDownCast(ren->GetActiveCamera());
//  if (!cam)
//    {
//    qWarning() << Q_FUNC_INFO << "The renderer's active camera must be set prior to calling InitializeViewFromCamera";
//    return;
//    }

  double newPhysicalScale = 100.0; // Default 10x magnification

//  double* sourceViewUp = sourceCamera->GetViewUp();
//  cam->SetViewUp(sourceViewUp);
//  d->RenderWindow->SetPhysicalViewUp(sourceViewUp);

//  double* sourcePosition = sourceCamera->GetPosition();
//  double* viewUp = cam->GetViewUp();
//  cam->SetFocalPoint(sourcePosition);
//  d->RenderWindow->SetPhysicalTranslation(
//    viewUp[0] * newPhysicalScale - sourcePosition[0],
//    viewUp[1] * newPhysicalScale - sourcePosition[1],
//    viewUp[2] * newPhysicalScale - sourcePosition[2]);

//  double* sourceDirectionOfProjection = sourceCamera->GetDirectionOfProjection();
//  d->RenderWindow->SetPhysicalViewDirection(sourceDirectionOfProjection);
//  double* idop = d->RenderWindow->GetPhysicalViewDirection();
//  cam->SetPosition(
//    -idop[0] * newPhysicalScale + sourcePosition[0],
//    -idop[1] * newPhysicalScale + sourcePosition[1],
//    -idop[2] * newPhysicalScale + sourcePosition[2]);

//  d->RenderWindow->SetPhysicalScale(newPhysicalScale);

  ren->ResetCameraClippingRange();
}
