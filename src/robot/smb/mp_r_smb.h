#if !defined(MP_R_SMB_H_)
#define MP_R_SMB_H_

/*!
 * @file
 * @brief File contains mp robot class declaration for SwarmItFix Mobile Base
 * @author twiniars <twiniars@ia.pw.edu.pl>, Warsaw University of Technology
 *
 * @ingroup smb
 */

#include "base/mp/mp_robot.h"
#include "robot/smb/const_smb.h"

namespace mrrocpp {
namespace mp {
namespace robot {
class smb : public robot
{
public:
	smb(task::task &mp_object_l);
};

} // namespace robot
} // namespace mp
} // namespace mrrocpp
#endif /*MP_R_SMB_H_*/