/**
  * \file weatherStation.cpp
  * \brief This file contains functions used to manages the netatmo weatherStation in oroclient
  * \author DUMONT Emmanuel
  * \date 05/2016
  */




#include <signal.h>

#include "ros/ros.h"
#include "std_msgs/String.h"

#include <iostream>
#include <stdexcept>
#include <stdio.h>
#include <string>


#include <orolib/orolib.hpp>


using namespace std;


//#define CURL_COMMAND "curl -s -u dumont:isir302 ftp://127.0.0.1:21//home/manu/Desktop/LocalServer/irPos/"
#define CURL_COMMAND "curl -s -u Chaire_EBiomed:EBiomed2015 -k sftp://172.18.135.237:22/LivingLab_Data/InfraredSensors/"
#define PERIOD 10 // Time between each measurement

int temps[26] = {0,0,2,3,8,0,16,2,1,2,0,2,0,2,0,2,1,3,0,4,3,2,1,1,2,0};

std::string lastZone;

// Name of the current node in the ontology
char * gName;
bool gToBeFree; // Boolean which indicates if gName has to be freed

std::string exec(int cpt);
void get_data(std::string msg);
void updateSourceData(char value);
void get_first_data(std::string msg);

// Create a function which manages a clean "CTRL+C" command -> sigint command
void sigint_handler(int dummy)
{
    ROS_INFO("IR server is shutting down...");
    
    // Free gName
    if(gToBeFree == true)
    {
      gToBeFree = false;
      free(gName);
    }
        
    ROS_INFO("\n\n... Bye bye !\n   -Manu");
    exit(EXIT_SUCCESS); // Shut down the program
}




int main(int argc, char **argv)
{
	// Initialize global variables
	gToBeFree = false;

  ros::init(argc, argv, "irServer");

  ros::NodeHandle nOroCl;  // Communicate wit oroclient
  
  // Override the signal interrupt from ROS
  signal(SIGINT, sigint_handler);
  
  // If a name is passed as an argument
  if(argc > 1)
  {
    gName = argv[argc-1];
    gToBeFree = false;
  }
  else
  {  	
    gName = (char *) malloc ( 7 * sizeof(char));
    memset(gName, 0, 7);
  	snprintf(gName, 7 ,"irSensor");
  	 
  	gToBeFree = true;
	}
 
  gOroChatter_pub = nOroCl.advertise<std_msgs::String>("oroChatter", 10);
  
  usleep(500000); // necessary to be able to send data
  
  std::string sensor = "Infrared";
  // say my name
  sayMyName(gName, sensor);
  
  //todo lastZone first time
  string msg;
  lastZone = "";
  msg = exec(0);
  get_first_data(msg);
  
  cout << "fin get first data"<<endl;
  
  //for(int cpt=1; cpt<27;cpt++)
  while(1)
  {
    //Get ir_pos data
    //msg = exec(cpt);
    msg = exec(0);
    
    get_data(msg);
    
    //cout << "[msg]\n"<< msg << "[\\msg]\n"<< endl;
    
    //sleep(temps[cpt-1]);
    sleep(1);
  }
  

  
  return 0;
}


// Execute PHP from C
std::string exec(int cpt)
{
  char buffer[128];
  std::string result = "";
  //std::string cmd = string(CURL_COMMAND , cpt , ".txt");
  
  
  std::stringstream temp_str;
  //temp_str << CURL_COMMAND << cpt << ".txt";
  temp_str << CURL_COMMAND << "irPos.txt";
  std::string cmd = temp_str.str();
    
  FILE* pipe = popen(cmd.c_str(), "r");
  if (!pipe) throw std::runtime_error("popen() failed!");
  try {
      while (!feof(pipe)) {
          if (fgets(buffer, 128, pipe) != NULL)
              result += buffer;
      }
  } catch (...) {
      pclose(pipe);
      throw;
  }
  pclose(pipe);
  return result;
}


void get_data(std::string msg)
{
  std::size_t zoneFound = 0;
  std::size_t secFound = 0;
  std::size_t diff = 0;
  
  char buffer[20];
  
  std::string data;
  std::string value;
  
  
  // Find first zone and first second 's' after last detection
  std::string str("Zone");
  std::string str2("s");
  
  //cout << "msg:" << msg << ":" << msg.length() << endl;
  //cout << msg << endl;
  
  // Find first occurence of last recorded data :
  secFound = msg.find(lastZone);
  
  // Loop
  while(1)
  {
    
    memset(buffer,'\0', 20);
    
    // if zoneFound > msg.length(), string will never been found
    if(secFound < msg.length())
    {
      zoneFound = msg.find(str,secFound);
      //cout << "zone " << zoneFound << endl;
      
      if( (zoneFound!=std::string::npos) && (zoneFound < msg.length()))
      {
        // Add "Zone" as position
        zoneFound += 4;
        secFound = msg.find(str2, zoneFound);
        
        if (secFound!=std::string::npos)
        {
          diff = secFound-zoneFound;
          
          // Check if diff is too large
          if( (diff > 0) && (diff < 20) )
          {
            // Copy data in a buffer to compare
            msg.copy(buffer,diff,zoneFound);
            
            // Compare if already saved
            if(lastZone.compare(buffer) !=0)
            {
              // If different, store data
              //cout << "lastZone was " << lastZone;
              cout << "   Zone is '" << buffer[0] <<"'" <<endl;
              
              updateSourceData(buffer[0]);
              
              // Save the current last string
              lastZone = "";
              lastZone = string(buffer);
            }
            else
            {
              // Same as previously
              cout << "Same as previously" << endl;
              continue;
            }
          }
          else{
            printf("-Warning: File isn't conform to the basic architecture \"XXX ZoneY HHh MMm et SSs\"");
            continue;
          }
        }
        else{
          // No more 's's found
          cout << "No more sec found"<<endl;
          break;
        }
      }
      else{
        // No more "Zone" found
        cout << "No more zone found"<<endl;
        break;
      }
    }
    else{
    cout << "rien" << endl;
      break;
    }
  }
}


