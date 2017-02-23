/*
 * Software License Agreement (BSD License)
 *
 * Copyright (c) 2017, Francisco Vina, francisco.vinab@gmail.com
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *       * Redistributions of source code must retain the above copyright
 *       notice, this list of conditions and the following disclaimer.
 *       * Redistributions in binary form must reproduce the above copyright
 *       notice, this list of conditions and the following disclaimer in the
 *       documentation and/or other materials provided with the distribution.
 *       * Neither the name of the Southwest Research Institute, nor the names
 *       of its contributors may be used to endorse or promote products derived
 *       from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef __YUMI_HW_EGM_H
#define __YUMI_HW_EGM_H

#include <yumi_hw/yumi_hw.h>

#include <boost/thread/mutex.hpp>
#include <boost/thread.hpp>

#include <boost/shared_ptr.hpp>

#include <ros/ros.h>
#include <abb_rws_interface/rws_interface_yumi.h>
#include <abb_egm_interface/egm_interface_default.h>

#ifndef N_YUMI_JOINTS
#define N_YUMI_JOINTS 14
#endif

#ifndef MAX_NUMBER_OF_EGM_CONNECTIONS
#define MAX_NUMBER_OF_EGM_CONNECTIONS 4
#endif


// Wrapper class for setting up EGM and RWS connections to the Yumi robot
// with their corresponding IO service threads
// It assumes velocity control mode
class YumiEGMInterface
{

    using namespace abb::egm_interface;
    using namespace abb::rws_interface;

public:


    YumiEGMInterface();

    ~YumiEGMInterface();

    /** \brief Gets parameters for EGM & RWS connections from the ROS parameter server.
     * The RAPID module TRobEGM.sys contains a brief description of the EGM
     * parameters:
     * LOCAL RECORD EGM_RECORD
     *   num comm_timeout;         ! Communication timeout [s].
     *
     *   string tool_name;         ! Name of a defined tool (e.g. "tool0").
     *   string wobj_name;         ! Name of a defined work object (e.g. "wobj0").
     *   num cond_min_max;         ! Condition value [deg or mm] for when the EGM correction is considered
     *                             ! to be finished. E.g. for joint mode, then the condition is fulfilled when
     *                             ! the joints are within [-MinMax, MinMax].
     *   num lp_filter;            ! Low pass filer bandwidth of the EGM controller [Hz].
     *   num max_speed_deviation;  ! Maximum admitted joint speed change [deg/s]:
     *                             !   Note: Should not be set higher than the lowest max speed [deg/s],
     *                             !         out of all the axis max speeds (found in the robot's data sheet).
     *
     *   num cond_time;            ! Condition time [s].
     *   num ramp_in_time;         ! Duration for ramp in [s].
     *   num pos_corr_gain;        ! Position correction gain of the EGM controller.
     * ENDRECORD
     *
     */
    void getParams();

    /** \brief Initializes EGM + RWS connection to the robot
     *
     */
    bool init();

    bool stop();


    /** \brief gets the current joint positions from the robot
     */
    void getCurrentJointPos(float (&joint_pos)[N_YUMI_JOINTS]);

    /** \brief gets the current joint velocities from the robot
     */
    void getCurrentJointVel(float (&joint_vel)[N_YUMI_JOINTS]);

    /** \brief sets the joint velocity commands
     */
    void setJointTargets(float (&joint_targets)[N_YUMI_JOINTS]);

protected:

    bool initRWS();

    bool initEGM();

    /** \brief Sends EGM parameters collected from the ROS parameter server to the robot controller through RWS (TCP connection), such as
     * LP filter bandwidth, condition time, etc. For more details see getParams()
     */
    bool sendEGMParams();

    void configureEGM();

    bool startEGM();

    bool stopEGM();


private:


    /* RWS */
    // RWS interface which uses TCP communication for starting the EGM joint mode on YuMi
    boost::shared_ptr<RWSInterfaceYuMi> rws_interface_yumi_;

    // RWS connection parameters
    std::string rws_ip_, rws_port_;
    double rws_delay_time_;
    unsigned int rws_max_signal_retries_;
    bool rws_connection_ready_;

    /* EGM */

    // EGM interface which uses UDP communication for realtime robot control @ 250 Hz
    boost::shared_ptr<EGMInterfaceDefault> left_arm_egm_interface_;
    boost::shared_ptr<EGMInterfaceDefault> right_arm_egm_interface_;



    // io service used for EGM
    boost::asio::io_service io_service_;
    boost::thread_group io_service_threads_;

    EGMData egm_params_;

    double max_joint_velocity_;

    bool has_params_;

};


class YumiHWEGM : public YumiHW
{
public:
    YumiHWEGM();

    ~YumiHWEGM();

    void initRWS();

    bool init();

    void read(ros::Time time, ros::Duration period);

    void write(ros::Time time, ros::Duration period);


private:
    bool is_initialized_, is_setup_, first_run_in_position_mode_;

    float sampling_rate_;

    bool is_rws_setup_;

    YumiEGMInterface yumi_egm_interface_;

    // command buffers
    float joint_command_[N_YUMI_JOINTS];

    // data buffers
    float joint_feedback_[N_YUMI_JOINTS];
};


#endif