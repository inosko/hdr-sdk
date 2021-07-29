/*
 * hdrlib_tonemap_intf.hpp
 *
 *  Created on: 6. 11. 2017
 *      Author: nosko
 */

#pragma once


#include <iostream>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

namespace hdr
{

namespace tmo
{


class TonemapInterface
{


public:
	TonemapInterface(){};

    ~TonemapInterface(){};

    virtual void apply(cv::Mat &im) = 0;

	virtual void show() = 0;

	virtual void writeImages(std::string path) = 0;

	virtual cv::Mat getImage() = 0;
};

}

}
