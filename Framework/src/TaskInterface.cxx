///
/// \file   TaskInterface.cxx
/// \author Barthelemy von Haller
///

#include <utility>

#include "QualityControl/TaskInterface.h"

namespace o2 {
namespace quality_control {
namespace core {

TaskInterface::TaskInterface(ObjectsManager *objectsManager) :
  mObjectsManager(objectsManager)
{
}

TaskInterface::TaskInterface() :
  mObjectsManager(nullptr)
{
}

const std::string &TaskInterface::getName() const
{
  return mName;
}

void TaskInterface::setName(const std::string &name)
{
  mName = name;
}

void TaskInterface::setObjectsManager(std::shared_ptr<ObjectsManager> objectsManager)
{
  mObjectsManager = objectsManager;
}

std::shared_ptr<ObjectsManager> TaskInterface::getObjectsManager()
{
  return mObjectsManager;
}

} // namespace core
} // namespace quality_control
} // namespace o2
