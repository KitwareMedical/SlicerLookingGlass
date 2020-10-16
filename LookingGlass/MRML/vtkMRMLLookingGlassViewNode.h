/*=auto=========================================================================

  Portions (c) Copyright 2005 Brigham and Women's Hospital (BWH) All Rights Reserved.

  See COPYRIGHT.txt
  or http://www.slicer.org/copyright/copyright.txt for details.

  Program:   3D Slicer
  Module:    $RCSfile: vtkMRMLLookingGlassViewNode.h,v $
  Date:      $Date: 2006/03/19 17:12:28 $
  Version:   $Revision: 1.6 $

=========================================================================auto=*/

#ifndef __vtkMRMLLookingGlassViewNode_h
#define __vtkMRMLLookingGlassViewNode_h

// MRML includes
#include <vtkMRMLViewNode.h>
#include <vtkMRMLLinearTransformNode.h>

// VTK includes
#include <vtkEventData.h>

#include "vtkSlicerLookingGlassModuleMRMLExport.h"

/// \brief MRML node to represent a 3D view.
///
/// View node contains view parameters.
class VTK_SLICER_LOOKINGGLASS_MODULE_MRML_EXPORT vtkMRMLLookingGlassViewNode
  : public vtkMRMLViewNode
{
public:

  static vtkMRMLLookingGlassViewNode* New();
  vtkTypeMacro(vtkMRMLLookingGlassViewNode, vtkMRMLViewNode);
  void PrintSelf(ostream& os, vtkIndent indent) override;

  //--------------------------------------------------------------------------
  /// MRMLNode methods
  //--------------------------------------------------------------------------

  virtual vtkMRMLNode* CreateNodeInstance() override;

  /// Read node attributes from XML file
  virtual void ReadXMLAttributes(const char** atts) override;

  /// Write this node's information to a MRML file in XML format.
  virtual void WriteXML(ostream& of, int indent) override;

  /// Copy the node's attributes to this object
  virtual void Copy(vtkMRMLNode* node) override;

  /// Get node XML tag name (like Volume, Model)
  virtual const char* GetNodeTagName() override;

  /// Update the references of the node to the scene.
  virtual void SetSceneReferences() override;

  /// Return the color the view nodes have for the background by default.
  static double* defaultBackgroundColor();
  static double* defaultBackgroundColor2();

  /// Get reference view node.
  /// Reference view node is a regular 3D view node, which content
  /// or view may be synchronized with the looking glass camera view.
  vtkMRMLViewNode* GetReferenceViewNode();
  /// Set reference view node.
  /// \sa GetReferenceViewNode
  void SetAndObserveReferenceViewNodeID(const char* layoutNodeId);
  /// Set reference view node.
  /// \sa GetReferenceViewNode
  bool SetAndObserveReferenceViewNode(vtkMRMLViewNode* node);

  /// Rendering mode options
  /// OnlyWhenRequested
  /// OnlyStillRenders
  /// Always
  enum
  {
    RenderingModeOnlyWhenRequested = 0,
    RenderingModeOnlyStillRenders = 1,
    RenderingModeAlways = 2,
    RenderingMode_Last
  };

  /// Get/Set flag that controls rendering mode
  vtkSetClampMacro(RenderingMode, int, 0, vtkMRMLLookingGlassViewNode::RenderingMode_Last - 1);
  vtkGetMacro(RenderingMode, int);
  std::string GetRenderingModeAsString();

  /// Convert between rendering mode ID and name
  static const char* GetRenderingModeAsString(int id);
  static int GetRenderingModeFromString(const char* name);

  /// Desired frame rate. Volume renderer may use this information
  /// for determining sampling distances (and other LOD actors, to
  /// determine display quality).
  vtkGetMacro(DesiredUpdateRate, double);
  vtkSetMacro(DesiredUpdateRate, double);

  /// Turn on/off use of near and far clipping limits.
  vtkGetMacro(UseClippingLimits, bool);
  vtkSetMacro(UseClippingLimits, bool);

  /// Near clipping limit.
  /// A limit for the ratio of the near clipping plane to the focal
  /// distance. This is a mechanism to limit parallax and resulting
  /// ghosting when using the looking glass display. The typical value
  /// should be around 0.8.
  vtkGetMacro(NearClippingLimit, double);
  vtkSetMacro(NearClippingLimit, double);

  /// Far clipping limit.
  /// A limit for the ratio of the far clipping plane to the focal
  /// distance. This is a mechanism to limit parallax and resulting
  /// ghosting when using the looking glass display. The typical value
  /// should be around 1.2.
  vtkGetMacro(FarClippingLimit, double);
  vtkSetMacro(FarClippingLimit, double);

  /// Return true if an error has occurred.
  /// "Connected" member requests connection but this method can tell if the
  /// hardware connection has been actually successfully established.
  bool HasError();

  /// Clear error state.
  void ClearError();

  /// Set error message. Non-empty string means that an error has occurred.
  void SetError(const std::string& errorText);

  /// Get error message. Non-empty string means that an error has occurred.
  std::string GetError() const;

protected:
  int RenderingMode;
  double DesiredUpdateRate;
  bool UseClippingLimits;
  double NearClippingLimit;
  double FarClippingLimit;

  std::string LastErrorMessage;

  vtkMRMLLookingGlassViewNode();
  ~vtkMRMLLookingGlassViewNode();
  vtkMRMLLookingGlassViewNode(const vtkMRMLLookingGlassViewNode&);
  void operator=(const vtkMRMLLookingGlassViewNode&);

  static const char* ReferenceViewNodeReferenceRole;
};

#endif
