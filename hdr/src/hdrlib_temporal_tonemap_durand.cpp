/*
 * hdrlib_tonemap_durnad.cpp
 *
 *  Created on: 7. 11. 2020
 *      Author: nosko
 */


#include <numeric>
#include <hdr/hdrlib_temporal_tonemap_durand.hpp>
#include <fstream>


hdr::tmo::TemporalTonemapDurand::TemporalTonemapDurand(double d, double baseContrast)
{
	this->sigS = 2.0f;
	this->sigR = 2.0f;
	this->base = baseContrast;

	temporalAdaptation = d;
	processed = 0;



}

void hdr::tmo::TemporalTonemapDurand::prepare(float sigS, float sigR, float base)
{
	this->sigS = sigS;
	this->sigR = sigR;
	this->base = base;
}

void hdr::tmo::TemporalTonemapDurand::apply(cv::Mat &im)
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

	findExtremes(lumaLog, min, max);

	cv::Mat tmp;
	cv::Mat tmp1;
	cv::medianBlur(lumaLog, tmp1, 3);

	cv::bilateralFilter(tmp1, tmp, 11, 2, 2);

	if (processed == 0 || temporalAdaptation == 0.0f)
	{
		baseLayer = tmp;
		baseLayerG = tmp.clone();
	}
	else
	{
		baseLayer = baseLayerG *  (1-temporalAdaptation) +  tmp * temporalAdaptation;
		baseLayerG = tmp;
	}


	baseLayer = tmp;


	cv::Scalar mean, stddev;
	findExtremes(baseLayer, min, max);



	im.copyTo(this->output);

	cv::Mat dstMap(this->input.size(), CV_8UC3);
	toneMapping(this->lumaOrig,
				this->lumaLog,
				this->baseLayer,
				this->input,
				this->output,
				dstMap,
				this->base);


	bmp = cv::Mat(this->output.size(), CV_8UC3);

	applyGamma(output, dstMap,  1.5, 1.5, 1.5);

	cv::meanStdDev(output, mean, stddev);


	for (int32_t i = 0; i < this->output.rows; ++i)
	{
		for (int32_t j = 0; j < this->output.cols; ++j)
		{
			cv::Vec3f pix = this->output.at<cv::Vec3f>(i, j);

			pix[0] = pix[0] > 0 ? pix[0] : 0;
			pix[1] = pix[1] > 0 ? pix[1] : 0;
			pix[2] = pix[2] > 0 ? pix[2] : 0;

			bmp.at<cv::Vec3b>(i, j)[0] = pix[0] * 255 > 255 ? 255 : pix[0] * 255;
			bmp.at<cv::Vec3b>(i, j)[1] = pix[1] * 255 > 255 ? 255 : pix[1] * 255;
			bmp.at<cv::Vec3b>(i, j)[2] = pix[2] * 255 > 255 ? 255 : pix[2] * 255;
		}
	}


}

void hdr::tmo::TemporalTonemapDurand::show()
{

}

void hdr::tmo::TemporalTonemapDurand::writeImages(std::string path)
{

	cv::imwrite(path, bmp);
}

cv::Mat hdr::tmo::TemporalTonemapDurand::getImage()
{
	return bmp;
}



void hdr::tmo::TemporalTonemapDurand::getLuminance(cv::Mat &input, cv::Mat& output)
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


void  hdr::tmo::TemporalTonemapDurand::applyLog(cv::Mat &input, cv::Mat& output)
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

void hdr::tmo::TemporalTonemapDurand::findExtremes(cv::Mat &im, cv::Vec3f& min, cv::Vec3f& max)
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
				min[k] = MIN(pix[k], min[k]);
			}
		}
	}



}

void hdr::tmo::TemporalTonemapDurand::findExtremes(cv::Mat &im, cv::Vec3f& min, cv::Vec3f& max, float percentil)
{
	float locMax;
	float locMin;

	std::vector<float> maxVectorB;
	std::vector<float> maxVectorG;
	std::vector<float> maxVectorR;

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
				pix[0] = pix[1] = pix[2] = im.at<float>(i ,j);



			maxVectorB.push_back(pix[0]);
			maxVectorG.push_back(pix[1]);
			maxVectorR.push_back(pix[2]);
		}
	}


    std::sort(maxVectorB.begin(), maxVectorB.end());
    std::sort(maxVectorG.begin(), maxVectorG.end());
    std::sort(maxVectorR.begin(), maxVectorR.end());

    max[0] = maxVectorB.at(maxVectorB.size() * percentil);
    min[0] = maxVectorB.at(maxVectorB.size()  - maxVectorB.size() * percentil);
    max[1] = maxVectorG.at(maxVectorG.size() * percentil);
    min[1] = maxVectorG.at(maxVectorG.size()  - maxVectorG.size() * percentil);
    max[2] = maxVectorR.at(maxVectorR.size() * percentil);
    min[2] = maxVectorR.at(maxVectorR.size()  - maxVectorR.size() * percentil);


}



