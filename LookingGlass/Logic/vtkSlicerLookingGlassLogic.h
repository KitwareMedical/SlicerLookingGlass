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

// .NAME vtkSlicerLookingGlassLogic - slicer logic class for volumes manipulation
// .SECTION Description
// This class manages the logic associated with reading, saving,
// and changing propertied of the volumes


#ifndef __vtkSlicerLookingGlassLogic_h
#define __vtkSlicerLookingGlassLogic_h

// Slicer includes
#include "vtkSlicerModuleLogic.h"

// MRML includes

// STD includes
#include <cstdlib>

#include "vtkSlicerLookingGlassModuleLogicExport.h"

class vtkSlicerVolumeRenderingLogic;
class vtkMRMLLookingGlassViewNode;

/// \ingroup Slicer_QtModules_ExtensionTemplate
class VTK_SLICER_LOOKINGGLASS_MODULE_LOGIC_EXPORT vtkSlicerLookingGlassLogic :
  public vtkSlicerModuleLogic
{
public:

  static vtkSlicerLookingGlassLogic *New();
  vtkTypeMacro(vtkSlicerLookingGlassLogic, vtkSlicerModuleLogic);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  /// Creates a singleton looking glass view node and adds it to the scene.
  /// If there is a looking glass view node in the scene already then it just returns that.
  /// If current view node is created, deleted, or modified then a Modified() event is invoked
  /// for this logic class, to make it easy for modules to detect view changes.
  vtkMRMLLookingGlassViewNode* AddLookingGlassViewNode();

  /// Get active singleton looking glass view node
  vtkMRMLLookingGlassViewNode* GetLookingGlassViewNode();

  /// Retrieves the default VR view node from the scene. Creates it if does not exist.
  vtkMRMLLookingGlassViewNode* GetDefaultLookingGlassViewNode();

  /// Connect/disconnect to headset.
  /// Adds looking glass view node if not added yet.
  void SetLookingGlassConnected(bool connect);
  bool GetLookingGlassConnected();

  /// Enable rendering updates in headset.
  /// Connects to device if not yet connected.
  void SetLookingGlassActive(bool activate);
  bool GetLookingGlassActive();

  /// Set the first visible 3D view as reference view for
  /// looking glass view.
  /// If a reference view has been already set then the
  /// method has no effect.
  void SetDefaultReferenceView();

  /// Set volume rendering logic
  void SetVolumeRenderingLogic(vtkSlicerVolumeRenderingLogic* volumeRenderingLogic);

protected:
  vtkSlicerLookingGlassLogic();
  virtual ~vtkSlicerLookingGlassLogic() override;

  void SetActiveViewNode(vtkMRMLLookingGlassViewNode* vrViewNode);

  virtual void SetMRMLSceneInternal(vtkMRMLScene* newScene) override;
  /// Register MRML Node classes to Scene. Gets called automatically when the MRMLScene is attached to this logic class.
  virtual void RegisterNodes() override;
  virtual void UpdateFromMRMLScene() override;
  virtual void OnMRMLSceneNodeAdded(vtkMRMLNode* node) override;
  virtual void OnMRMLSceneNodeRemoved(vtkMRMLNode* node) override;
  virtual void OnMRMLSceneEndImport() override;
  virtual void ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* callData) override;

protected:
  /// Active VR view node
  vtkMRMLLookingGlassViewNode* ActiveViewNode;

  /// Volume rendering logic
  vtkSlicerVolumeRenderingLogic* VolumeRenderingLogic;

private:

  vtkSlicerLookingGlassLogic(const vtkSlicerLookingGlassLogic&); // Not implemented
  void operator=(const vtkSlicerLookingGlassLogic&); // Not implemented
};

#endif
