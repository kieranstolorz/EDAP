/*RoughDraft.v1.0 */


#include "vxWorks.h"
#include "msgQLib.h"
#include "time.h"
#include "iostream"
#include "fstream"
#include "ioLib.h"

#define MSGLEN 1024
#define MAX_MSG 10

MSG_Q_ID QueueID;
using namespace std;

struct YPR
 {
   double pitch;
   double roll;
   double yaw;
 }ypr;