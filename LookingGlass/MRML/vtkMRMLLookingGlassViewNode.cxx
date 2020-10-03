/*=auto=========================================================================

Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

See COPYRIGHT.txt
or http://www.slicer.org/copyright/copyright.txt for details.

Program:   3D Slicer
Module:    $RCSfile: vtkMRMLLookingGlassViewNode.cxx,v $
Date:      $Date: 2006/03/03 22:26:39 $
Version:   $Revision: 1.3 $

=========================================================================auto=*/

// MRML includes
#include "vtkMRMLScene.h"
#include "vtkMRMLViewNode.h"
#include "vtkMRMLLookingGlassViewNode.h"

// VTK includes
#include <vtkObjectFactory.h>

// STD includes
#include <sstream>

const char* vtkMRMLLookingGlassViewNode::ReferenceViewNodeReferenceRole = "ReferenceViewNodeRef";

//----------------------------------------------------------------------------
vtkMRMLNodeNewMacro(vtkMRMLLookingGlassViewNode);

//----------------------------------------------------------------------------
vtkMRMLLookingGlassViewNode::vtkMRMLLookingGlassViewNode()
  : RenderingMode(vtkMRMLLookingGlassViewNode::RenderingModeOnlyStillRenders)
  , DesiredUpdateRate(60.0)
  , UseClippingLimits(false)
  , NearClippingLimit(0.8)
  , FarClippingLimit(1.2)

{
  this->Visibility = 0; // hidden by default to not connect to the headset until it is needed
  this->BackgroundColor[0] = this->defaultBackgroundColor()[0];
  this->BackgroundColor[1] = this->defaultBackgroundColor()[1];
  this->BackgroundColor[2] = this->defaultBackgroundColor()[2];
  this->BackgroundColor2[0] = this->defaultBackgroundColor2()[0];
  this->BackgroundColor2[1] = this->defaultBackgroundColor2()[1];
  this->BackgroundColor2[2] = this->defaultBackgroundColor2()[2];
}

//----------------------------------------------------------------------------
vtkMRMLLookingGlassViewNode::~vtkMRMLLookingGlassViewNode()
{
}

