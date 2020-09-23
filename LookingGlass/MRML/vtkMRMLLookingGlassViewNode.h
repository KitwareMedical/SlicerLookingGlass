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
  static const char* GetVirtualRealityInteractionTransformAttributeName() { return "VirtualReality.InteractionTransform"; };

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
  virtual void SetSceneReferences();

  /// Return the color the view nodes have for the background by default.
  static double* defaultBackgroundColor();
  static double* defaultBackgroundColor2();

  /// Get reference view node.
  /// Reference view node is a regular 3D view node, which content
  /// or view may be synchronized with the virtual reality camera view.
  vtkMRMLViewNode* GetReferenceViewNode();
  /// Set reference view node.
  /// \sa GetReferenceViewNode
  void SetAndObserveReferenceViewNodeID(const char* layoutNodeId);
  /// Set reference view node.
  /// \sa GetReferenceViewNode
  bool SetAndObserveReferenceViewNode(vtkMRMLViewNode* node);

  /// Controls two-sided lighting property of the renderer
  vtkGetMacro(TwoSidedLighting, bool);
  vtkSetMacro(TwoSidedLighting, bool);
  vtkBooleanMacro(TwoSidedLighting, bool);

  /// If enabled then 4 lights are used, otherwise just 2.
  vtkGetMacro(BackLights, bool);
  vtkSetMacro(BackLights, bool);
  vtkBooleanMacro(BackLights, bool);

  /// Desired frame rate. Volume renderer may use this information
  /// for determining sampling distances (and other LOD actors, to
  /// determine display quality).
  vtkGetMacro(DesiredUpdateRate, double);
  vtkSetMacro(DesiredUpdateRate, double);

  /// Magnification of world [0.01, 100].
  /// Translated to physical scale of the VR render window
  vtkGetMacro(Magnification, double);
  vtkSetMacro(Magnification, double);

  /// Motion speed of fly (i.e. dolly) in meters per second.
  /// Default is walking speed: 6 km/h = 1.66 m/s
  vtkGetMacro(MotionSpeed, double);
  vtkSetMacro(MotionSpeed, double);

  /// Motion sensitivity (between 0.0 and 1.0).
  /// If virtual reality headset is not moving then update rate
  /// is decreased to allow higher quality rendering.
  vtkGetMacro(MotionSensitivity, double);
  vtkSetMacro(MotionSensitivity, double);

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
  bool TwoSidedLighting;
  bool BackLights;
  double DesiredUpdateRate;
  double Magnification;
  double MotionSpeed;
  double MotionSensitivity;

  std::string LastErrorMessage;

  vtkMRMLLookingGlassViewNode();
  ~vtkMRMLLookingGlassViewNode();
  vtkMRMLLookingGlassViewNode(const vtkMRMLLookingGlassViewNode&);
  void operator=(const vtkMRMLLookingGlassViewNode&);

  static const char* ReferenceViewNodeReferenceRole;
};

#endif
