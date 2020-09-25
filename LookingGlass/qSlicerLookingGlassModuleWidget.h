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

#ifndef __qSlicerLookingGlassModuleWidget_h
#define __qSlicerLookingGlassModuleWidget_h

// Slicer includes
#include "qSlicerAbstractModuleWidget.h"

#include "qSlicerLookingGlassModuleExport.h"

class qSlicerLookingGlassModuleWidgetPrivate;
class vtkMRMLNode;

class Q_SLICER_QTMODULES_LOOKINGGLASS_EXPORT qSlicerLookingGlassModuleWidget :
  public qSlicerAbstractModuleWidget
{
  Q_OBJECT

public:

  typedef qSlicerAbstractModuleWidget Superclass;
  qSlicerLookingGlassModuleWidget(QWidget *parent=nullptr);
  virtual ~qSlicerLookingGlassModuleWidget();

public slots:
  void setLookingGlassConnected(bool connect);
  void setLookingGlassActive(bool activate);
  void setReferenceViewNode(vtkMRMLNode*);
  void updateViewFromReferenceViewCamera();
  void onDesiredUpdateRateChanged(double);
  void onNearClippingLimitChanged(double);
  void onFarClippingLimitChanged(double);
  void pullFocalPlaneForward();
  void pushFocalPlaneBack();

protected:
  QScopedPointer<qSlicerLookingGlassModuleWidgetPrivate> d_ptr;

  virtual void setup();

private:
  Q_DECLARE_PRIVATE(qSlicerLookingGlassModuleWidget)
  Q_DISABLE_COPY(qSlicerLookingGlassModuleWidget)
};

#endif
