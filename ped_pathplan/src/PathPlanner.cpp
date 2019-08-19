#include <ros/ros.h>
#include <tf/transform_listener.h>
#include <costmap_2d/costmap_2d_ros.h>
#include <costmap_2d/costmap_2d.h>
#include <ped_pathplan/pathplan_ros.h>


int main(int argc, char** argv) {
    ros::init(argc, argv, "path_planner_node");

    tf::TransformListener tf(ros::Duration(10));
    //costmap_2d::Costmap2DROS costmap("ped_costmap", *tf.getTF2BufferPtr());
    costmap_2d::Costmap2DROS costmap("ped_costmap", tf);

    //navfn::NavfnROS p;
    ped_pathplan::PathPlanROS p;
    p.initialize("planner", &costmap);

	ros::MultiThreadedSpinner spinner(3); // Use 4 threads
	spinner.spin();
    //ros::spin();
    return 0;
}

