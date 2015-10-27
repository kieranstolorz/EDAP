/*
The purpose of this function is to read data over the serial bus from the FPGA, package it's, save it to file, and transmit it to the GCS. The wifi address of both the OBC (the computer this code runs on) and the GCS is to be assigned by SpaceX during the competition. The IP address of the OBC will be configured by a user using ifconfig, and the user will edit a file labeled OBC_IO.cfg that contains the GCS computer's IP address.
The first step simply uses the ifconfig system call to read its own wifi IP address and reads the GCS wifi IP address from the OBC_IO.cfg file. If this is blank, it can run the main loop and record FPGA data, but must wait until both of these are filled with data before it can transmit data to the GCS. (Maybe it should ping the GCS, and if it cannot will simply log in a file that GCS connection is lost. Similarly, if it is not receiving any FPGA data, it will do nothing. The task of sending notifications of FPGA loss to the GCS and safe-stop if GCS link loss should be handled by other functions.)
Assuming it knows it's own and the GCS's IP address, and has link it will transmit data along with saving.
FPGA data coming in on the serial bus will have a header followed by data. For our first bit of code, we are using the IMU data that is structured like so (I think):
#YPR=-155.73,-76.48,-129.51
Accel x,y,z=-2.00,3.00,294.00
What we will do is use a class to sort these into an array. Let's look at the gyro data first (#YPR). What we want to do is create a class with a data buffer and a time buffer. We want the buffer to fill up to 1.5kB of data before transmitting to the GCS. However, we don't want to wait too long before transmitting any particular piece of data. So there will be a time buffer to ensure that data of the same type is transmitted at least 10 times per second. Once we put this data into this array, we transmit it once the buffer is filled or the time requirement has been met, whichever is first, and also save it to a file.
What we want to do is open a file, name it with something that identifies it (so YPG), followed by a time stamp, which will be the time stamp of when the first bit of data was detected. So for this example, it will create a file called "YPR1445672229.dat". We don't want the file to get too large. It's really arbitrary, but we don't want to have to deal with too large of files, so let's close the file once it reaches 100MB in size and open a new one. The code should time stamp each line that it reads as it comes in. So, if you were to open YPR1445672229.dat it would look like this:
1445672229.00001, -155.73,-76.48,-129.51
1445672229.00002, -155.43,-75.43,-127.32
1445672229.00003, -155.73,-76.48,-129.51
...
This represents what it would look like on the day file and on the data stream being sent to the GCS.
This is how it will work with all data types.
*/
#include <fstream>
#include <iostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdlib.h>
#include <stdio.h>
#include <netdb.h>
#include "header.h"
#include <SerialStream.h>
using namespace std;
using namespace LibSerial ;
string ipaddress(string); //**********someone change this to a class, please**********/////////
bool is_master(string); //**********someone change this to a class, please**********/////////
void setup ()
{
    struct sockaddr_in myaddr, remaddr;
    const char *destination, *myipaddr, *otherOBCaddr, /*FPGA_addr*/ /*not sure if FPGA will have ethernet connection*/; //this will take the ip address string and put it into a const
    string value; //this is the string value of the ip address
    string dest_parameter = "GCS_wifi", own_parameter, own_wifi_parameter, other_OBC_parameter; //this is the line in the .cfg file we're looking for
    bool GCS_address_valid=0, my_adddress_valid=0, masterOBC, has connection;
    //bool FPGA_ipaddress_valid //not sure if FPGA will have ethernet connection
    string FPGA_port="COM1"; //change to correct COM port (set by design/not changeable)
    value=ipaddress(dest_parameter); //This finds the line in the OBC_IO.cfg file and returns the ip address, the GCS ip address in this case
    destination=value.c_str(); //This puts it into a const, so this is the GCS address
    //check if ipaddress is valid
    if (inet_aton(destination, &remaddr.sin_addr)==0)
    {
        GCS_address_valid=0;
    }
    else
    {
        GCS_address_valid=1;
    }
    /*test for validity
    cout << destination << endl << GCS_address_valid << endl;
    */
    masterOBC=is_master("is_default_master"); //returns true if the config file says so
    if (masterOBC==true)
    {
        own_wifi_parameter="OBC_1_wifi";
        own_paramter="OBC_1_ethernet";
        other_OBC_parameter="OBC_2_ethernet";
    }
    else
    {
        own_parameter="OBC_2_wifi";
        own_paramter="OBC_2_ethernet";
        other_OBC_parameter="OBC_1_ethernet";
    }
    //now we know if we know simultaneously if we are the master or secondary, and what to look for in the config file to find the ip address
    value=ipaddress(own_parameter);
    myipaddr=value.c_str(); //store my own ip address in case it's needed
    //Do a system call to ping the GCS
    //Call a function or object that checks time since last GCS ping returned positive
    //This part will not compile until work is done
    check_connection.sendping(destination);//command a ping be sent to the GCS, using destination ip
    if (check_connection.checkpingtime(destination)==true)//is a 1 if last ping return is less than 1 second old
    {
        has_connection==true;
    }
    else
    {
        has_connection=false;
    }
    //this part will not compile until work is done
    //For USB comms use this website http://libserial.sourceforge.net/tutorial.html#opening-a-serial-port-i-o-stream
    //First task, read from serial port, package, and save to file
    #include <SerialStream.h>
    //
    using namespace LibSerial ;
    //
    // Create and open the serial port for communication.
    //
    SerialStream my_serial_stream( "/dev/ttyS0" ) ;
    //
    // The various available baud rates are defined in SerialStreamBuf class.
    // This is to be changed soon. All serial port settings will be placed in
    // in the SerialPort class.
    //
    my_serial_stream.SetBaudRate( SerialStreamBuf::BAUD_115200 ) ;
    //
    // Use 8 bit wide characters.
    // ensures we're only getting ASCII
    my_serial_port.SetCharSize( SerialStreamBuf::CHAR_SIZE_8 ) ;
    //
    // Use one stop bit.
    //
    my_serial_port.SetNumOfStopBits(1) ; //Even I don't know what this does or if it's necessary
    //
    // Use odd parity during serial communication.
    //
    my_serial_port.SetParity( SerialStreamBuf::PARITY_ODD ) ;
    //
    // Use hardware flow-control.
    //
    my_serial_port.SetFlowControl( SerialStreamBuf::FLOW_CONTROL_HARD ) ; //I guess this is necessary
    //create MessageQueue
    QueueID = msgQCreate( MAX_MSG, MSGLEN, MSG_Q_PRIORITY);
    if( QueueID == NULL)
    return(ERROR);
    //enableTimeStamp
    sysTimestampEnable();
    //now we can go into sending and receiving
}

