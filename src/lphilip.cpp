/* task1 = readmessage from FPGA
*  task2 = saveData
*  task3 = Send Data to GCS
*
*  From other core, raise signal when faulty condition occurs. task3 will register itself for that event. when signal handler is invoked,   *  task3 is notified. ors oemthing ike that.
*  
*/
 
#include "header.h"
 
void setup()
{
   //create MessageQueue
   QueueID = msgQCreate( MAX_MSG, MSGLEN, MSG_Q_PRIORITY);
   if( QueueID == NULL)
  return(ERROR);
  
  //enableTimeStamp
  sysTimestampEnable();
  
  //All error messages are being re-directed to a file. Need to check if this is the requirement!
  ofstream errorData.open("name of error file", O_RDWR);
  ioGlobalStdSet (2, errorData); 
  
}

readYPRdata(string buffer)
{
  int array[3], count = 0; 
  char* data;

  data = strtok(buffer.c_str(),',');
  while(data != NULL)
  {
    array[count++] = (int)data;
    data = strtok(NULL, ',');
  }
  ypr.pitch = array[0]; ypr.roll = array[1]; ypr.yaw = array[2];
  msgQsend(QueueID, ypr,sizeof(ypr),1000, MSG_PRI_URGENT);
  
}

task1()
{
  ifstream fdFPGA, 
  ofstream fdData, fdSave; // file descriptor
  UINT32 time_val = sysTimestampLock(); //ensures that interrupts are disabled
  char *name = "YPGA" + (char)time_val; /*******do you mean FPGA? ***********//
  char buffer[1024];
  char *temp;
  string str;
  fdData.open(name, ios::in|ios::out);// file to save data
  
  fdFPGA.open("/tyCo",O_RDWR,0);
  ioctl(fdFPGA, FIOOPTIONS, OPT_RAW);
  ioctl(fdFPGA, FIOBAUDRATE, 115200); //setting baud rate
  
  
  while(!fdFPGA.eof())
  {
    fdFPGA.getline(buffer,sizeof(buffer));
    str(buffer); //converting to c++ string.

    /* YPG Data */
    if (buffer.find("#YPG") != string::npos)
    { //YPR data yo!
      readYPRdata(buffer.erase(0,5)); // delete #YPR=
    }

    /* Some other Data */
    else if(buffer.find("#something_else") != string::npos)
    {
      //someother data
    }
    
    /*extract x y z values. 
    pack into struct.
    send message in messageQueue.*/
  }
  
  //end of FPGA file. Now what? Does it call itself again ?
  
}
