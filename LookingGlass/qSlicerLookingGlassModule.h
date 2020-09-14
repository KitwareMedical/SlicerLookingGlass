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

#ifndef __qSlicerLookingGlassModule_h
#define __qSlicerLookingGlassModule_h

// Slicer includes
#include "qSlicerLoadableModule.h"

#include "qSlicerLookingGlassModuleExport.h"

// CTK includes
#include <ctkVTKObject.h>

class qSlicerLookingGlassModulePrivate;
class qMRMLLookingGlassView;
class QToolBar;

class Q_SLICER_QTMODULES_LOOKINGGLASS_EXPORT
qSlicerLookingGlassModule
  : public qSlicerLoadableModule
{
  Q_OBJECT
  QVTK_OBJECT
  Q_PLUGIN_METADATA(IID "org.slicer.modules.loadable.qSlicerLoadableModule/1.0")
  Q_INTERFACES(qSlicerLoadableModule)

public:

  typedef qSlicerLoadableModule Superclass;
  explicit qSlicerLookingGlassModule(QObject *parent=nullptr);
  virtual ~qSlicerLookingGlassModule() override;

  qSlicerGetTitleMacro(QTMODULE_TITLE)

  virtual QString helpText()const override;
  virtual QString acknowledgementText()const override;
  virtual QStringList contributors()const override;

  virtual QIcon icon()const override;

  virtual QStringList categories()const override;
  virtual QStringList dependencies() const override;

  Q_INVOKABLE bool isToolBarVisible();
  Q_INVOKABLE QToolBar* toolBar();

  Q_INVOKABLE virtual qMRMLLookingGlassView* viewWidget();

public slots:
  void setToolBarVisible(bool visible);
  void enableLookingGlass(bool);
  void updateViewFromReferenceViewCamera();
  void switchToLookingGlassModule();

  /// Set MRML scene for the module. Updates the default view settings based on
  /// the application settings.
  virtual void setMRMLScene(vtkMRMLScene* scene) override;

protected slots:
  void onViewNodeModified();

protected:

  /// Initialize the module. Register the volumes reader/writer
  virtual void setup() override;

  /// Create and return the widget representation associated to this module
  virtual qSlicerAbstractModuleRepresentation * createWidgetRepresentation() override;

  /// Create and return the logic associated to this module
  virtual vtkMRMLAbstractLogic* createLogic() override;

protected:
  QScopedPointer<qSlicerLookingGlassModulePrivate> d_ptr;

private:
  Q_DECLARE_PRIVATE(qSlicerLookingGlassModule)
  Q_DISABLE_COPY(qSlicerLookingGlassModule)

};

#endif