void  hdr::tmo::TemporalTonemapDurand::toneMapping(cv::Mat &intensitiesOrig,
					cv::Mat &intensitiesLog,
					cv::Mat &base,
					cv::Mat &colors,
					cv::Mat &dst,
				    cv::Mat &dstMap,
					float &baseContrast)
{

	cv::Vec3f max, min;

//	std::cout << "Base TMO: ";
	cv::Vec3f _min(0,0,0);
	cv::Vec3f _max(0,0,0);
	cv::Vec3f _minG(0,0,0);
	cv::Vec3f _maxG(0,0,0);

	findExtremes(base,_min, _max, 0.999);

	mins.push_back(_min);
	maxs.push_back(_max);

	if (std::isnormal(_min[0]) && std::isnormal(_max[0]))
	{
		float a = std::abs(mins[0][0] - _min[0]) * 0.1f;
		float b = std::abs(maxs[0][0] - _max[0]) * 0.1f;

		if (mins[0][0] < _min[0])
		{
			mins[0][0] += a;
		}
		else if (mins[0][0] > _min[0])
		{
			mins[0][0] -= a;
		}

		if (maxs[0][0] < _max[0])
		{
			maxs[0][0] += b;
		}
		else if (maxs[0][0] > _max[0])
		{
			maxs[0][0] -= b;
		}
	}


	float k1 = 1.6774, k2 = 0.9925;
	float compressionFactor = baseContrast/(maxs[0][0]-mins[0][0]);
	float s = ((1 + k1) * powf(compressionFactor, k2)) / ( 1 + k1 * powf(compressionFactor, k2));




	cv:: Mat cm_img0;

	cv::Mat detailMat(base.size(), CV_32FC1);
	cv::Mat detailMat1(base.size(), CV_32FC1);
	cv::Mat iNewMat(base.size(), CV_32FC1);


	detailMat = intensitiesLog - base;


	for (int32_t i = 0; i < base.rows; i++)
	{
		for (int32_t j = 0; j < base.cols; j++)
		{
			float b = 0.0f, I = 0.0f, logI = 0.0f, detail = 0.0f, detail1, bNew = 0.0f, INew = 0.0f;
			cv::Vec3f pix;

			pix = colors.at<cv::Vec3f>(i, j);
			I = intensitiesOrig.at<float>(i, j);
			detail = detailMat.at<float>(i, j);
			detailMat1.at<float>(i, j);
			b = base.at<float>(i, j);
			for (int32_t k = 0; k < 3; k++)
				pix[k] = (float(pix[k]) / I);


			bNew = float(b) * compressionFactor +  detail;
			INew = bNew + (float(mins[0][0])  * compressionFactor);

			INew = expf(INew);
			iNewMat.at<float>(i, j) = INew;


			for (int32_t k = 0; k < 3; k++)
			{
				pix[k] = powf(pix[k], s) * INew;
			}
			dst.at<cv::Vec3f>(i, j) = pix;
		}
	}


	cv::Vec3f _minGlob(0,0,0);
	cv::Vec3f _maxGlob(0,0,0);

	findExtremes(dst,_minGlob, _maxGlob);
	findExtremes(dst,_min, _max, 0.999);

	minsMap.push_back(_min);
	maxsMap.push_back(_max);

	for (int i = 0; i < 3; i++)
	{
		float a = std::abs(minsMap[0][i] - _min[i]) * 0.1f;
		float b = std::abs(maxsMap[0][i] - _max[i]) * 0.1f;
		if (minsMap[0][i] > _min[i])
		{
			minsMap[0][i] -= a;
		}
		else if (minsMap[0][i] < _min[i])
		{
			minsMap[0][i] += a;
		}

		if (maxsMap[0][i] > _max[i])
		{
			maxsMap[0][i] -= b;
		}
		else if (maxsMap[0][i] < _max[i])
		{
			maxsMap[0][i] += b;
		}
	}

	for (int32_t i = 0; i < base.rows; i++)
	{
		for (int32_t j = 0; j < base.cols; j++)
		{
			cv::Vec3f pix;
			pix = dst.at<cv::Vec3f>(i, j);


			dstMap.at<cv::Vec3b>(i, j)[0] = 0;
			dstMap.at<cv::Vec3b>(i, j)[1] = 0;
			dstMap.at<cv::Vec3b>(i, j)[2] = 0;


			for (int32_t k = 0; k < 3; k++)
			{
				pix[k] = (pix[k] - minsMap[0][k])  / (maxsMap[0][k] - minsMap[0][k]);
			}

			dst.at<cv::Vec3f>(i, j) = pix;
		}
	}

	processed++;

}

void  hdr::tmo::TemporalTonemapDurand::applyGamma(cv::Mat &input, cv::Mat &map, float gammaR , float gammaG , float gammaB)
{
	for( int32_t i = 0; i < input.rows; i++ )
	{
		for( int32_t j = 0; j < input.cols; j++ )
		{
			/* extract intesities  */
			if (map.at<cv::Vec3b>(i,j)[0] == 0)
			{
				cv::Vec3f rgbTriplet = input.at<cv::Vec3f>(i, j);
				input.at<cv::Vec3f>(i, j)[0] = powf(rgbTriplet[0], (1 / gammaB));
				input.at<cv::Vec3f>(i, j)[1] = powf(rgbTriplet[1], (1 / (gammaG)));
				input.at<cv::Vec3f>(i, j)[2] = powf(rgbTriplet[2], (1 / (gammaR)));
			}
		}
	}
}

void  hdr::tmo::TemporalTonemapDurand::applyGammaB(cv::Mat &input, cv::Mat &map, float gamma)
{
	for( int32_t i = 0; i < input.rows; i++ )
	{
		for( int32_t j = 0; j < input.cols; j++ )
		{

			cv::Vec3b rgbTriplet = input.at<cv::Vec3b>(i, j);
			input.at<cv::Vec3b>(i, j)[0] = powf(rgbTriplet[0], (1 / gamma));
			input.at<cv::Vec3b>(i, j)[1] = powf(rgbTriplet[1], (1 / gamma));
			input.at<cv::Vec3b>(i, j)[2] = powf(rgbTriplet[2], (1 / gamma));
		}
	}
}

float hdr::tmo::TemporalTonemapDurand::getMin()
{
	return mins[0][0];
}


