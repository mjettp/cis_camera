#pragma once

#include <libuvc/libuvc.h>

#include <ros/ros.h>
#include <image_transport/image_transport.h>
#include <image_transport/camera_publisher.h>
#include <dynamic_reconfigure/server.h>
#include <camera_info_manager/camera_info_manager.h>
#include <boost/thread/mutex.hpp>

#include <cis_camera/CISCameraConfig.h>


namespace cis_camera
{

class CameraDriver
{
public:
  
  CameraDriver( ros::NodeHandle nh, ros::NodeHandle priv_nh );
  ~CameraDriver();
  
  bool Start();
  void Stop();
  
  int  setToFMode_All();
  void getToFInfo_All();
  
  
private:
  
  enum State
  {
    Initial = 0,
    Stopped = 1,
    Running = 2,
  };
  
  // Flags controlling whether the sensor needs to be stopped (or reopened) when changing settings
  static const int ReconfigureClose   = 3; // Need to close and reopen sensor to change this setting
  static const int ReconfigureStop    = 1; // Need to stop the stream before changing this setting
  static const int ReconfigureRunning = 0; // We can change this setting without stopping the stream
  
  void readConfigFromParameterServer();
  void advertiseROSTopics();
  void OpenCamera();
  void CloseCamera();
  
  // Accept a reconfigure request from a client
  void ReconfigureCallback( CISCameraConfig &config, uint32_t level );
  enum uvc_frame_format GetVideoMode( std::string vmode );
  
  // Accept a new image frame from the camera
  void ImageCallback( uvc_frame_t *frame );
  static void ImageCallbackAdapter( uvc_frame_t *frame, void *ptr );
  
  
  // TOF Camera
  enum uvc_extention_unit_id
  {
    UVC_EXT_UNIT_TOF = 3,
    UVC_EXT_UNIT_RGB = 9
  };
  
  enum tof_process_num
  {
    TOF_SET_EEPROM          = 0x0000,
    TOF_SET_DEPTH_IR        = 0x0001,
    TOF_SET_DEPTH_RANGE     = 0x0002,
    TOF_SET_THRESHOLD       = 0x0003,
    TOF_SET_NR_FILTER       = 0x0004,
    TOF_SET_PULSE_COUNT     = 0x0005,
    TOF_SET_LD_ENABLE       = 0x0006,
    TOF_SET_IR_GAIN         = 0x0009,
    TOF_SET_ERROR_STOP      = 0x0010,
    TOF_SET_ERROR_CLEAR     = 0x7F01,
    TOF_GET_DEPTH_IR        = 0x8001,
    TOF_GET_DEPTH_RANGE     = 0x8002,
    TOF_GET_THRESHOLD       = 0x8003,
    TOF_GET_NR_FILTER       = 0x8004,
    TOF_GET_PULSE_COUNT     = 0x8005,
    TOF_GET_LD_ENABLE       = 0x8006,
    TOF_GET_DEPTH_CNV_GAIN  = 0x8007,
    TOF_GET_DEPTH_INFO      = 0x8008,
    TOF_GET_IR_GAIN         = 0x8009,
    TOF_GET_TEMPERATURE     = 0x800A,
    TOF_GET_ERROR_STOP      = 0x8010,
    TOF_GET_VERSION         = 0xFF00,
    TOF_GET_ERROR_INFO      = 0xFF01,
  };
  
  enum tof_eeprom_mode
  {
    TOF_EEPROM_FACTORY_DEFAULT  = 0x0000,
    TOF_EEPROM_UPDATE_CURRENT   = 0x0001,
  };
  
  int setToFCtrl( uint16_t *data ,int len );
  int getToFCtrl( uint16_t *data ,int len );
  
  int setToFMode_ROSParameter( std::string param_name, int param );
  int setToFEEPROMMode( uint16_t mode );
  int clearToFError();
  
  int getToFDepthIR( uint16_t& depth_ir );
  int getToFDepthRange( uint16_t& depth_range, uint16_t& dr_index );
  int getToFThreshold( uint16_t& threshold );
  int getToFNRFilter( uint16_t& nr_filter );
  int getToFPulseCount( uint16_t& pulse_count );
  int getToFLDEnable( uint16_t& ld_enable );
  int getToFDepthCnvGain( double& depth_cnv_gain );
  int getToFDepthInfo( short&          offset_val,
                       unsigned short& max_data,
                       unsigned short& min_dist,
                       unsigned short& max_dist );
  int getToFIRGain( uint16_t& ir_gain );
  int getToFTemperature( double& t1, double& t2 );
  int getToFErrorStop( uint16_t& error_stop );
  int getToFVersion( uint16_t& version_n,
                     uint16_t& build_n,
                     uint16_t& build_y,
                     uint16_t& build_d );
  int getToFErrorInfo( uint16_t& common_err,
                       uint16_t& eeprom_err_factory,
                       uint16_t& eeprom_err,
                       uint16_t& mipi_temp_err );
  
  void publishToFTemperature( std::string frame_id );
  
  ros::Publisher pub_tof_t1_;
  ros::Publisher pub_tof_t2_;
  
  // END TOF Camera
  
  ros::NodeHandle nh_, priv_nh_;
  
  State                  state_;
  boost::recursive_mutex mutex_;
  
  uvc_context_t       *ctx_;
  uvc_device_t        *dev_;
  uvc_device_handle_t *devh_;
  uvc_frame_t         *rgb_frame_;
  
  image_transport::ImageTransport  it_;
  image_transport::CameraPublisher pub_camera_;
  image_transport::CameraPublisher pub_bgr8_;
  image_transport::CameraPublisher pub_color_;
  image_transport::CameraPublisher pub_depth_;
  image_transport::CameraPublisher pub_ir_;
  
  dynamic_reconfigure::Server<CISCameraConfig> config_server_;
  
  CISCameraConfig config_;
  bool            config_changed_;
  
  camera_info_manager::CameraInfoManager cinfo_manager_;
  camera_info_manager::CameraInfoManager cinfo_manager_color_;
  camera_info_manager::CameraInfoManager cinfo_manager_ir_;
  
  std::string camera_info_url_;
  std::string camera_info_url_ir_;
  std::string camera_info_url_color_;
  
};

};