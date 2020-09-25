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

// LookingGlass Logic includes
#include "vtkMRMLLookingGlassViewNode.h"
#include "vtkSlicerLookingGlassLogic.h"

// MRML includes
#include <vtkMRMLScene.h>

// Slicer includes
#include "vtkSlicerVolumeRenderingLogic.h"

// VTK includes
#include <vtkIntArray.h>
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// STD includes
#include <cassert>

//----------------------------------------------------------------------------
vtkStandardNewMacro(vtkSlicerLookingGlassLogic);
vtkCxxSetObjectMacro(vtkSlicerLookingGlassLogic, VolumeRenderingLogic, vtkSlicerVolumeRenderingLogic);

//----------------------------------------------------------------------------
vtkSlicerLookingGlassLogic::vtkSlicerLookingGlassLogic()
  : ActiveViewNode(nullptr)
  , VolumeRenderingLogic(nullptr)
{
}

//----------------------------------------------------------------------------
vtkSlicerLookingGlassLogic::~vtkSlicerLookingGlassLogic()
{
  this->SetActiveViewNode(nullptr);
  this->SetVolumeRenderingLogic(nullptr);
}

//----------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);
}

//---------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::SetMRMLSceneInternal(vtkMRMLScene * newScene)
{
  vtkNew<vtkIntArray> events;
  events->InsertNextValue(vtkMRMLScene::NodeAddedEvent);
  events->InsertNextValue(vtkMRMLScene::NodeRemovedEvent);
  events->InsertNextValue(vtkMRMLScene::EndBatchProcessEvent);
  events->InsertNextValue(vtkMRMLScene::EndImportEvent);
  this->SetAndObserveMRMLSceneEventsInternal(newScene, events.GetPointer());
}

//-----------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::RegisterNodes()
{
  assert(this->GetMRMLScene() != 0);
  // Register LookingGlass view node class
  this->GetMRMLScene()->RegisterNodeClass((vtkSmartPointer<vtkMRMLLookingGlassViewNode>::New()));
}

//---------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::UpdateFromMRMLScene()
{
  assert(this->GetMRMLScene() != 0);
  vtkMRMLLookingGlassViewNode* lgViewNode = nullptr;
  if (this->GetMRMLScene())
    {
    lgViewNode = vtkMRMLLookingGlassViewNode::SafeDownCast(
                   this->GetMRMLScene()->GetSingletonNode("Active", "vtkMRMLLookingGlassViewNode"));
    }
  this->SetActiveViewNode(lgViewNode);
}

//---------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic
::OnMRMLSceneNodeAdded(vtkMRMLNode* node)
{
  vtkMRMLLookingGlassViewNode* lgViewNode = vtkMRMLLookingGlassViewNode::SafeDownCast(node);
  if (!lgViewNode)
    {
    return;
    }
  // If a new active VR view node added then use it automatically.
  if (lgViewNode->GetSingletonTag() &&
      strcmp(lgViewNode->GetSingletonTag(), "Active") == 0)
    {
    this->SetActiveViewNode(lgViewNode);
    }
}

//---------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic
::OnMRMLSceneNodeRemoved(vtkMRMLNode* node)
{
  vtkMRMLLookingGlassViewNode* deletedLgViewNode = vtkMRMLLookingGlassViewNode::SafeDownCast(node);
  if (!deletedLgViewNode)
    {
    return;
    }
  if (deletedLgViewNode == this->ActiveViewNode)
    {
    this->SetActiveViewNode(nullptr);
    }
}

//----------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::OnMRMLSceneEndImport()
{
  if (this->ActiveViewNode != nullptr && this->ActiveViewNode->GetActive())
    {
    // Override the active flag and visibility flags, as LG connection is not restored on scene load
    this->ActiveViewNode->SetActive(0);
    this->ActiveViewNode->SetVisibility(0);
    }

  this->Modified();
}

//----------------------------------------------------------------------------
vtkMRMLLookingGlassViewNode* vtkSlicerLookingGlassLogic::GetLookingGlassViewNode()
{
  return this->ActiveViewNode;
}

//---------------------------------------------------------------------------
vtkMRMLLookingGlassViewNode* vtkSlicerLookingGlassLogic::AddLookingGlassViewNode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("AddLookingGlassViewNode: Invalid MRML scene");
    return nullptr;
    }

  if (this->ActiveViewNode)
    {
    // There is already a usable VR node, return that
    return this->ActiveViewNode;
    }

  // Create LookingGlass view node. Use CreateNodeByClass so that node properties
  // can be overridden with default node properties defined in the scene.
  vtkSmartPointer<vtkMRMLLookingGlassViewNode> vrViewNode = vtkSmartPointer<vtkMRMLLookingGlassViewNode>::Take(
        vtkMRMLLookingGlassViewNode::SafeDownCast(scene->CreateNodeByClass("vtkMRMLLookingGlassViewNode")));
  // We create the node as a singleton to make sure there is only one VR view node in the scene.
  vrViewNode->SetSingletonTag("Active");
  // If a singleton node by that name exists already then it is overwritten
  // and pointer of that node is returned.
  vrViewNode = vtkMRMLLookingGlassViewNode::SafeDownCast(scene->AddNode(vrViewNode));

  return vrViewNode;
}