void get_first_data(std::string msg)
{
  std::size_t zoneFound = 0;
  std::size_t secFound = 0;
  std::size_t diff = 0;
  
  char buffer[20];
  
  std::string data;
  std::string value;
  
  
  // Find first zone and first second 's' after last detection
  std::string str("Zone");
  std::string str2("s");
  
  //cout << "msg:" << msg << ":" << msg.length() << endl;
  //cout << msg << endl;
  
  // Find first occurence of last recorded data :
  secFound = 0;
  secFound = 0;
  // Loop
  while(1)
  {
    
    memset(buffer,'\0', 20);
    
    // if zoneFound > msg.length(), string will never been found
    if(secFound < msg.length())
    {
      //cout << msg; 
      zoneFound = msg.find(str, secFound);
      //zoneFound = msg.find(str);
      //cout << "zone " << zoneFound << endl;
      
      if( (zoneFound!=std::string::npos) && (zoneFound < msg.length()))
      {
        // Add "Zone" as position
       // cout << "zoneFound "<< zoneFound << endl;
        zoneFound += 4;
        secFound = msg.find(str2,zoneFound);        
        
        if (secFound!=std::string::npos)
        {
          //cout << "secFound" << endl;
          diff = secFound-zoneFound;
          
          // Check if diff is too large
          if( (diff > 0) && (diff < 20) )
          {
            // Copy data in a buffer to compare
            msg.copy(buffer,diff,zoneFound);
            
            lastZone = "";
            lastZone = string(buffer);
          }
          else{
            printf("-Warning: File isn't conform to the basic architecture \"XXX ZoneY HHh MMm et SSs\"");
            continue;
          }
        }
        else{
          // No more 's's found
          //cout << "No more sec found"<<endl;
          break;
        }
      }
      else{
        // No more "Zone" found
        //cout << "No more zone found"<<endl;
        break;
      }
    }
    else{
      break;
    }
  }
}

// Update a specific data of a source
/*void updateData( std::string value)
{
  std::stringstream ss;
  char enumCmd = 0;
  
  // Clear previous data otherwise update won't be done correctly
  ss.str("");
  enumCmd = (char)CMD_CLEAR;
  ss << "BigBrother#"<<enumCmd<<"#"<< gName <<"#"<< data <<"#?y";
  std_msgs::String msg;
  msg.data = ss.str();
  oroChatterSender(msg);
  
  // "Update" in oro server seems to not work correctly, re-add instead
  ss.str("");
  enumCmd = (char)CMD_ADD_PROP;
  ss << "BigBrother#" << enumCmd <<"#"<< gName << "#"<< data <<"#"<< value;
  
  msg.data = ss.str();
  oroChatterSender(msg);
}*/

void updateSourceData(char value)
{
  std::stringstream ss;
  char enumCmd = 0;
  
  //ROS_INFO("Update %c in source : s%d", data,id);
  
  // Clear timestamp otherwise update won't be done correctly
  ss.str("");
  enumCmd = (char)CMD_CLEAR;
  ss << "BigBrother"<< DELIMITER <<enumCmd<< DELIMITER << "h"<< gName << DELIMITER << "?x" << DELIMITER <<"?y";
  std_msgs::String msg;
  msg.data = ss.str();
  oroChatterSender(msg);
  
  // "Update" in oro server seems to not work correctly, re-add instead
  ss.str("");
  enumCmd = (char)CMD_ADD_PROP;
  if(value == 5)ss << "BigBrother"<< DELIMITER <<enumCmd<< DELIMITER << "h" << gName << DELIMITER << "isIn" << DELIMITER << "Kitchen" <<", h"<< gName <<" rdf:type Human";
  if((value == 6) || (value = 4)) ss << "BigBrother"<< DELIMITER <<enumCmd<< DELIMITER << "h" << gName << DELIMITER << "isIn" << DELIMITER << "LivingRoom" <<", h"<< gName <<" rdf:type Human";
  if(value == 27)  ss << "BigBrother"<< DELIMITER <<enumCmd<< DELIMITER << "h" << gName << DELIMITER << "isIn" << DELIMITER << "Entry" <<", h"<< gName <<" rdf:type Human";
  msg.data = ss.str();
  oroChatterSender(msg);
}
