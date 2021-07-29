/*
 * hdrlib_tonemap_durnad.cpp
 *
 *  Created on: 7. 11. 2017
 *      Author: nosko
 */



#include <hdr/hdrlib_tonemap_durand.hpp>


hdr::tmo::TonemapDurand::TonemapDurand()
{
	this->sigS = 2.0f;
	this->sigR = 1.0f;
	this->base = 2.5f;
}

void hdr::tmo::TonemapDurand::prepare(float sigS, float sigR, float base)
{
	this->sigS = sigS;
	this->sigR = sigR;
	this->base = base;
}

void hdr::tmo::TonemapDurand::apply(cv::Mat &im)
{
	im.copyTo(this->input);
	this->output = cv::Mat(im.size(), CV_32FC3);

	this->lumaOrig = cv::Mat(this->input.size(), CV_32FC1);
	this->lumaLog = cv::Mat(this->input.size(), CV_32FC1);
	this->baseLayer = cv::Mat(this->input.size(), CV_32FC1);
	this->bmp = cv::Mat(this->input.size(), CV_8UC3);

	 cv::Vec3f min;
	 cv::Vec3f max;
	getLuminance(input, lumaOrig);
	applyLog(lumaOrig, lumaLog);
	cv::bilateralFilter(lumaLog, baseLayer, 11, 4, 4);
	toneMapping(this->lumaOrig,
				this->lumaLog,
				this->baseLayer,
				this->input,
				this->output,
				this->base);


//	applyGamma(this->output,  2.2);

	bmp = cv::Mat(this->output.size(), CV_8UC3);


	for (int32_t i = 0; i < this->lumaLog.rows; ++i)
	{
		for (int32_t j = 0; j < this->lumaLog.cols; ++j)
		{
			cv::Vec3f pix = this->output.at<cv::Vec3f>(i, j);

			bmp.at<cv::Vec3b>(i, j)[0] = pix[0] * 255 > 255 ? 255 : pix[0] * 255;
			bmp.at<cv::Vec3b>(i, j)[1] = pix[1] * 255 > 255 ? 255 : pix[1] * 255;
			bmp.at<cv::Vec3b>(i, j)[2] = pix[2] * 255 > 255 ? 255 : pix[2] * 255;
		}
	}

}

void hdr::tmo::TonemapDurand::show()
{

}

void hdr::tmo::TonemapDurand::writeImages(std::string path)
{

	cv::imwrite(path, bmp);
	//cv::imwrite(path + "durand.exr", output);
}

cv::Mat hdr::tmo::TonemapDurand::getImage()
{
	return bmp;
}



void hdr::tmo::TonemapDurand::getLuminance(cv::Mat &input, cv::Mat& output)
{
	for( int32_t i = 0; i < input.rows; i++ )
	{
		for( int32_t j = 0; j < input.cols; j++ )
		{
			/* extract intesities  */
			cv::Vec3f rgbTriplet = input.at< cv::Vec3f >( i, j );
			float rTmp = (0.2126) * (rgbTriplet.val[2]);
			float gTmp = (0.7152) * (rgbTriplet.val[1]);
			float bTmp = (0.0722) * (rgbTriplet.val[0]);

			float tmp = (rTmp + gTmp + bTmp);
			if (!std::isnormal(tmp))
			{
				tmp = 0;
			}

			if (output.type() == CV_32FC1)
			{
				output.at<float>(i, j) = tmp;
			}
			else
			{
				output.at<cv::Vec3f>(i, j)[0] = tmp;
				output.at<cv::Vec3f>(i, j)[1] = tmp;
				output.at<cv::Vec3f>(i, j)[2] = tmp;
			}
		}
	}
}


void  hdr::tmo::TonemapDurand::applyLog(cv::Mat &input, cv::Mat& output)
{
	for( int32_t i = 0; i < input.rows; i++ )
	{
		for( int32_t j = 0; j < input.cols; j++ )
		{
			float l = 0;

			if (input.type() == CV_32FC1)
			{
				l =  logf(input.at<float>(i, j));
			}
			else
			{
				l =  logf(input.at<cv::Vec3f>(i, j)[0]);
			}
			if (!std::isnormal(l))
				l = 0;

			if (output.type() == CV_32FC1)
			{
				output.at<float>(i, j) = l;
			}
			else
			{
				output.at<cv::Vec3f>(i, j)[0] = l;
				output.at<cv::Vec3f>(i, j)[1] = l;
				output.at<cv::Vec3f>(i, j)[2] = l;
			}
		}
	}
}