//----------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::SetActiveViewNode(vtkMRMLLookingGlassViewNode* viewNode)
{
  if (this->ActiveViewNode == viewNode)
    {
    return;
    }

  if (!this->GetMRMLScene())
    {
    return;
    }

  this->GetMRMLNodesObserverManager()->SetAndObserveObject(vtkObjectPointer(&this->ActiveViewNode), viewNode);

  this->Modified();
}

//-----------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::ProcessMRMLNodesEvents(vtkObject* caller, unsigned long event, void* vtkNotUsed(callData))
{
  if (caller == this->ActiveViewNode && event == vtkCommand::ModifiedEvent)
    {
    this->Modified();
    }
}

//-----------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::SetLookingGlassConnected(bool connect)
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("SetLookingGlassConnected: Invalid MRML scene");
    return;
    }

  if (connect)
    {
    if (!this->ActiveViewNode)
      {
      // Check if there is a LookingGlass view node in the scene, in case the scene has been loaded
      // from file and VR view properties has been changed
      if (scene->GetNumberOfNodesByClass("vtkMRMLLookingGlassViewNode") > 0)
        {
        // Use the first one if any found
        this->SetActiveViewNode(
          vtkMRMLLookingGlassViewNode::SafeDownCast(scene->GetNthNodeByClass(0, "vtkMRMLLookingGlassViewNode")));
        }
      else
        {
        vtkMRMLLookingGlassViewNode* newViewNode = this->AddLookingGlassViewNode();
        this->SetActiveViewNode(newViewNode);
        }
      }
    if (this->ActiveViewNode)
      {
      this->ActiveViewNode->SetVisibility(1);
      }
    else
      {
      vtkErrorMacro("Failed to create looking glass view node");
      }
    }
  else
    {
    if (this->ActiveViewNode)
      {
      this->ActiveViewNode->SetVisibility(0);
      }
    }
}

//-----------------------------------------------------------------------------
bool vtkSlicerLookingGlassLogic::GetLookingGlassConnected()
{
  if (!this->ActiveViewNode)
    {
    return false;
    }
  return (this->ActiveViewNode->GetVisibility() != 0);
}

//-----------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::SetLookingGlassActive(bool activate)
{
  if (activate)
  {
    if (this->GetLookingGlassConnected()
        && this->GetLookingGlassViewNode()
        && this->GetLookingGlassViewNode()->HasError())
    {
      // If it is connected already but there is an error then disconnect first then reconnect
      // as the error may be resolved by reconnecting.
      this->SetLookingGlassConnected(false);
    }
    this->SetLookingGlassConnected(true);
    if (this->ActiveViewNode)
    {
      this->ActiveViewNode->SetActive(1);
    }
    else
    {
      vtkErrorMacro("vtkSlicerLookingGlassLogic::SetLookingGlassActive failed: view node is not available");
    }
  }
  else
  {
    if (this->ActiveViewNode != nullptr)
    {
      this->ActiveViewNode->SetActive(0);
    }
  }
}

//-----------------------------------------------------------------------------
bool vtkSlicerLookingGlassLogic::GetLookingGlassActive()
{
  if (!this->ActiveViewNode)
    {
    return false;
    }
  return (this->ActiveViewNode->GetVisibility() != 0 && this->ActiveViewNode->GetActive() != 0);
}

//---------------------------------------------------------------------------
void vtkSlicerLookingGlassLogic::SetDefaultReferenceView()
{
  if (!this->ActiveViewNode)
    {
    return;
    }
  if (this->ActiveViewNode->GetReferenceViewNode() != nullptr)
    {
    // Reference view is already set, there is nothing to do
    return;
    }
  if (!this->GetMRMLScene())
    {
    return;
    }
  vtkSmartPointer<vtkCollection> nodes = vtkSmartPointer<vtkCollection>::Take(
      this->GetMRMLScene()->GetNodesByClass("vtkMRMLViewNode"));
  vtkMRMLViewNode* viewNode = nullptr;
  vtkCollectionSimpleIterator it;
  for (nodes->InitTraversal(it); (viewNode = vtkMRMLViewNode::SafeDownCast(
                                    nodes->GetNextItemAsObject(it)));)
    {
    if (viewNode->GetVisibility() && viewNode->IsMappedInLayout())
      {
      // Found a view node displayed in current layout, use this
      break;
      }
    }
  // Either use a view node displayed in current layout or just any 3D view node found in the scene
  this->ActiveViewNode->SetAndObserveReferenceViewNode(viewNode);
}

//-----------------------------------------------------------------------------
vtkMRMLLookingGlassViewNode* vtkSlicerLookingGlassLogic::GetDefaultLookingGlassViewNode()
{
  vtkMRMLScene* scene = this->GetMRMLScene();
  if (!scene)
    {
    vtkErrorMacro("GetDefaultLookingGlassViewNode failed: invalid scene");
    return nullptr;
    }
  vtkMRMLNode* defaultNode = scene->GetDefaultNodeByClass("vtkMRMLLookingGlassViewNode");
  if (!defaultNode)
    {
    defaultNode = scene->CreateNodeByClass("vtkMRMLLookingGlassViewNode");
    scene->AddDefaultNode(defaultNode);
    defaultNode->Delete(); // scene owns it now
    }
  return vtkMRMLLookingGlassViewNode::SafeDownCast(defaultNode);
}
