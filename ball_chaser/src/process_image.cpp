#include "ros/ros.h"
#include "ball_chaser/DriveToTarget.h"
#include <sensor_msgs/Image.h>
#include<iostream> 
#include<algorithm> 

// Define a global client that can request services
ros::ServiceClient client;

// This function calls the command_robot service to drive the robot in the specified direction
void drive_robot(float lin_x, float ang_z)
{
    // TODO: Request a service and pass the velocities to it to drive the robot
    ball_chaser::DriveToTarget srv;
    srv.request.linear_x = lin_x;
    srv.request.angular_z = ang_z;

    client.call(srv);

    if (!client.call(srv))
        ROS_ERROR("Failed to call service safe_move");
}

// This callback function continuously executes and reads the image data
void process_image_callback(const sensor_msgs::Image img)
{
    
    // TODO: Loop through each pixel in the image and check if there's a bright white one
    // Then, identify if this pixel falls in the left, mid, or right side of the image
    // Depending on the white ball position, call the drive_bot function and pass velocities to it
    // Request a stop when there's no white ball seen by the camera
    int s = img.data.size();
    int num_white = 0, x_sum = 0;

    for (int i = 0; i < s-2; i+=3){
        int red = img.data[i];
        int green = img.data[i+1];
        int blue = img.data[i+2];

        if (red == 255 && green == 255 && blue == 255){
            int x_pos = (i % (img.width * 3)) / 3;
            x_sum += x_pos;
            num_white += 1;
        }
    }

    if (num_white == 0){
        //stay if saw nothing
        ROS_INFO_STREAM("no ball");
        drive_robot(0, 0);
    }
    else {
        int x_average = x_sum / num_white;
        if (x_average < img.width / 3)
            drive_robot(0.5, 0.5); //Turn left
        else if ((x_average >= img.width / 3) && (x_average < img.width * 2 / 3))
            drive_robot(0.5, 0); //Straight
        else 
            drive_robot(0.5, -0.5);  //Turn right


    }
}

int main(int argc, char** argv)
{
    // Initialize the process_image node and create a handle to it
    ros::init(argc, argv, "process_image");
    ros::NodeHandle n;

    // Define a client service capable of requesting services from command_robot
    client = n.serviceClient<ball_chaser::DriveToTarget>("/ball_chaser/command_robot");

    // Subscribe to /camera/rgb/image_raw topic to read the image data inside the process_image_callback function
    ros::Subscriber sub1 = n.subscribe("/camera/rgb/image_raw", 10, process_image_callback);

    // Handle ROS communication events
    ros::spin();

    return 0;
}