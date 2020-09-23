/*==============================================================================

  Copyright (c) Laboratory for Percutaneous Surgery (PerkLab)
  Queen's University, Kingston, ON, Canada. All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Unless required by applicable law or agreed to in writing, software
  distributed under the License is distributed on an "AS IS" BASIS,
  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
  See the License for the specific language governing permissions and
  limitations under the License.

  This file was originally developed by Csaba Pinter, PerkLab, Queen's University
  and was supported through CANARIE's Research Software Program, and Cancer
  Care Ontario.

==============================================================================*/

// STL includes
#include <sstream>

// VTK includes
#include <vtkNew.h>
#include <vtkObjectFactory.h>

// MRML includes
#include "vtkMRMLLookingGlassLayoutNode.h"

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLLookingGlassLayoutNode);

//----------------------------------------------------------------------------
vtkMRMLLookingGlassLayoutNode::vtkMRMLLookingGlassLayoutNode()
{
  this->SetSingletonTag("vtkMRMLLookingGlassLayoutNode");

  this->CurrentLayoutDescription = NULL;

  // Synchronize the view description with the layout
  //TODO:
  //this->AddLayoutDescription(vtkMRMLLookingGlassLayoutNode::SlicerLayoutNone, "");
}

//----------------------------------------------------------------------------
vtkMRMLLookingGlassLayoutNode::~vtkMRMLLookingGlassLayoutNode()
{
  this->SetCurrentLayoutDescription(0);
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassLayoutNode::WriteXML(ostream& of, int nIndent)
{
  // Write all attributes, since the parsing of the string is dependent on the
  // order here.

  Superclass::WriteXML(of, nIndent);

  //of << " layout=\"" << this->CurrentLayoutDescription << "\"";
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassLayoutNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  Superclass::ReadXMLAttributes(atts);

  const char* attName;
  const char* attValue;

  while (*atts != NULL)
    {
    attName = *(atts++);
    attValue = *(atts++);
    if ( !strcmp(attName, "layout"))
      {
      (void)attValue; // Avoid -Wunused-but-set-variable
      //this->SetAndParseCurrentLayoutDescription(attValue);
      }
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
bool vtkMRMLLookingGlassLayoutNode::AddLayoutDescription(int layout, const char* layoutDescription)
{
  if (this->IsLayoutDescription(layout))
    {
    vtkDebugMacro( << "Layout " << layout << " has already been registered");
    return false;
    }
  this->Layouts[layout] = std::string(layoutDescription);
  this->Modified();
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLLookingGlassLayoutNode::SetLayoutDescription(int layout, const char* layoutDescription)
{
  if (!this->IsLayoutDescription(layout))
    {
    vtkDebugMacro( << "Layout " << layout << " has NOT been registered");
    return false;
    }
  if (this->Layouts[layout] == layoutDescription)
    {
    return true;
    }
  this->Layouts[layout] = std::string(layoutDescription);
  int wasModifying = this->StartModify();
  this->UpdateCurrentLayoutDescription();
  this->Modified();
  this->EndModify(wasModifying);
  return true;
}

//----------------------------------------------------------------------------
bool vtkMRMLLookingGlassLayoutNode::IsLayoutDescription(int layout)
{
  std::map<int, std::string>::const_iterator it = this->Layouts.find(layout);
  return it != this->Layouts.end();
}

//----------------------------------------------------------------------------
std::string vtkMRMLLookingGlassLayoutNode::GetLayoutDescription(int layout)
{
  std::map<int, std::string>::const_iterator it = this->Layouts.find(layout);
  if (it == this->Layouts.end())
    {
    vtkWarningMacro("Can't find layout:" << layout);
    return std::string();
    }
  return it->second;
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassLayoutNode::UpdateCurrentLayoutDescription()
{
  //if (this->GetViewArrangement() == vtkMRMLLookingGlassLayoutNode::SlicerLayoutCustomView)
  //  {
  //  return;
  //  }
  //std::string description = this->GetLayoutDescription(this->ViewArrangement);
  //if (this->GetCurrentLayoutDescription() &&
  //    description == this->GetCurrentLayoutDescription())
  //  {
  //  return;
  //  }
  //this->SetAndParseCurrentLayoutDescription(description.c_str());
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassLayoutNode::SetAndParseCurrentLayoutDescription(const char* vtkNotUsed(description))
{
  //// Be careful that it matches the ViewArrangement value
  //if (this->LayoutRootElement)
  //  {
  //  this->LayoutRootElement->Delete();
  //  }
  //this->LayoutRootElement = this->ParseLayout(description);
  //if (this->LayoutRootElement == NULL)
  //  {
  //  // ParseLayout has already logged an error, if there was any
  //  this->SetCurrentLayoutDescription("");
  //  return;
  //  }

  //this->SetCurrentLayoutDescription(description);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, LabelText, ID
void vtkMRMLLookingGlassLayoutNode::Copy(vtkMRMLNode *anode)
{
  int disabledModify = this->StartModify();

  //vtkObject::Copy(anode);
  vtkMRMLLookingGlassLayoutNode *node = (vtkMRMLLookingGlassLayoutNode*)anode;
  // Try to copy the registered layout descriptions. However, if the node
  // currently has layout descriptions (more than the default None description)
  // then we don't want to copy them (it would overwrite the descriptions)
  if (node->Layouts.size() > 1 && this->Layouts.size() == 1)
    {
    this->Layouts = node->Layouts;
    }

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassLayoutNode::PrintSelf(ostream& os, vtkIndent indent)
{
  Superclass::PrintSelf(os,indent);
}
