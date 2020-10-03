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

#ifndef __qMRMLLookingGlassView_p_h
#define __qMRMLLookingGlassView_p_h

//
//  W A R N I N G
//  -------------
//
// This file is not part of the Slicer API.  It exists purely as an
// implementation detail.  This header file may change from version to
// version without notice, or even be removed.
//
// We mean it.
//

// VTK includes
#include <vtkEventData.h>
#include <vtkWeakPointer.h>

// CTK includes
#include <ctkPimpl.h>
#include <ctkVTKObject.h>

// qMRML includes
#include "qMRMLLookingGlassView.h"

// Qt includes
#include <QTime>
#include <QTimer>

class QLabel;
class vtkMRMLCameraNode;
class vtkMRMLDisplayableManagerGroup;
class vtkMRMLTransformNode;
class vtkMRMLLookingGlassViewNode;
class vtkObject;
//class vtkOpenVRInteractorStyle;
//class vtkOpenVRRenderWindowInteractor;
class vtkTimerLog;
class vtkLookingGlassViewInteractor;
class vtkLookingGlassViewInteractorStyle;
class vtkMRMLThreeDViewInteractorStyle;


//-----------------------------------------------------------------------------
class qMRMLLookingGlassViewPrivate: public QObject
{
  Q_OBJECT
  QVTK_OBJECT
  Q_DECLARE_PUBLIC(qMRMLLookingGlassView);
protected:
  qMRMLLookingGlassView* const q_ptr;
public:
  qMRMLLookingGlassViewPrivate(qMRMLLookingGlassView& object);
  ~qMRMLLookingGlassViewPrivate();

  virtual void init();

  void startLookingGlass();
  void stopLookingGlass();

  double desiredUpdateRate();

public slots:
  void updateWidgetFromMRML();

protected:
  void createRenderWindow();
  void destroyRenderWindow();

  vtkSlicerCamerasModuleLogic* CamerasLogic;

  vtkSmartPointer<vtkMRMLDisplayableManagerGroup> DisplayableManagerGroup;
  vtkWeakPointer<vtkMRMLLookingGlassViewNode> MRMLLookingGlassViewNode;
  vtkSmartPointer<vtkRenderer> Renderer;
  vtkSmartPointer<vtkOpenGLRenderWindow> RenderWindow;
  vtkSmartPointer<vtkRenderWindowInteractor> Interactor;
  vtkSmartPointer<vtkMRMLThreeDViewInteractorStyle> InteractorStyle;
  //vtkSmartPointer<vtkOpenVRRenderWindowInteractor> Interactor; //TODO: For debugging the original interactor
//  vtkSmartPointer<vtkLookingGlassViewInteractorStyle> InteractorStyle;
  //vtkSmartPointer<vtkOpenVRInteractorStyle> InteractorStyle; //TODO: For debugging the original interactor
  vtkSmartPointer<vtkCamera> Camera;

  vtkSmartPointer<vtkTimerLog> LastViewUpdateTime;
  double LastViewDirection[3];
  double LastViewUp[3];
  double LastViewPosition[3];

  bool ReferenceViewInteractive;

  // Copied from ctkVTKAbstractViewPrivate
  QTimer*                                       RequestTimer;
  QTime                                         RequestTime;

  QTimer LookingGlassLoopTimer;
};

#endif
