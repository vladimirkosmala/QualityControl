///
/// \file   DatabaseFactory.h
/// \author Barthelemy von Haller
///

#ifndef QUALITYCONTROL_LIBS_CORE_DatabaseFactory_H_
#define QUALITYCONTROL_LIBS_CORE_DatabaseFactory_H_

#include <iostream>
// O2
#include "Common/Exceptions.h"
// QC
#include "QualityControl/DatabaseInterface.h"

namespace o2 {
namespace quality_control {
namespace repository {

/// \brief Factory to get a database accessor
class DatabaseFactory
{
  public:

    /// \brief Create a new instance of a DatabaseInterface.
    /// The DatabaseInterface actual class is decided based on the parameters passed.
    /// The ownership is returned as well.
    /// \todo use unique_ptr
    /// \param name Possible values : "mysql"
    /// \author Barthelemy von Haller
    static DatabaseInterface* create(std::string name);
};

} // namespace core
} // namespace QualityControl
} // namespace o2

#endif // QUALITYCONTROL_LIBS_CORE_DatabaseFactory_H_