void task1()
{
    //In this function, we read from serial, and package it for later
    const int BUFFER_SIZE = 256 ;
    char input_buffer[BUFFER_SIZE] ;
    short counter =0;
    string strWords[20];
    string serial_line, yaw, pitch, roll, timestamp;
    string messageID = "YPR";
    int num_messages_of_this_type=0;
    ofstream myfile;
    //
    my_serial_stream.read( input_buffer,
    BUFFER_SIZE ) ;
    //convert input_buffer to string
    //******this should be part of struct*******//
    if (serial_line.find(messageID) != std::string::npos) //so if this line looks like what we're looking for
    {
        short counter = 0;
        for(short i=0;i<line.length();i++)
        {
            strWords[counter] += line[i]; // Append fixed
            if(line[i] == ',')
            {
                counter++;
            }
        }
    }
    yaw=strWords[1];
    pitch=strWords[2];
    roll=strWords[2];
    timestamp = //************figure out how to do a timestamp here******************/////////
    //******this should be part of struct*******//
    if (num_messages_of_this_type==0)
    {
        //*******do this here************////////
        myfile.open (timestamp, "YPT.dat");//create file named timestampYPR.dat
        out << timestamp, ",", yaw, ",", pitch "," roll << endl;
        //put struct into UDP buffer for YPR
    }
    if(/*put something here to see if file is greater than 100kB*/)
    {
        myfile.close();
        myfile.open (timestamp, "YPT.dat");//create file named timestampYPR.dat
        out << timestamp, ",", yaw, ",", pitch "," roll << endl;
    }
    else
    {
        //figure out how to find the latest file of this type open
        out.open(last_file_timestamp, "YPR.dat", std::ios::app);
        out << timestamp, ",", yaw, ",", pitch "," roll << endl;
    }
    if(/*******something here to check if YPR buffer is greater than 1.5kB***************/ or /***********Something here to check if the last YPR message was sent greater than 10ms ago)
    {
        /***********code here to send current YPR buffer to GCS (ip address supplied***********/
    }
    else
    /**********code here to store struct into next line of YPR buffer should look like this
    timestamp[0], yaw[0], pitch[0], roll[0]
    timestamp[1], yaw[1], pitch[1], roll[1]
    ************************/
}
    
task2()
{
    //similar to task 1, just for a different data type (like accelleration, air_bearing_1 pressure, etc.)
}
  
string ipaddress(string parameter) //**********someone change this to a class, please**********/////////
{
    string line;
    string result;
    string strWords[5];
    short counter = 0;
    std::ifstream myfile( "OBC_IO.cfg" );
    if (myfile) // same as: if (myfile.good())
    {
        while (getline( myfile, line )) // same as: while (getline( myfile, line ).good())
        {
            if (line.find(parameter) != std::string::npos)
            {
                short counter = 0;
                for(short i=0;i<line.length();i++)
                {
                    strWords[counter] += line[i]; // Append fixed
                    if(line[i] == ' ')
                    {
                        counter++;
                    }
                }
            }
        }
        myfile.close();
    }
    return strWords[1];
}
  
bool is_master(string parameter) //**********someone change this to a class, please**********/////////
{
    string line;
    string result;
    string strWords[5];
    short counter = 0;
    bool is1;
    std::ifstream myfile( "OBC_IO.cfg" );
    if (myfile) // same as: if (myfile.good())
    {
        while (getline( myfile, line )) // same as: while (getline( myfile, line ).good())
        {
            if (line.find(parameter) != std::string::npos)
            {
                short counter = 0;
                for(short i=0;i<line.length();i++)
                {
                    strWords[counter] += line[i]; // Append fixed
                    if(line[i] == ' ')
                    {
                        counter++;
                    }
                }
            }
        }
        myfile.close();
    }
    if strWords[1]=="1";
    is1=true;
    else
    is1=false;
    return is1;
}
