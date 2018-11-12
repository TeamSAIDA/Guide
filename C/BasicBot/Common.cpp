#include "Common.h"

using namespace MyBot;

void Logger::appendTextToFile(const std::string & logFile, const std::string & msg)
{
	std::ofstream logStream;
	logStream.open(logFile.c_str(), std::ofstream::app);
	logStream << msg;
	logStream.flush();
	logStream.close();
}

void Logger::appendTextToFile(const std::string & logFile, const char *fmt, ...)
{
	va_list arg;

	va_start(arg, fmt);
	//vfprintf(log_file, fmt, arg);
	char buff[256];
	vsnprintf_s(buff, 256, fmt, arg);
	va_end(arg);

	std::ofstream logStream;
	logStream.open(logFile.c_str(), std::ofstream::app);
	logStream << buff;
	logStream.flush();
	logStream.close();
}

void Logger::overwriteToFile(const std::string & logFile, const std::string & msg)
{
	std::ofstream logStream(logFile.c_str());
	logStream << msg;
	logStream.flush();
	logStream.close();
}


std::string FileUtil::readFile(const std::string & filename)
{
	std::stringstream ss;

	FILE *file;
	errno_t err;

	if ((err = fopen_s(&file, filename.c_str(), "r")) != 0)
	{
		std::cout << "Could not open file: " << filename.c_str();
	}
	else
	{
		char line[4096]; /* or other suitable maximum line size */
		while (fgets(line, sizeof line, file) != nullptr) /* read a line */
		{
			ss << line;
		}
		fclose(file);
	}

	return ss.str();
}

void FileUtil::readResults()
{
	std::string enemyName = BWAPI::Broodwar->enemy()->getName();
	std::replace(enemyName.begin(), enemyName.end(), ' ', '_');

	std::string enemyResultsFile = Config::Files::ReadDirectory + enemyName + ".txt";

	//int wins = 0;
	//int losses = 0;

	FILE *file;
	errno_t err;

	if ((err = fopen_s(&file, enemyResultsFile.c_str(), "r")) != 0)
	{
		std::cout << "Could not open file: " << enemyResultsFile.c_str();
	}
	else
	{
		char line[4096]; /* or other suitable maximum line size */
		while (fgets(line, sizeof line, file) != nullptr) /* read a line */
		{
			//std::stringstream ss(line);
			//ss >> wins;
			//ss >> losses;
		}

		fclose(file);
	}
}

void FileUtil::writeResults()
{
	std::string enemyName = BWAPI::Broodwar->enemy()->getName();
	std::replace(enemyName.begin(), enemyName.end(), ' ', '_');

	std::string enemyResultsFile = Config::Files::WriteDirectory + enemyName + ".txt";

	std::stringstream ss;

	//int wins = 1;
	//int losses = 0;

	//ss << wins << " " << losses << "\n";

	Logger::overwriteToFile(enemyResultsFile, ss.str());
}