//----------------------------------------------------------------------------
const char* vtkMRMLLookingGlassViewNode::GetNodeTagName()
{
  return "LookingGlassView";
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassViewNode::WriteXML(ostream& of, int nIndent)
{
  this->Superclass::WriteXML(of, nIndent);

  vtkMRMLWriteXMLBeginMacro(of);
  vtkMRMLWriteXMLEnumMacro(renderingMode, RenderingMode);
  vtkMRMLWriteXMLFloatMacro(desiredUpdateRate, DesiredUpdateRate);
  vtkMRMLWriteXMLBooleanMacro(useClippingLimits, UseClippingLimits);
  vtkMRMLWriteXMLFloatMacro(nearClippingLimit, NearClippingLimit);
  vtkMRMLWriteXMLFloatMacro(farClippingLimit, FarClippingLimit);
  vtkMRMLWriteXMLEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassViewNode::ReadXMLAttributes(const char** atts)
{
  int disabledModify = this->StartModify();

  this->Superclass::ReadXMLAttributes(atts);

  vtkMRMLReadXMLBeginMacro(atts);
  vtkMRMLReadXMLEnumMacro(renderingMode, RenderingMode);
  vtkMRMLReadXMLFloatMacro(desiredUpdateRate, DesiredUpdateRate);
  vtkMRMLReadXMLBooleanMacro(useClippingLimits, UseClippingLimits);
  vtkMRMLReadXMLFloatMacro(nearClippingLimit, NearClippingLimit);
  vtkMRMLReadXMLFloatMacro(farClippingLimit, FarClippingLimit);
  vtkMRMLReadXMLEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
// Copy the node's attributes to this object.
// Does NOT copy: ID, FilePrefix, Name, ID
void vtkMRMLLookingGlassViewNode::Copy(vtkMRMLNode* anode)
{
  int disabledModify = this->StartModify();

  this->Superclass::Copy(anode);

  vtkMRMLCopyBeginMacro(anode);
  vtkMRMLCopyEnumMacro(RenderingMode);
  vtkMRMLCopyFloatMacro(DesiredUpdateRate);
  vtkMRMLCopyBooleanMacro(UseClippingLimits);
  vtkMRMLCopyFloatMacro(NearClippingLimit);
  vtkMRMLCopyFloatMacro(FarClippingLimit);
  vtkMRMLCopyEndMacro();

  this->EndModify(disabledModify);
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassViewNode::PrintSelf(ostream& os, vtkIndent indent)
{
  this->Superclass::PrintSelf(os, indent);

  vtkMRMLPrintBeginMacro(os, indent);
  vtkMRMLPrintEnumMacro(RenderingMode);
  vtkMRMLPrintFloatMacro(DesiredUpdateRate);
  vtkMRMLPrintBooleanMacro(UseClippingLimits);
  vtkMRMLPrintFloatMacro(NearClippingLimit);
  vtkMRMLPrintFloatMacro(FarClippingLimit);
  vtkMRMLPrintEndMacro();
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassViewNode::SetSceneReferences()
{
  if (!this->Scene)
  {
    vtkErrorMacro( << "SetSceneReferences: Scene is expected to be non NULL.");
    return;
  }

  this->SetAndObserveParentLayoutNode(this);
}

//------------------------------------------------------------------------------
double* vtkMRMLLookingGlassViewNode::defaultBackgroundColor()
{
  //static double backgroundColor[3] = {0.70196, 0.70196, 0.90588};
  static double backgroundColor[3] = {0.7568627450980392,
                                      0.7647058823529412,
                                      0.9098039215686275
                                     };
  return backgroundColor;
}

//------------------------------------------------------------------------------
double* vtkMRMLLookingGlassViewNode::defaultBackgroundColor2()
{
  static double backgroundColor2[3] = {0.4549019607843137,
                                       0.4705882352941176,
                                       0.7450980392156863
                                      };
  return backgroundColor2;
}

//----------------------------------------------------------------------------
vtkMRMLViewNode* vtkMRMLLookingGlassViewNode::GetReferenceViewNode()
{
  return vtkMRMLViewNode::SafeDownCast(this->GetNodeReference(vtkMRMLLookingGlassViewNode::ReferenceViewNodeReferenceRole));
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassViewNode::SetAndObserveReferenceViewNodeID(const char* viewNodeId)
{
  this->SetAndObserveNodeReferenceID(vtkMRMLLookingGlassViewNode::ReferenceViewNodeReferenceRole, viewNodeId);
}

//----------------------------------------------------------------------------
bool vtkMRMLLookingGlassViewNode::SetAndObserveReferenceViewNode(vtkMRMLViewNode* node)
{
  if (node == NULL)
  {
    this->SetAndObserveReferenceViewNodeID(NULL);
    return true;
  }
  if (this->Scene != node->GetScene() || node->GetID() == NULL)
  {
    vtkErrorMacro("SetAndObserveReferenceViewNode: The referenced and referencing node are not in the same scene");
    return false;
  }
  this->SetAndObserveReferenceViewNodeID(node->GetID());
  return true;
}

//----------------------------------------------------------------------------
std::string vtkMRMLLookingGlassViewNode::GetRenderingModeAsString()
{
  return vtkMRMLLookingGlassViewNode::GetRenderingModeAsString(this->RenderingMode);
}

//-----------------------------------------------------------
const char* vtkMRMLLookingGlassViewNode::GetRenderingModeAsString(int id)
{
  switch (id)
  {
  case RenderingModeOnlyWhenRequested: return "OnlyWhenRequested";
  case RenderingModeOnlyStillRenders: return "OnlyStillRenders";
  case RenderingModeAlways: return "Always";
  default:
    // invalid id
    return "";
  }
}

//-----------------------------------------------------------
int vtkMRMLLookingGlassViewNode::GetRenderingModeFromString(const char* name)
{
  if (name == nullptr)
  {
    // invalid name
    return -1;
  }
  for (int ii = 0; ii < RenderingMode_Last; ii++)
  {
    if (strcmp(name, GetRenderingModeAsString(ii)) == 0)
    {
      // found a matching name
      return ii;
    }
  }
  // unknown name
  return -1;
}

//----------------------------------------------------------------------------
bool vtkMRMLLookingGlassViewNode::HasError()
{
  return !this->LastErrorMessage.empty();
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassViewNode::ClearError()
{
  this->SetError("");
}

//----------------------------------------------------------------------------
void vtkMRMLLookingGlassViewNode::SetError(const std::string& errorText)
{
  if (this->LastErrorMessage == errorText)
  {
    // no change
    return;
  }
  this->LastErrorMessage = errorText;
  this->Modified();
}

//----------------------------------------------------------------------------
std::string vtkMRMLLookingGlassViewNode::GetError() const
{
  return this->LastErrorMessage;
}
