#include <vector>
#include <string>
#include <iostream>
#include <sstream>
#include <pmdsdk2.h>

// on error, prepend absolute path to files before plugin names
#define SOURCE_PLUGIN "O3D3xxCamera"

// Source Params string format:
// Ip Address : port Number : XmlRPC port : Application ID (Optional)
// If no application ID is provided in source params, a new application 
// is created on connect, and deleted at disconnect.
//#define CAMERA_IP_1 "192.168.0.69"
#define CAMERA_IP_1 "172.29.23.21"

#define PCIC_PORT_NUMBER "80"
#define XMLRPC_PORT_NUMBER "50010"
#define APPLICATION_ID "1"
#define SOURCE_PARAM CAMERA_IP_1 ":" PCIC_PORT_NUMBER ":" XMLRPC_PORT_NUMBER 
#define SOURCE_PARAM_APP_ID CAMERA_IP_1 ":" PCIC_PORT_NUMBER ":" XMLRPC_PORT_NUMBER ":" APPLICATION_ID 

#define PROC_PLUGIN "O3D3xxProc"
#define PROC_PARAM ""

// Define length of buffer for response from source command
#define SOURCE_CMD_BUFFER_LENGTH 256
// Define length for Clipping Cuboid String
#define CLIPPING_CUBOID_STRING_LENGTH 1024

int main(int argc, char **argv)
{
	PMDHandle hnd = 0; // connection handle
	int res = 0;
	PMDDataDescription dd;
	unsigned int integrationTime = 0;
	std::vector<float> amp;
	std::vector<float> dist;
	std::vector<unsigned> flags;
	std::vector<float> xyz3Dcoordinate;
	int imgHeight = 0;
	int imgWidth = 0;
	char err[256] = { 0 };
	std::string command = "";
	std::string source_params;
	std::string ip_add;
	std::string xmlprc_port = "80";
	std::string pcic_port = "50010";

	std::cout << "Enter ip address of camera: " << std::endl;
	getline(std::cin, ip_add);
	
	source_params = ip_add + ":" + xmlprc_port + ":" + pcic_port;

	printf("\n =======================O3D300 Sample Basic Code=========================");
	printf("\n Connecting to camera: \n");
	// connect to camera
	res = pmdOpen(&hnd, SOURCE_PLUGIN, source_params.data(), PROC_PLUGIN, PROC_PARAM);

	if (res != PMD_OK)
	{
		fprintf(stderr, "Could not connect: \n");
		getchar();
		return res;
	}
	printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");
	printf("\n Updating the framedata from camera ");
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

	imgWidth = dd.img.numColumns;
	imgHeight = dd.img.numRows;

	printf("\n -----------------------------------------------------------------------------");


	dist.resize(imgWidth * imgHeight);
	amp.resize(imgWidth * imgHeight);
	flags.resize(imgWidth * imgHeight);
	xyz3Dcoordinate.resize(imgHeight * imgWidth * 3); // value three is for 3 images

	printf("\n Obtaining different image data from camera viz amplitude and Distance Image  \n");

	res = pmdGetDistances(hnd, &dist[0], dist.size() * sizeof(float));
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not get distance data: \n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}
	printf("\n Middle pixel Distance value: %fm\n", dist[(imgHeight / 2) * imgWidth + imgWidth / 2]);

	res = pmdGetAmplitudes(hnd, &amp[0], amp.size() * sizeof(float));
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not get amplitude data: \n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}
	printf("\n Middle Amplitude: %f\n", amp[(dd.img.numRows / 2) * dd.img.numColumns + dd.img.numColumns / 2]);

	res = pmdGetFlags(hnd, &flags[0], flags.size() * sizeof(float));
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not get flag data: \n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}

	/* 3D coordinates are stored in interleved way .i.e.
	for every pixel i
	xcordinate value = xyz3Dcoordinate[i+0]
	ycordinate value = xyz3Dcoordinate[i+1]
	zcordinate value = xyz3Dcoordinate[i+2]*/

	pmdGet3DCoordinates(hnd, &xyz3Dcoordinate[0], xyz3Dcoordinate.size() * sizeof(float));
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not get xyz data: \n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}

	//calculating the mean values of X/Y/Z/ Image
	float XSum = 0, YSum = 0, ZSum = 0;
	float ASum = 0, DSum = 0;
	unsigned counter = 0;
	for (int i = 0; i < imgHeight * imgWidth; i++)
	{
		if (!(flags[i] & 1)) // first bit is set to 1,if pixel is invalid
		{
			DSum += dist[i];
			ASum += amp[i];
			XSum += xyz3Dcoordinate[i * 3 + 0];
			YSum += xyz3Dcoordinate[i * 3 + 1];
			ZSum += xyz3Dcoordinate[i * 3 + 2];
			counter++;
		}
	}

	float DMean = counter ? (DSum / float(counter)) : 0.f;
	float AMean = counter ? (ASum / float(counter)) : 0.f;
	float XMean = counter ? (XSum / float(counter)) : 0.f;
	float YMean = counter ? (YSum / float(counter)) : 0.f;
	float ZMean = counter ? (ZSum / float(counter)) : 0.f;

	printf(" DMean = %fm \n AMean = %f \n", DMean, AMean);
	printf(" XMean = %fm \n YMean = %fm \n ZMean = %fm ", XMean, YMean, ZMean);

	printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");
	printf("\n Setting and getting parameters viz : integrationTime \n");

	res = pmdSetIntegrationTime(hnd, 0, 1000);
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not set the integration time: \n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}

	res = pmdGetIntegrationTime(hnd, &integrationTime, 0);
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not set the integration time:\n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}
	printf("Integration Time = %d \n", integrationTime);
	printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");
	printf("\n Closing the connection \n");

	res = pmdClose(hnd);
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not close the connection %s\n", err);

		return res;
	}

	printf("\n Camera closed Successfully");

	printf("\n -----------------------------------------------------------------------------");
	printf("\n---------------Connecting to camera 1 at a specific application ID------------");
	printf("\n -----------------------------------------------------------------------------");

	printf("\n Connecting to camera 1 at application ID 1 : \n");
	// connect to camera at a specific application ID
	// If no application ID is provided in the source param, then, SDK creates a new
	// application. If application ID is provided, SDK will connect at the given ID.
	// If no application is found at given ID, the camera generates xmlrpc error 
	// resulting in pmdOpen function to fail.
	// For this sample code to work, camera should contain an application at index 1.
	res = pmdOpen(&hnd, SOURCE_PLUGIN, SOURCE_PARAM_APP_ID, PROC_PLUGIN, PROC_PARAM);
	if (res != PMD_OK)
	{
		fprintf(stderr, "Could not connect: \n");
		getchar();
		return res;
	}
	printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");
	printf("\n -----------------------------------------------------------------------------");
	printf("\n Updating the framedata from camera ");
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
	res = pmdGetIntegrationTime(hnd, &integrationTime, 0);
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not set the integration time:\n%s\n", err);
		pmdClose(hnd);
		printf("Camera Connection Closed. \n");
		getchar();
		return res;
	}
	printf("Integration Time = %d \n", integrationTime);
	printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");
	printf("\n Closing the connection \n");

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
