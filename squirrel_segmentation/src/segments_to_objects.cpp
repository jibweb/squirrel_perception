/**
 * segments_to_objects.cpp
 * 
 * Takes a list of segmented clusters and returns "proto" objects: 3D poses and 3D points.
 *
 * @author: Michael Zillich
 * @date 2015-03-04
 */

#include <pcl/point_types.h>
#include <pcl/common/centroid.h>
#include <ros/ros.h>
#include <pcl_conversions/pcl_conversions.h>
#include <std_msgs/String.h>
#include <sensor_msgs/PointCloud2.h>
#include <geometry_msgs/PoseStamped.h>
#include <squirrel_object_perception_msgs/SegmentsToObjects.h>

class SegmentsToObjectsImpl
{
private:
  typedef pcl::PointXYZRGB PointT;

  ros::ServiceServer service_;
  ros::NodeHandle *nh_;
  bool segmentsToObjects(squirrel_object_perception_msgs::SegmentsToObjects::Request & req, squirrel_object_perception_msgs::SegmentsToObjects::Response & response);

public:
  SegmentsToObjectsImpl();
  ~SegmentsToObjectsImpl();
  void init(int argc, char ** argv);
};

SegmentsToObjectsImpl::SegmentsToObjectsImpl()
{
  nh_ = 0;
}

SegmentsToObjectsImpl::~SegmentsToObjectsImpl()
{
  delete nh_;
}

bool SegmentsToObjectsImpl::segmentsToObjects(squirrel_object_perception_msgs::SegmentsToObjects::Request &req,
                                              squirrel_object_perception_msgs::SegmentsToObjects::Response &response)
{
  ROS_INFO ("SegmentsToObjectsImpl::segmentsToObjects called\n");
  pcl::PointCloud<PointT>::Ptr scene(new pcl::PointCloud<PointT>);
  pcl::fromROSMsg(req.cloud, *scene);
  response.points.resize(req.clusters_indices.size());
  response.poses.resize(req.clusters_indices.size());
  for(size_t i = 0; i < req.clusters_indices.size(); i++)
  {
    pcl::PointCloud<PointT>::Ptr object(new pcl::PointCloud<PointT>);
    for(size_t j = 0; j < req.clusters_indices[i].data.size(); j++)
    {
      object->points.push_back(scene->points[req.clusters_indices[i].data[j]]);
    }
    pcl::toROSMsg(*object, response.points[i]);

    Eigen::Vector4d centroid;
    pcl::compute3DCentroid(*scene, req.clusters_indices[i].data, centroid);
    response.poses[i].header.stamp = req.cloud.header.stamp;
    response.poses[i].header.frame_id = req.cloud.header.frame_id;
    response.poses[i].pose.position.x = centroid[0];
    response.poses[i].pose.position.y = centroid[1];
    response.poses[i].pose.position.z = centroid[2];
    // note: the orientation is quite arbitrary
    response.poses[i].pose.orientation.x = 0.;
    response.poses[i].pose.orientation.y = 0.;
    response.poses[i].pose.orientation.z = 0.;
    response.poses[i].pose.orientation.w = 1.;
  }
  
  return true;
}

void SegmentsToObjectsImpl::init(int argc, char ** argv)
{
  ros::init (argc, argv, "segments_to_objects");
  nh_ =  new ros::NodeHandle ("~");
  service_ = nh_->advertiseService ("/squirrel_segments_to_objects", &SegmentsToObjectsImpl::segmentsToObjects, this);
  ROS_INFO ("Ready to receive service calls...");
}

int main (int argc, char ** argv)
{
  SegmentsToObjectsImpl s;
  s.init(argc, argv);
  ros::spin();
  return 0;
}
