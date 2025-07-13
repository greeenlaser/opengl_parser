#include <iostream>
#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <vector>
#include <array>
#include <unordered_map>
#include <unordered_set>

using std::cout;
using std::cin;
using std::filesystem::current_path;
using std::filesystem::path;
using std::filesystem::exists;
using std::filesystem::copy_file;
using std::filesystem::copy_options;
using std::filesystem::directory_iterator;
using std::string;
using std::to_string;
using std::ifstream;
using std::ofstream;
using std::getline;
using std::vector;
using std::exception;
using std::string_view;
using std::array;
using std::pair;
using std::unordered_map;
using std::unordered_set;
using std::max;

enum class MessageType
{
	TYPE_MESSAGE,
	TYPE_SUCCESS,
	TYPE_ERROR
};

static void PrintMessage(
	MessageType type,
	const string& message,
	int indentCount = 0);
static bool ParseExtensions();

static path parser_in{};
static path parser_out{};
static path result_log_path{};
static ofstream result_log{};

int main ()
{
	parser_in = path(current_path().parent_path().parent_path().parent_path() / "parser_in");
	if (!exists(parser_in))
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find parser_in folder from '" + parser_in.string() + "'!");
		
		cin.get();
		return 0;
	}
	
	parser_out = path(current_path().parent_path().parent_path().parent_path() / "parser_out");
	if (!exists(parser_out))
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find parser_out folder from '" + parser_out.string() + "'!");
			
		cin.get();
		return 0;
	}
	
	result_log_path = path(parser_out / "result_log.txt");
	result_log.open(result_log_path);
	if (!result_log.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to open '" + result_log_path.string() + "' for writing!");
		
		cin.get();
		return 0;
	}
	
	PrintMessage(
		MessageType::TYPE_MESSAGE, 
		"Starting to parse extensions...");
	
	if (ParseExtensions()) 
	{
		PrintMessage(
			MessageType::TYPE_SUCCESS,
			"Parsed all extensions!");
	}
	else 
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to parse extensions!");
			
		result_log.close();
		cin.get();
		return 0;
	}
	
	result_log.close();
	cin.get();
	return 0;
}

void PrintMessage(
	MessageType type,
	const string& message,
	int indentCount)
{
	string typeStr{};
	switch (type)
	{
	case MessageType::TYPE_MESSAGE:
	{
		typeStr = "";
		break;
	}
	case MessageType::TYPE_SUCCESS:
	{
		typeStr = "[SUCCESS] ";
		break;
	}
	case MessageType::TYPE_ERROR:
	{
		typeStr = "[ERROR] ";
		break;
	}
	}
	
	string indentStr{};
	for (int i = 0; i < indentCount; ++i)
	{
		indentStr += " ";
	}
	
	string fullMessage = indentStr + typeStr + message + "\n";
	
	cout << fullMessage;
	result_log << fullMessage;
}

