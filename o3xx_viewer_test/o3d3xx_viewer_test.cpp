#include <pcl/visualization/cloud_viewer.h>
#include <pcl/common/common_headers.h>
#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <pmdsdk2.h>
#include <windows.h>
//#include <pcl/visualization/cloud_viewer.h>
#include <pcl/point_types.h>

// on error, prepend absolute path to files before plugin names
#define SOURCE_PLUGIN "O3D3xxCamera"

#define PROC_PLUGIN "O3D3xxProc"
#define PROC_PARAM ""

// Define length of buffer for response from source command
#define SOURCE_CMD_BUFFER_LENGTH 256
//void updateCloud()
/*
Command Line arguments :
--IP=IP of camera
--COUNT=Count to run the data grabbing loop, default value when not specified = 10000

*/
int main(int argc, char **argv)
{
	PMDHandle hnd = 0; // connection handle
	PMDDataDescription dd;
	std::string command = "";
	int res = 0, loop_count_by_100 = 100;
	char response[256] = { 0 }, err[256] = { 0 };
	std::vector<float> ampl;
	std::vector<float> dist;
	std::vector<float> xyz3Dcoordinate;
	std::string diagnosticData = "";
	//std::string CAMERA_IP_1 = "192.168.0.69";
	std::string CAMERA_IP = "172.29.23.21";
	std::string PCIC_PORT_NUMBER = "80";
	std::string XMLRPC_PORT_NUMBER = "50010";

	int imgHeight = 0;
	int imgWidth = 0;
	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double sumFps1 = 0, sumFps2 = 0, fps = 0;

	std::cout << "Enter your camera IP address:" << std::endl;
	getline(std::cin, CAMERA_IP);
	std::string SOURCE_PARAM = CAMERA_IP + ":" + PCIC_PORT_NUMBER + ":" + XMLRPC_PORT_NUMBER;

	// get ticks per second
	QueryPerformanceFrequency(&frequency);

	
	printf("\n ===================================================================");
	printf("\n Connecting to camera \n");
	// connect to camera
	res = pmdOpen(&hnd, SOURCE_PLUGIN, SOURCE_PARAM.c_str(), PROC_PLUGIN, PROC_PARAM);

	if (res != PMD_OK)
	{
		fprintf(stderr, "Could not connect: \n");
		getchar();
		return res;
	}
	printf("\n DONE");

	res = pmdUpdate(hnd); // to update the camera parameter and framedata
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 256);
		fprintf(stderr, "Could not updateData: \n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}
	printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");
	printf("\n Retrieving source data description\n");
	res = pmdGetSourceDataDescription(hnd, &dd);
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not get data description: \n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}
	printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");

	xyz3Dcoordinate.resize(dd.img.numColumns * dd.img.numRows * 3);

	//printf("\n Starting Loop for 23K resolution \n");

	// start timer
	//QueryPerformanceCounter(&t1);
	imgWidth = dd.img.numColumns;
	imgHeight = dd.img.numRows;

	pcl::PointCloud<pcl::PointXYZ>::Ptr cloud(new pcl::PointCloud<pcl::PointXYZ>);
	boost::shared_ptr<pcl::visualization::PCLVisualizer> viewer(new pcl::visualization::PCLVisualizer("3D Viewer"));
	cloud->width = imgHeight * imgWidth;
	cloud->height = 1;
	viewer->setSize(800, 600);
	cloud->is_dense = false;
	cloud->points.resize(cloud->width * cloud->height);

	bool is_first = true;
	int v_pcl(0);
	viewer->createViewPort(0., 0., 1., 1., v_pcl);
	viewer->setBackgroundColor(0, 0, 0, v_pcl);

	while (!viewer->wasStopped())
	{
		viewer->spinOnce(100);
		res = pmdUpdate(hnd);
		if (res != PMD_OK)
		{
			continue;
		}

		//------------
		// Point cloud
		//-----------

		if (is_first)
		{
			is_first = false;
			viewer->addPointCloud<pcl::PointXYZ>(cloud, "cloud");
		}
		else
		{
			viewer->updatePointCloud(cloud, "cloud");
		}

		// update cloud data
		res = pmdGet3DCoordinates(hnd, &xyz3Dcoordinate[0], xyz3Dcoordinate.size() * sizeof(float));
		for (size_t i = 0; i < imgHeight * imgWidth; i++)
		{
			cloud->points[i].x = xyz3Dcoordinate[i * 3 + 0];
			cloud->points[i].y = xyz3Dcoordinate[i * 3 + 1];
			cloud->points[i].z = xyz3Dcoordinate[i * 3 + 2];
		}
	} // end: while (...)

	
	printf("\n Closing connection 1 \n");
	res = pmdClose(hnd);
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not close the connection %s\n", err);
		return res;
	}

	printf("\n Camera closed Successfully");

	printf("\n ================================================================================");

	return PMD_OK;
}