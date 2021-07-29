#include <iostream>

#include <iostream>
#include <filesystem>
#include <vector>

#include <opencv2/opencv.hpp>

#include <elpp/easylogging.hpp>

#include <cxxopts/cxxopts.hpp>

#include <hdr/hdr_lib.hpp>
#include <hdr/hdrlib_temporal_tonemap_durand.hpp>

INITIALIZE_EASYLOGGINGPP

static void setLogLevel(std::string level);
static int process(cxxopts::ParseResult& opts);

int main(int argc, char *argv[])
{
	LOG(INFO) << "HDR Convertor";
	LOG(INFO) << "BUT FIT, DCGM";


	cxxopts::Options options(argv[0], " - hdr convert command line tool ");

	cxxopts::ParseResult result;
	try
	{
		options.positional_help("[optional args]")
				.show_positional_help();

		options.allow_unrecognised_options()
				.add_options()
						("l,log", R"(Log level-  "off", "fatal", "error", "warning", "info", "all")", cxxopts::value<std::string>()->default_value("info"))
						("i,image", "Input HDR image", cxxopts::value<std::string>()->default_value(""))
						("p,path", "Input path which contains HDR frames of HDR video", cxxopts::value<std::string>()->default_value(""))
						("d,display", "Display output", cxxopts::value<bool>()->default_value("false"))
						("s,show", "Display output", cxxopts::value<bool>()->default_value("false"))
						("h,help", "Display help", cxxopts::value<bool>()->default_value("false"))
						("b,base", "base contrast (for most scenes 3.5 is sufficient for good results)", cxxopts::value<float>()->default_value("3.5"))
						("o,output", "Output directory (if type == image then directory is expected) or Output file (if type == video then output video path is expected)", cxxopts::value<std::string>()->default_value(""))
						("t,type", "Type of output (image|video)", cxxopts::value<std::string>()->default_value("image"))
						("w,cv_waitkey", "Value for opencv waitkey", cxxopts::value<int>()->default_value("0"));



		result = options.parse(argc, const_cast<char **&>(argv));
	}
	catch (const cxxopts::OptionException& e)
	{
		std::cerr << "error parsing options: " << e.what() << std::endl;
		return -1;
	}


	if (result["help"].as<bool>())
	{
		std::cout << options.help() << std::endl;
		return 0;
	}

	setLogLevel(result["log"].as<std::string>());


	process(result);

	return 0;
}


void setLogLevel(std::string level)
{

	std::string lowerString;
	std::transform(level.begin(), level.end(), std::back_inserter(lowerString),
   [](auto c)
   {
	   return std::tolower(c);
   });

	level = lowerString;

	el::Configurations defaultConf;
	defaultConf.setToDefault();

	defaultConf.setGlobally(el::ConfigurationType::Format, "%datetime %level %msg");

	defaultConf.set(el::Level::Global, el::ConfigurationType::Enabled, "true");

	defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "false");
	defaultConf.set(el::Level::Verbose, el::ConfigurationType::Enabled, "false");
	defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "false");
	defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "false");
	defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "false");
	defaultConf.set(el::Level::Error, el::ConfigurationType::Enabled, "false");
	defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "false");


	if (level == "all")
	{
		defaultConf.set(el::Level::Debug, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Verbose, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Trace, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Error, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "true");
	}
	else if (level == "info")
	{
		defaultConf.set(el::Level::Info, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Error, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "true");
	}
	else if (level == "warning")
	{
		defaultConf.set(el::Level::Warning, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Error, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "true");
	}
	else if (level == "error")
	{
		defaultConf.set(el::Level::Error, el::ConfigurationType::Enabled, "true");
		defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "true");
	}
	else if (level == "fatal")
	{
		defaultConf.set(el::Level::Fatal, el::ConfigurationType::Enabled, "true");
	}


	el::Loggers::reconfigureAllLoggers(defaultConf);
}

int process(cxxopts::ParseResult& opts)
{
	std::vector<std::filesystem::path> files;


	if (opts.count("path"))
	{
		std::string path = opts["path"].as<std::string>();
		if (std::filesystem::is_directory(path))
		{
			for (auto &p: std::filesystem::directory_iterator(path))
			{
				auto pp = p.path();
				files.emplace_back(pp);
			}
		}
		else
		{
			LOG(ERROR) << "Not a directory: " << path;
			return -1;
		}
	}
	else if (opts.count("file"))
	{
		std::string f = opts["file"].as<std::string>();
		if (std::filesystem::is_regular_file(f))
		{
			files.emplace_back(f);
		}
		else
		{
			LOG(ERROR) << "Not a file: " << f;
			return -1;
		}
	}
	else
	{
		LOG(ERROR) << "Missing input files";
		return -2;
	}

	float base = opts["base"].as<float>();



	cv:: VideoWriter video;

	bool started = true;


	hdr::tmo::TemporalTonemapDurand durand(0.8, base);
	for (const auto &f : files)
	{
		LOG(INFO) << "Processing: " << f.filename();

		cv::Mat mat = cv::imread(f.string(), cv::IMREAD_UNCHANGED);

		if (started)
		{
			if (opts["type"].as<std::string>() == "video")
			{
				video = cv::VideoWriter(opts["output"].as<std::string>().c_str(), cv::VideoWriter::fourcc('M','J','P','G'), 25, mat.size());
			}
			started = false;
		}


		if (mat.channels() == 1)
		{
			cv::normalize(mat, mat, 1, 65535, cv::NORM_MINMAX, CV_32F);

			cv::Mat cvimageSRC = cv::Mat(mat.rows, mat.cols, CV_16UC1);
			mat.convertTo(cvimageSRC, CV_16UC1);

			cv::Mat cvimageDST = cv::Mat(mat.rows, mat.cols, CV_16UC3);

			cv::cvtColor(cvimageSRC, cvimageDST, cv::COLOR_BayerBG2BGR);

			mat = cv::Mat(mat.rows, mat.cols, CV_32FC3);
			cvimageDST.convertTo(mat, CV_16UC1);
		}


		mat.convertTo(mat, CV_32FC3);


		if (mat.empty())
		{
			continue;
		}

		durand.apply(mat);

		auto toneMapped = durand.getImage();

		if (opts["type"].as<std::string>() == "video")
		{
			video.write(toneMapped);
		}
		else if (opts["type"].as<std::string>() == "image")
		{
			cv::imwrite(opts["output"].as<std::string>().c_str(), toneMapped);
		}

		if (opts["show"].as<bool>() || opts["display"].as<bool>())
		{
			cv::imshow("output", toneMapped);
			cv::waitKey(opts["cv_waitkey"].as<int>());
		}
	}


	return 0;

}