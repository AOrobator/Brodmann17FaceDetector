/*
 * tinybd17example.cpp
 *
 *  Created on: May 14, 2018
 *      Author: netanell
 */

#include "libbrodmann17.h"
#include <string>
#include <unistd.h>
#include <opencv2/opencv.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <chrono>
#include <stdlib.h>
#include <stdio.h>
#include <dirent.h>

static const int MAX_DETECTIONS = 500;

using namespace bd17;
using namespace std;
using namespace cv;

int imageExample(string image_filename) {

	// Prepare Detector

	Init();
	void* detector = CreateDetector();

	unsigned int detections_num = 0;
	float* detections = new float [PARAMS_PER_DETECTION*MAX_DETECTIONS];

	std::vector<std::string> files_list;
	//Check if it is a directory

	std::string dir_path = image_filename;
	if (dir_path.at(dir_path.length()-1) != '/')
		dir_path += "/";

	DIR *dir;
	struct dirent *ent;
	if ((dir = opendir (dir_path.c_str())) != NULL) {
		/* take all the files and directories within directory */
		while ((ent = readdir (dir)) != NULL) {
			std::string p(ent->d_name);
			if (p.compare(".") && p.compare(".."))
			{
				files_list.push_back(dir_path + p);
				printf ("%s\n", ent->d_name);
			}
		}
		closedir (dir);
	} else {
		//If not run on given file
		files_list.push_back(image_filename);
	}


	for (auto& imfile: files_list) {
		Mat im = cv::imread(imfile, cv::IMREAD_COLOR);

		if (!im.size().area()) return EXIT_FAILURE;

		auto start_1 = std::chrono::high_resolution_clock::now();
		// Run detection
		if (!Detect(detector, detections, &detections_num, MAX_DETECTIONS,
				(void*)im.data, im.cols, im.rows, bd17_image_format_t::bd17_bgr_interleaved_byte,
				NULL, NULL))
		{
			fprintf(stderr, "Error: Detection error");
			DestroyDetector(detector);
			delete [] detections;
			return EXIT_FAILURE;
		}
		auto end_1 = std::chrono::high_resolution_clock::now();
		std::chrono::duration<double> det_time = end_1-start_1;
		printf("image %s time: %f\r\n", imfile.c_str(), det_time);
		// Draw results
		for (unsigned int i = 0; i < detections_num; i++)
		{
			// Upper left corner
			Point pt1(detections[i*PARAMS_PER_DETECTION], detections[i*PARAMS_PER_DETECTION+1]);

			// Bottom right corner
			Point pt2(detections[i*PARAMS_PER_DETECTION] + detections[i*PARAMS_PER_DETECTION+2] - 1, detections[i*PARAMS_PER_DETECTION+1] + detections[i*PARAMS_PER_DETECTION+3] - 1);

			// Draw rectangle
			rectangle(im, pt1, pt2, Scalar(0, 0, 255), 2);

			printf(" %f,%f,%f,%f \r\n", detections[i*PARAMS_PER_DETECTION],
					detections[i*PARAMS_PER_DETECTION+1],
					detections[i*PARAMS_PER_DETECTION] + detections[i*PARAMS_PER_DETECTION+2] - 1,
					detections[i*PARAMS_PER_DETECTION+1] + detections[i*PARAMS_PER_DETECTION+3] - 1);
		}
		//If you dont want to show remove the next line
		//imshow("Output", im);

		//Save the image with .out.png suffix
		imwrite(imfile+".out.png", im);

		//int key = waitKey(0);
	}
	// Clean Up
	DestroyDetector(detector);
	delete [] detections;

	return EXIT_SUCCESS;
}

static inline bool is_file_exists (const std::string& name) {
    return ( access( name.c_str(), F_OK ) != -1 );
}

int main(int argc, char ** argv) {
	std::string image = "./example.jpg";
	if (argc != 2) {
		fprintf(stderr, "Input file is not given, uses example.jpg\r\n");
	} else {
		image = std::string(argv[1]);
	}
	if (!is_file_exists(image)) {
		fprintf(stderr, "File %s does not exists\r\n", image.c_str());
		return -1;
	}
	return imageExample(image);
}


