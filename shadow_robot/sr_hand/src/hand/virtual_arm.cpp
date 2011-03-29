/**
 * @file   virtual_arm.cpp
 * @author Ugo Cupcic <ugo@shadowrobot.com>, Contact <contact@shadowrobot.com>
 * @date   Tue Jun 29 14:56:10 2010
 *
*
* Copyright 2011 Shadow Robot Company Ltd.
*
* This program is free software: you can redistribute it and/or modify it
* under the terms of the GNU General Public License as published by the Free
* Software Foundation, either version 2 of the License, or (at your option)
* any later version.
*
* This program is distributed in the hope that it will be useful, but WITHOUT
* ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
* FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License for
* more details.
*
* You should have received a copy of the GNU General Public License along
* with this program.  If not, see <http://www.gnu.org/licenses/>.
*
 * @brief The virtual arm can be used as a simulator. It modelizes the Shadow Robot muscle arm.
 *
 */

#include "sr_hand/hand/virtual_arm.h"

#include <time.h>
#include <ros/ros.h>

#ifdef GAZEBO
#include <std_msgs/Float64.h>
#endif

namespace shadowrobot
{
VirtualArm::VirtualArm() :
    SRArticulatedRobot()
{
#ifdef GAZEBO 
    ROS_INFO("This ROS interface is built for Gazebo.");
    //initialises the subscriber to the Gazebo joint_states messages
    std::string prefix;
    std::string searched_param;
    n_tilde = ros::NodeHandle("~");

    n_tilde.searchParam("gazebo_joint_states_prefix", searched_param);
    n_tilde.param(searched_param, prefix, std::string());
    std::string full_topic = prefix + "joint_states";
    gazebo_subscriber = node.subscribe(full_topic, 2, &VirtualArm::gazeboCallback, this);
#else
    ROS_INFO("This ROS interface is not built for Gazebo.");
#endif

    srand(time(NULL));
    initializeMap();
}

VirtualArm::~VirtualArm()
{
}

void VirtualArm::initializeMap()
{
    joints_map_mutex.lock();
    JointData tmpData;

#ifdef GAZEBO
    std::string topic_prefix = "/";
    std::string topic_suffix = "/command";
    std::string full_topic = "";
#endif

    tmpData.min = -45.0;
    tmpData.max = 90.0;
#ifdef GAZEBO 
    full_topic = topic_prefix + "trunk_rotation_controller" + topic_suffix;
    gazebo_publishers.push_back(node.advertise<std_msgs::Float64>(full_topic, 2));
    int tmp_index = 0;
    tmpData.publisher_index = tmp_index;
#endif
    joints_map["trunk_rotation"] = tmpData;
    tmpData.min = 0.0;
    tmpData.max = 90.0;
#ifdef GAZEBO 
    full_topic = topic_prefix + "shoulder_rotation_controller" + topic_suffix;
    gazebo_publishers.push_back(node.advertise<std_msgs::Float64>(full_topic, 2));
    tmp_index ++;
    tmpData.publisher_index = tmp_index;
#endif
    joints_map["shoulder_rotation"] = tmpData;
    tmpData.min = 0.0;
    tmpData.max = 120.0;
#ifdef GAZEBO 
    full_topic = topic_prefix + "elbow_abduction_controller" + topic_suffix;
    gazebo_publishers.push_back(node.advertise<std_msgs::Float64>(full_topic, 2));
    tmp_index ++;
    tmpData.publisher_index = tmp_index;
#endif
    joints_map["elbow_abduction"] = tmpData;
    tmpData.min = -90.0;
    tmpData.max = 90.0;
#ifdef GAZEBO 
    full_topic = topic_prefix + "forearm_rotation_controller" + topic_suffix;
    gazebo_publishers.push_back(node.advertise<std_msgs::Float64>(full_topic, 2));
    tmp_index ++;
    tmpData.publisher_index = tmp_index;
#endif
    joints_map["forearm_rotation"] = tmpData;

    joints_map_mutex.unlock();
}

short VirtualArm::sendupdate( std::string joint_name, double target )
{
    joints_map_mutex.lock();

    JointsMap::iterator iter = joints_map.find(joint_name);

#ifdef GAZEBO
    std_msgs::Float64 target_msg;
#endif

    //not found
    if( iter == joints_map.end() )
    {
        ROS_DEBUG("Joint %s not found.", joint_name.c_str());
        joints_map_mutex.unlock();
        return -1;
    }

    //joint found
    JointData tmpData(iter->second);
    if( target < tmpData.min )
        target = tmpData.min;
    if( target > tmpData.max )
        target = tmpData.max;

#ifdef GAZEBO
    //gazebo targets are in radians
    target_msg.data = toRad(target);
    gazebo_publishers[tmpData.publisher_index].publish(target_msg);
    ros::spinOnce();
#else
    tmpData.position = target;
#endif
    tmpData.target = target;

    joints_map[joint_name] = tmpData;

    joints_map_mutex.unlock();
    return 0;
}

JointData VirtualArm::getJointData( std::string joint_name )
{
    joints_map_mutex.lock();
    JointsMap::iterator iter = joints_map.find(joint_name);

    //joint found
    if( iter != joints_map.end() )
    {
        //return the position
        iter->second.temperature = ((double)(rand() % 100) / 100.0);
        iter->second.current = ((double)(rand() % 100) / 100.0);
        iter->second.force = ((double)(rand() % 100) / 100.0);

        JointData tmpData = JointData(iter->second);
        joints_map_mutex.unlock();
        return tmpData;
    }

    ROS_ERROR("Joint %s not found.", joint_name.c_str());
    JointData noData;
    joints_map_mutex.unlock();
    return noData;
}

SRArticulatedRobot::JointsMap VirtualArm::getAllJointsData()
{
    joints_map_mutex.lock();
    JointsMap tmpMap;

    for( JointsMap::const_iterator it = joints_map.begin(); it != joints_map.end(); ++it )
    {
        JointData tmpData = it->second;
        tmpData.temperature = ((double)(rand() % 100) / 100.0);
        tmpData.current = ((double)(rand() % 100) / 100.0);
        tmpData.force = ((double)(rand() % 100) / 100.0);
        tmpData.jointIndex = 0;
        tmpData.flags = "";

        joints_map[it->first] = tmpData;
    }

    tmpMap = JointsMap(joints_map);
    joints_map_mutex.unlock();
    return tmpMap;
}

short VirtualArm::setContrl( std::string contrlr_name, JointControllerData ctrlr_data )
{
    ROS_WARN("The setContrl method is not yet implemented");
    return 0;
}

JointControllerData VirtualArm::getContrl( std::string contrlr_name )
{
    ROS_WARN("The getContrl method is not yet implemented");
    JointControllerData no_result;
    return no_result;
}

short VirtualArm::setConfig( std::vector<std::string> myConfig )
{
    ROS_WARN("The set config function is not implemented in the virtual arm.");
    return 0;
}

void VirtualArm::getConfig( std::string joint_name )
{
    ROS_WARN("The get config function is not implemented in the virtual arm.");
}

std::vector<DiagnosticData> VirtualArm::getDiagnostics()
{
    joints_map_mutex.lock();
    std::vector<DiagnosticData> returnVect;

    for( JointsMap::const_iterator it = joints_map.begin(); it != joints_map.end(); ++it )
    {
        DiagnosticData tmpDiag;
        tmpDiag.joint_name = it->first;
        tmpDiag.level = 0;
        tmpDiag.flags = "";
        tmpDiag.target_sensor_num = 0;
        tmpDiag.target = it->second.target;
        tmpDiag.position_sensor_num = 0;
        tmpDiag.position = it-> second.position;

        returnVect.push_back(tmpDiag);
    }

    joints_map_mutex.unlock();
    return returnVect;
}

#ifdef GAZEBO
void VirtualArm::gazeboCallback(const sensor_msgs::JointStateConstPtr& msg)
{
    joints_map_mutex.lock();
    //loop on all the names in the joint_states message
    for(unsigned int index = 0; index < msg->name.size(); ++index)
    {
        std::string joint_name = msg->name[index];
        JointsMap::iterator iter = joints_map.find(joint_name);

        //not found => can be a joint from the arm / hand
        if(iter == joints_map.end())
        continue;

        //joint found
        JointData tmpData(iter->second);

        tmpData.position = toDegrees(msg->position[index]);
        tmpData.force = msg->effort[index];

        joints_map[joint_name] = tmpData;
    }
    joints_map_mutex.unlock();
}
#endif
} //end namespace