void hdr::tmo::TonemapDurand::findExtremes(cv::Mat &im, cv::Vec3f& min, cv::Vec3f& max)
{
	max = 0 - (1e20);
	min = 1e20;
	for (int32_t i = 0; i < im.rows; ++i)
	{
		for (int32_t j = 0; j < im.cols; ++j)
		{
			cv::Vec3f pix;

			if (im.type() == CV_32FC3)
				pix = im.at<cv::Vec3f>(i ,j);
			else
				pix [0] = pix[1] = pix[2] = im.at<float>(i ,j);


			for (int32_t k = 0; k < 3; k++)
			{
				max[k] = MAX(pix[k], max[k]);
				min[k] = MIN(pix[k], max[k]);
			}
		}
	}


	std::cout << max << " " << min << std::endl;

}

void hdr::tmo::TonemapDurand::findExtremes(cv::Mat &im, cv::Vec3f& min, cv::Vec3f& max, float percentil)
{
	float locMax;
	float locMin;

	std::vector<float> minVector;
	std::vector<float> maxVector;

	max = 0 - (1e20);
	min = 1e20;
	for (int32_t i = 0; i < im.rows; ++i)
	{
		for (int32_t j = 0; j < im.cols; ++j)
		{
			cv::Vec3f pix;

			if (im.type() == CV_32FC3)
				pix = im.at<cv::Vec3f>(i ,j);
			else
				pix [0] = pix[1] = pix[2] = im.at<float>(i ,j);

			locMax = pix[0];
			locMin = pix[0];

			for (int32_t k = 0; k < 3; k++)
			{
				locMax = MAX(pix[k], locMax);
				locMin = MIN(pix[k], locMin);
			}


			maxVector.push_back(locMax);
			minVector.push_back(locMin);

		}
	}


    std::sort(maxVector.begin(), maxVector.end());
    std::sort(minVector.begin(), minVector.end());

    max[0] = max[1] = max[2] = maxVector.at(maxVector.size() * percentil);
    min[0] = min[1] = min[2] = minVector.at(minVector.size()  - minVector.size() * percentil);


	//std::cout << max << " " << min << std::endl;

}



void  hdr::tmo::TonemapDurand::toneMapping(cv::Mat &intensitiesOrig,
					cv::Mat &intensitiesLog,
					cv::Mat &base,
					cv::Mat &colors,
					cv::Mat &dst,
					float &baseContrast)
{
	 cv::Vec3f min;
	 cv::Vec3f max;

	std::cout << "Base TMO: ";
	findExtremes(base, min, max);


	float k1 = 1.48, k2 = 0.82;
	float compressionFactor = baseContrast/(max[0]-min[0]);
	float s = ((1 + k1) * powf(compressionFactor, k2)) / ( 1 + k1 * powf(compressionFactor, k2));




	for (int32_t i = 0; i < base.rows; i++)
	{
		for (int32_t j = 0; j < base.cols; j++)
		{
			float b = 0.0f, I = 0.0f, logI = 0.0f, detail = 0.0f, bNew = 0.0f, INew = 0.0f;
			cv::Vec3f pix;

			pix = colors.at<cv::Vec3f>(i, j);;
			I = intensitiesOrig.at<float>(i, j);
			logI = intensitiesLog.at<float>(i, j);
			b = base.at<float>(i, j);
			for (int32_t k = 0; k < 3; k++)
				pix[k] = (float(pix[k]) / I);
			detail = logI - b;
			bNew = float(b) * compressionFactor +  1.0 * detail;
			INew = bNew - (float(4.3) + float(min[0]) * compressionFactor);

			INew = expf(INew);


			for (int32_t k = 0; k < 3; k++)
			{
//				pix[k] = (pix[k]) * s + 1;
				pix[k] = powf(pix[k], s) * INew;
			}
			dst.at<cv::Vec3f>(i, j) = pix;
		}
	}


	findExtremes(dst,min, max, 0.999);

	for (int32_t i = 0; i < base.rows; i++)
	{
		for (int32_t j = 0; j < base.cols; j++)
		{
			cv::Vec3f pix;
			pix = dst.at<cv::Vec3f>(i, j);

			for (int32_t k = 0; k < 3; k++)
			{
				pix[k] = (pix[k] - min[k])  / (max[k] - min[k]);
			}

			dst.at<cv::Vec3f>(i, j) = pix;

		}
	}
}

	//std::cout << min << std::endl;
	//std::cout << max << std::endl;


void  hdr::tmo::TonemapDurand::applyGamma(cv::Mat &input, float gamma)
{
	for( int32_t i = 0; i < input.rows; i++ )
	{
		for( int32_t j = 0; j < input.cols; j++ )
		{
			/* extract intesities  */
			cv::Vec3f rgbTriplet = input.at< cv::Vec3f >( i, j );
			input.at<cv::Vec3f>(i, j)[0] = powf(rgbTriplet[0], (1/gamma));
			input.at<cv::Vec3f>(i, j)[1] = powf(rgbTriplet[1], (1/gamma));
			input.at<cv::Vec3f>(i, j)[2] = powf(rgbTriplet[2], (1/gamma));
		}
	}
}