bool ParseExtensions()
{
	path filePath = path(parser_in / "gl.xml");
	if (!exists(filePath))
	{
		PrintMessage(
			MessageType::TYPE_ERROR,
			"Failed to find file '" + filePath.string() + "'!");
		return false;
	}

	ifstream file(filePath);
	if (!file.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR,
			"Failed to open file '" + filePath.string() + "' for reading!");
		return false;
	}

	string line{};

	auto RemoveChar = [](string& str, char c)
		{
			str.erase(remove(str.begin(), str.end(), c), str.end());
		};
	auto RemoveStr = [](string& str, string_view sub)
		{
			if (sub.empty()) return;

			size_t pos = 0;
			while ((pos = str.find(sub, pos)) != string::npos)
			{
				str.erase(pos, sub.size());
			}
		};
	auto ReplaceChar = [](const string& str, char original, char target)
		{
			string out = str;
			replace(out.begin(), out.end(), original, target);
			return out;
		};
	auto Trim = [](string& str)
		{
			const string ws = " \t\n\r\f\v";

			//find first non-whitespace
			auto start = str.find_first_not_of(ws);

			//all whitespace or empty
			if (start == string::npos)
			{
				str.clear();
				return;
			}

			//find last non-whitespace
			auto end = str.find_last_not_of(ws);

			str.erase(end + 1);
			str.erase(0, start);
		};
	auto RemoveTag = [](string& str, const string& tag)
		{
			const string key = tag + "=\"";
			size_t pos = 0;
			while ((pos = str.find(key, pos)) != string::npos)
			{
				size_t start = pos;
				pos += key.size();

				//find the closing '"'
				size_t end = str.find('"', pos);
				//no closing quote - erase from start to end of string
				if (end == string::npos)
				{
					str.erase(start);
					return;
				}

				//erase from the start of tag through the closing quote
				str.erase(start, (end - start) + 1);

				//continues onward to see if there are more
			}
		};
	auto AddCommaAfterQuote = [](string& str)
		{
			bool isOpening = false;
			for (size_t i = 0; i < str.size(); ++i)
			{
				if (str[i] != '"') continue;

				isOpening = !isOpening;
				if (!isOpening
					&& i + 1 < str.size())
				{
					str.insert(i + 1, ", ");
					i += 2;
				}
			}
		};

	//
	// STEP 1: GET ALL KNOWN EXTENSIONS AND STORE VERIFIED ONES
	//

	vector<string> extensions{};
	while (getline(file, line))
	{
		if (line.find("<extension") == string::npos) continue;

		//
		// FILTER BAD RESULTS
		//

		bool isSupported = line.find("glcore") != string::npos;

		if (!isSupported) continue;

		bool isValid = (
			(line.find("name=\"GL_KHR_") == string::npos
			&& line.find("name=\"GL_EXT_") == string::npos
			&& line.find("name=\"GL_ARB_") == string::npos));

		if (isValid) continue;

		bool isUseless = (
			line.find("GL_EXT_EGL") != string::npos
			|| line.find("GL_ARB_ES") != string::npos);

		if (isUseless) continue;

		//
		// CLEAN UP GOOD RESULTS
		//
		
		Trim(line);

		RemoveChar(line, '<');
		RemoveChar(line, '>');

		RemoveTag(line, "supported");
		RemoveTag(line, "comment");

		RemoveStr(line, "extension ");
		RemoveStr(line, "name");

		RemoveChar(line, '=');
		RemoveChar(line, '/');
		RemoveChar(line, '"');
		RemoveChar(line, ' ');

		extensions.push_back(line);
	}

	file.close();

	//
	// STEP 2: EXTRACT ALL FEATURES TO FIGURE OUT WHICH EXTENSIONS ARE CORE
	//

	file.open(filePath);

	bool inFeature = false;
	vector<string> currentFeature{};
	vector<vector<string>> allFeatures{};

	auto CaptureFeature = [&](const string& line)
		{
			if (!inFeature)
			{
				if (line.find("<feature") != string::npos)
				{
					inFeature = true;
					currentFeature.clear();
					currentFeature.push_back(line);
				}
			}
			else
			{
				currentFeature.push_back(line);
				if (line.find("</feature") != string::npos)
				{
					inFeature = false;
					allFeatures.push_back(currentFeature);
				}
			}
		};

	while (getline(file, line))
	{
		//stops once we reach 4.0, we dont care about that stuff
		if (line.find("name=\"GL_VERSION_4_0") != string::npos) break;

		CaptureFeature(line);
	}

	file.close();

	//
	// STEP 3: SORT THROUGH FILTERS TO FIGURE OUT WHICH EXTENSIONS WERE ADDED TO CORE
	//

	vector<string> cleanedFeatureLines{};
	for (auto& feature : allFeatures)
	{
		for (auto& line : feature)
		{
			bool isUseless = (
				line.find("Reuse ") == string::npos
				&& line.find("Promoted from ") == string::npos);

			if (isUseless) continue;

			Trim(line);
			RemoveChar(line, '<');
			RemoveChar(line, '>');
			RemoveStr(line, "require");

			if (line.find("Reuse tokens from ") != string::npos)
			{
				RemoveStr(line, "Reuse tokens from");
			}
			if (line.find("Reuse commands from ") != string::npos)
			{
				RemoveStr(line, "Reuse commands from");
			}
			if (line.find("Reuse ") != string::npos)
			{
				RemoveStr(line, "Reuse ");
			}
			if (line.find("Promoted from ") != string::npos)
			{
				RemoveStr(line, "Promoted from ");
			}

			RemoveStr(line, "comment=");
			RemoveStr(line, " (none)");
			RemoveStr(line, " subset to core");
			RemoveStr(line, " compatibility profile");
			RemoveStr(line, "profile=\"compatibility\"");

			RemoveChar(line, '"');
			RemoveChar(line, ' ');

			line = "GL_" + line;

			cleanedFeatureLines.push_back(line);
		}
	}

	for (const auto& line : cleanedFeatureLines)
	{
		//auto oldSize = extensions.size();

		extensions.erase(remove(
			extensions.begin(),
			extensions.end(),
			line), extensions.end());

		/*
		auto newSize = extensions.size();

		if (newSize < oldSize)
		{
			cout << "Removed core extension: " << line << "\n";
		}
		*/
	}

	//cout << "\n";

	//these extensions were marked as core
	//before the khronos registry started annotating promotions
	//so they must be manually removed
	vector<string> manuallyRemoved
	{
		"GL_ARB_texture_border_clamp",     //core since OpenGL 1.3
		"GL_ARB_texture_non_power_of_two", //core since OpenGL 2.0
		"GL_ARB_pixel_buffer_object",      //core since OpenGL 2.1
		"GL_ARB_draw_instanced",           //core since OpenGL 3.1
		"GL_ARB_texture_buffer_object",    //core since OpenGL 3.1
		"GL_ARB_geometry_shader4",         //core since OpenGL 3.2
		"GL_ARB_vertex_array_bgra",        //core since OpenGL 3.2
		"GL_ARB_instanced_arrays"          //core since OpenGL 3.3
	};

	for (const auto& line : manuallyRemoved)
	{
		extensions.erase(remove(
			extensions.begin(),
			extensions.end(),
			line), extensions.end());
	}

	//
	// AND FINALLY SAVE TO EXTERNAL FILE
	//

	size_t extSize = extensions.size();
	if (extSize == 0)
	{
		PrintMessage(
			MessageType::TYPE_ERROR,
			"Failed to find any extensions from file '" + filePath.string() + "'!");
		return false;
	}

	sort(extensions.begin(), extensions.end());

	//
	// SAVE EXTENSIONS
	//

	//write output
	path extensionsPath = path(parser_out / "extensions.txt");

	ofstream outEx(extensionsPath);
	if (!outEx.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR,
			"Failed to open file '" + extensionsPath.string() + "' for writing!");
		return false;
	}

	for (const auto& ext : extensions)
	{
		outEx << ext << "\n";
	}
	outEx.close();

	PrintMessage(
		MessageType::TYPE_SUCCESS,
		"Found and saved '" + to_string(extSize) +
		"' extensions to '" + extensionsPath.string() + "'!");

	return true;
}