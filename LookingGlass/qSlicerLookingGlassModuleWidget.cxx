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

// Qt includes
#include <QDebug>

// Slicer includes
#include "qSlicerLookingGlassModuleWidget.h"
#include "ui_qSlicerLookingGlassModuleWidget.h"

//-----------------------------------------------------------------------------
/// \ingroup Slicer_QtModules_ExtensionTemplate
class qSlicerLookingGlassModuleWidgetPrivate: public Ui_qSlicerLookingGlassModuleWidget
{
public:
  qSlicerLookingGlassModuleWidgetPrivate();
};

//-----------------------------------------------------------------------------
// qSlicerLookingGlassModuleWidgetPrivate methods

//-----------------------------------------------------------------------------
qSlicerLookingGlassModuleWidgetPrivate::qSlicerLookingGlassModuleWidgetPrivate()
{
}

//-----------------------------------------------------------------------------
// qSlicerLookingGlassModuleWidget methods

//-----------------------------------------------------------------------------
qSlicerLookingGlassModuleWidget::qSlicerLookingGlassModuleWidget(QWidget* _parent)
  : Superclass( _parent )
  , d_ptr( new qSlicerLookingGlassModuleWidgetPrivate )
{
}

//-----------------------------------------------------------------------------
qSlicerLookingGlassModuleWidget::~qSlicerLookingGlassModuleWidget()
{
}

//-----------------------------------------------------------------------------
void qSlicerLookingGlassModuleWidget::setup()
{
  Q_D(qSlicerLookingGlassModuleWidget);
  d->setupUi(this);
  this->Superclass::setup();
}
