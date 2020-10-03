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

#ifndef __qMRMLLookingGlassView_h
#define __qMRMLLookingGlassView_h

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKRenderView.h>

// Qt includes
#include <QWidget>

#include "qSlicerLookingGlassModuleWidgetsExport.h"

// CTK includes
#include <ctkVTKObject.h> 

class qMRMLLookingGlassViewPrivate;
class vtkMRMLLookingGlassViewNode;
class vtkCollection;
class vtkGenericOpenGLRenderWindow;
class vtkRenderWindowInteractor;
class vtkSlicerCamerasModuleLogic;

class vtkLookingGlassInterface;
//class vtkOpenVRRenderer;
class vtkOpenGLRenderWindow;
class vtkRenderWindowInteractor;
//class vtkOpenVRCamera;

/// \brief 3D view for view nodes.
/// For performance reasons, the view block refreshes when the scene is in
/// batch process state.
/// VR hardware connection state is controlled by associated view node's properties:
/// - Visible: connection is made with OpenVR.
/// - Active: scene is rendered in the VR headset.
/// \sa qMRMLVRWidget, qMRMLVRViewControllerWidget, qMRMLSliceView
class Q_SLICER_MODULE_LOOKINGGLASS_WIDGETS_EXPORT qMRMLLookingGlassView : public QWidget
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PROPERTY(bool referenceViewInteractive READ isReferenceViewInteractive WRITE setReferenceViewInteractive)
public:
  /// Superclass typedef
  typedef QWidget Superclass;

  /// Constructors
  explicit qMRMLLookingGlassView(QWidget* parent = nullptr);
  virtual ~qMRMLLookingGlassView();

  /// Add a displayable manager to the view,
  /// the displayable manager is proper to the 3D view and is not shared
  /// with other views.
  /// If you want to register a displayable manager with all the 3D
  /// views (existing or future), you need to do it via
  /// vtkMRMLLookingGlassViewDisplayableManagerFactory::RegisterDisplayableManager()
  /// By default: vtkMRMLCameraDisplayableManager,
  /// vtkMRMLViewDisplayableManager and vtkMRMLModelDisplayableManager are
  /// already registered.
  void addDisplayableManager(const QString& displayableManager);
  Q_INVOKABLE void getDisplayableManagers(vtkCollection *displayableManagers);

  /// Set Cameras module logic.
  /// Required for updating camera from reference view node.
  void setCamerasLogic(vtkSlicerCamerasModuleLogic* camerasLogic);
  vtkSlicerCamerasModuleLogic* camerasLogic()const;

  /// Get the 3D View node observed by view.
  Q_INVOKABLE vtkMRMLLookingGlassViewNode* mrmlLookingGlassViewNode()const;

  /// Get a reference to the associated vtkRenderer
  vtkRenderer* renderer()const;

  /// Get underlying RenderWindow
  Q_INVOKABLE vtkOpenGLRenderWindow* renderWindow()const;

  /// Get LookingGlass interface
  Q_INVOKABLE vtkLookingGlassInterface* lookingGlassTnterface()const;

  /// Get underlying RenderWindow interactor
  Q_INVOKABLE vtkRenderWindowInteractor* interactor()const;

  /// Initialize the looking glass view to most closely
  /// matched the camera of the reference view camera.
  Q_INVOKABLE void updateViewFromReferenceViewCamera();

  /// Get underlying RenderWindow
  Q_INVOKABLE bool isHardwareConnected()const;

  /// Indicate if reference view is being interacted with
  bool isReferenceViewInteractive() const;

  /// Set interaction state associated with the reference view.
  ///
  /// \warning This should only be set from qSlicerLookingGlassModuleWidget based
  /// on vtkCommand::StartInteractionEvent and vtkCommand::EndInteractionEvent
  void setReferenceViewInteractive(bool interactive);

public slots:
  /// Set the current \a viewNode to observe
  void setMRMLLookingGlassViewNode(vtkMRMLLookingGlassViewNode* newViewNode);

  void pushFocalPlaneBack();
  void pullFocalPlaneForward();

  /// Notify that the view needs to be rendered.
  /// scheduleRender() respects the maximum update rate of the view,
  /// it won't render the window more frequently than what the maximum
  /// update rate is.
  /// \sa setMaximumUpdateRate
  virtual void scheduleRender();

  /// Force a render even if a render is already ocurring
  /// Be careful when calling forceRender() as it can slow down your
  /// application. It is preferable to use scheduleRender() instead.
  /// \sa scheduleRender
  virtual void forceRender();

  /// Set maximum rate for rendering (in frames per second).
  /// If rendering is requested more frequently than this rate using scheduleRender,
  /// actual rendering will happen at this rate.
  /// This mechanism prevents repeated rendering caused by cluster of rendering requests.
  ///
  /// If maximum update rate is set to 0 then it indicates that rendering is done next time
  /// the application is idle, i.e., pending timer events are processed. This option should be used
  /// with caution, as policy of timer event processing may differ between operating systems.
  /// Specifically, on macOS, timer events may be indefinitely delayed if user interface continuously
  /// generates events.
  ///
  /// RenderWindow's DesiredUpdateRate property is intended for determining rendering quality settings,
  /// and is not suitable to be used as maximum update rate. The main reason is that VTK usually makes the
  /// rendering much faster and lower quality than DesiredUpdateRate would dictate, and so it would
  /// unnecessarily decrease the actual refresh rate.
  ///
  /// By default maximum update rate is set to 60FPS, which allows smooth updates, while effectively
  /// suppressing repeated update requests (after a rendering has been completed,
  /// repeated rendering requests will be ignored for 17 milliseconds).
  ///
  /// \sa scheduleRender
//  void setMaximumUpdateRate(double fps);

protected slots:

  /// Calls forceRender if the rendering has not been paused from pauseRender()
  /// \sa pauseRender
  virtual void requestRender();

protected:

  QScopedPointer<qMRMLLookingGlassViewPrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qMRMLLookingGlassView)
  Q_DISABLE_COPY(qMRMLLookingGlassView)
};

#endif
