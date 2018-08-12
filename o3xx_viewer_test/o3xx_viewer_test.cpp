#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <pmdsdk2.h>
#include <windows.h>

// on error, prepend absolute path to files before plugin names
#define SOURCE_PLUGIN "O3D3xxCamera"

#define PROC_PLUGIN "O3D3xxProc"
#define PROC_PARAM ""

// Define length of buffer for response from source command
#define SOURCE_CMD_BUFFER_LENGTH 256

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
	std::string CAMERA_IP_1 = "172.29.23.21";
	std::string PCIC_PORT_NUMBER = "80";
	std::string XMLRPC_PORT_NUMBER = "50010";

	LARGE_INTEGER frequency;        // ticks per second
	LARGE_INTEGER t1, t2;           // ticks
	double sumFps1 = 0, sumFps2 = 0, fps = 0;

	for (int i = 1; i < argc; i++)
	{
		std::string argument = argv[i];
		std::string command = argument.substr(0, argument.find('='));
		std::string cmd_value = argument.substr(argument.find('=') + 1, argument.length());
		if (command == "--IP")
			CAMERA_IP_1 = cmd_value;
		else if (command == "--COUNT")
			loop_count_by_100 = atoi(cmd_value.c_str()) / 100;
	}

	std::string SOURCE_PARAM_1 = CAMERA_IP_1 + ":" + PCIC_PORT_NUMBER + ":" + XMLRPC_PORT_NUMBER;

	// get ticks per second
	QueryPerformanceFrequency(&frequency);

	printf("\n ===================================================================");
	printf("\n Connecting to camera 1: \n");
	// connect to camera
	res = pmdOpen(&hnd, SOURCE_PLUGIN, SOURCE_PARAM_1.c_str(), PROC_PLUGIN, PROC_PARAM);

	if (res != PMD_OK)
	{
		fprintf(stderr, "Could not connect: \n");
		getchar();
		return res;
	}
	printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");
	printf("\n Updating the framedata from camera");
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

	dist.resize(dd.img.numColumns * dd.img.numRows);
	ampl.resize(dd.img.numColumns * dd.img.numRows);
	xyz3Dcoordinate.resize(dd.img.numColumns * dd.img.numRows * 3);

	printf("\n Starting Loop for 23K resolution \n");

	// start timer
	QueryPerformanceCounter(&t1);

	for (int i = 0; i < loop_count_by_100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			res = pmdUpdate(hnd);
			if (res != PMD_OK)
			{
				printf("\n Error while updating camera \n");
				pmdClose(hnd);
				return PMD_RUNTIME_ERROR;
			}

			res = pmdGetDistances(hnd, &dist[0], dist.size() * sizeof(float));

			if (res != PMD_OK)
			{
				printf("\n Error while getting distances \n");
				pmdClose(hnd);
				return PMD_RUNTIME_ERROR;
			}

			res = pmdGetAmplitudes(hnd, &ampl[0], ampl.size() * sizeof(float));

			if (res != PMD_OK)
			{
				printf("\n Error while getting amplitudes \n");
				pmdClose(hnd);
				return PMD_RUNTIME_ERROR;
			}

			res = pmdGet3DCoordinates(hnd, &xyz3Dcoordinate[0], xyz3Dcoordinate.size() * sizeof(float));

			if (res != PMD_OK)
			{
				printf("\n Error while getting 3D coordinates \n");
				pmdClose(hnd);
				return PMD_RUNTIME_ERROR;
			}

		}

		// stop timer
		QueryPerformanceCounter(&t2);

		// compute the fps
		fps = ((100.0f * (i + 1) * frequency.QuadPart) / (t2.QuadPart - t1.QuadPart));
		sumFps1 += fps;
		printf("Frame Rate = %f fps\n", fps);

	}
	printf("\n Finished running for 23K resolution");
	printf("\n -----------------------------------------------------------------------------");

	printf("\n Changing Image Resolution to 352 X 264 \n");
	command = "SetImageResolution 1";
	res = pmdSourceCommand(hnd, response, SOURCE_CMD_BUFFER_LENGTH, command.c_str());
	if (res != PMD_OK)
	{
		pmdGetLastError(hnd, err, 128);
		fprintf(stderr, "Could not enable distance image filtering:\n%s\n", err);
		printf("Press any key to continue....");
		getchar();
	}
	else
		printf("\n DONE");

	printf("\n -----------------------------------------------------------------------------");

	printf("\n Updating the framedata from camera");
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

	dist.clear();
	ampl.clear();
	xyz3Dcoordinate.clear();
	dist.resize(dd.img.numColumns * dd.img.numRows);
	ampl.resize(dd.img.numColumns * dd.img.numRows);
	xyz3Dcoordinate.resize(dd.img.numColumns * dd.img.numRows * 3);

	printf("\n Resolution = %d X %d \n", dd.img.numColumns, dd.img.numRows);
	printf("\n Starting Loop for 100K resolution \n");
	// start timer
	QueryPerformanceCounter(&t1);
	for (int i = 0; i < loop_count_by_100; i++)
	{
		for (int j = 0; j < 100; j++)
		{
			res = pmdUpdate(hnd);
			if (res != PMD_OK)
			{
				printf("\n Error while updating camera \n");
				pmdClose(hnd);
				return PMD_RUNTIME_ERROR;
			}

			res = pmdGetDistances(hnd, &dist[0], dist.size() * sizeof(float));

			if (res != PMD_OK)
			{
				printf("\n Error while getting distances \n");
				pmdClose(hnd);
				return PMD_RUNTIME_ERROR;
			}

			res = pmdGetAmplitudes(hnd, &ampl[0], ampl.size() * sizeof(float));

			if (res != PMD_OK)
			{
				printf("\n Error while getting amplitudes \n");
				pmdClose(hnd);
				return PMD_RUNTIME_ERROR;
			}

			res = pmdGet3DCoordinates(hnd, &xyz3Dcoordinate[0], xyz3Dcoordinate.size() * sizeof(float));

			if (res != PMD_OK)
			{
				printf("\n Error while getting 3D coordinates \n");
				pmdClose(hnd);
				return PMD_RUNTIME_ERROR;
			}

		}
		// stop timer
		QueryPerformanceCounter(&t2);

		// compute the fps
		fps = ((100.0f * (i + 1) * frequency.QuadPart) / (t2.QuadPart - t1.QuadPart));
		sumFps2 += fps;
		printf("Frame Rate = %f fps\n", fps);

	}
	printf("\n Finished running for 100K resolution");
	printf("\n 23K resolution => Mean FPS = %f", (sumFps1 / loop_count_by_100));
	printf("\n 100K resolution => Mean FPS = %f", (sumFps2 / loop_count_by_100));
	printf("\n -----------------------------------------------------------------------------");

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