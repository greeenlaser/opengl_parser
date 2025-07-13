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
static bool ParseLayers();

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
	
	PrintMessage(
		MessageType::TYPE_MESSAGE, 
		"Starting to parse layers...");
	
	if (ParseLayers()) 
	{
		PrintMessage(
			MessageType::TYPE_SUCCESS,
			"Parsed all layers!");
	}
	else 
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to parse layers!");
			
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
	path filePath = path(parser_in / "vk.xml");
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

	unordered_map<string, vector<string>> cleanedLines{};

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
	auto ParseDepends = [](string& str) -> vector<string>
		{
			vector<string> result{};
			const string key = "depends=\"";
			auto pos = str.find(key);
			if (pos == string::npos) return result; //no depends=

			auto valueStart = pos + key.size();
			auto valueEnd = str.find('"', valueStart);
			if (valueEnd == string::npos) return result; //malformed

			string content = str.substr(valueStart, valueEnd - valueStart);

			size_t start = 0;
			while (start <= content.size())
			{
				auto comma = content.find(',', start);
				string token{};
				if (comma == string::npos)
				{
					token = content.substr(start);
					if (!token.empty()) result.push_back(token);
					break;
				}
				else
				{
					token = content.substr(start, comma - start);
					if (!token.empty()) result.push_back(token);
					start = comma + 1;
				}
			}

			return result;
		};

	while (getline(file, line))
	{
		if (line.find("<extension") == string::npos) continue;

		//
		// FILTER BAD RESULTS
		//

		bool isSupported = (
			line.find("supported=\"disabled") == string::npos
			&& line.find("supported=\"deprecated") == string::npos
			&& line.find("supported=\"vulkansc") == string::npos);

		if (!isSupported) continue;

		bool isPromotedToCorrectVersion = (
			line.find("promotedto=\"VK_VERSION_1_0") != string::npos
			|| line.find("promotedto=\"VK_VERSION_1_1") != string::npos
			|| line.find("promotedto=\"VK_VERSION_1_2") != string::npos
			|| line.find("promotedto") == string::npos);

		if (!isPromotedToCorrectVersion) continue;

		bool isCorrectVersion = (
			line.find("VK_VERSION_1_3") == string::npos
			&& line.find("VK_VERSION_1_4") == string::npos
			&& line.find("VK_VERSION_1_5") == string::npos);

		if (!isCorrectVersion) continue;

		bool isVendor = (
			(line.find("name=\"VK_KHR_") == string::npos
			&& line.find("name=\"VK_EXT_") == string::npos));

		if (isVendor) continue;

		bool isAllowedPlatform = (
			line.find("platform=\"android") == string::npos
			&& line.find("platform=\"directfb") == string::npos
			&& line.find("platform=\"metal") == string::npos
			&& line.find("platform=\"provisional") == string::npos
			&& line.find("platform=\"wayland") == string::npos
			&& line.find("platform=\"xcb") == string::npos
			&& line.find("_drm") == string::npos);

		if (!isAllowedPlatform) continue;

		//
		// CLEAN UP GOOD RESULTS
		//

		Trim(line);

		RemoveChar(line, '<');
		RemoveChar(line, '>');
		RemoveChar(line, ' ');
		RemoveStr(line, "extension");

		RemoveTag(line, "author");
		RemoveTag(line, "contact");
		RemoveTag(line, "number");
		RemoveTag(line, "specialuse");
		RemoveTag(line, "ratified");
		RemoveTag(line, "comment");

		AddCommaAfterQuote(line);

		line = ReplaceChar(line, '+', ',');
		RemoveChar(line, '(');
		RemoveChar(line, ')');

		vector<string> dependsData = ParseDepends(line);

		cleanedLines[line] = dependsData;
	}

	file.close();

	vector<string> deviceExtensions{};
	vector<string> instanceExtensions{};

	auto ExtractAttribute = [](const string& line, const string& key) -> string
		{
			string pattern = key + "=\"";
			size_t start = line.find(pattern);
			if (start == string::npos) return "";

			start += pattern.length();
			size_t end = line.find('"', start);
			if (end == string::npos) return "";

			return line.substr(start, end - start);
		};

	unordered_set<string> validExtensionKeys{};
	for (const auto& [k, _] : cleanedLines)
	{
		string name = ExtractAttribute(k, "name");
		validExtensionKeys.insert(name);
	}

	for (const auto& [key, deps] : cleanedLines)
	{
		string name = ExtractAttribute(key, "name");
		string type = ExtractAttribute(key, "type");

		int maxMinor = -1;
		vector<string> extDeps{};
		for (auto const& dep : deps)
		{
			if (dep.rfind("VK_VERSION_1_", 0) == 0)
			{
				int minor = stoi(dep.substr(strlen("VK_VERSION_1_")));
				maxMinor = max(maxMinor, minor);
			}
			else extDeps.push_back(dep);
		}

		if (maxMinor >= 3)
		{
			PrintMessage(
				MessageType::TYPE_MESSAGE,
				"Skipped extension '" + name + "' because version is '" + to_string(maxMinor) + "'!",
				4);
			continue;
		}

		bool isValid = true;
		for (auto const& e : extDeps)
		{
			if (!validExtensionKeys.count(e))
			{
				isValid = false;
				PrintMessage(
					MessageType::TYPE_MESSAGE,
					"Skipped extension '" + name + "' because Vulkan 1.2 does not have extension '" + e + "'!",
					4);
				break;
			}
		}

		if (!isValid) continue;

		if (type == "device") deviceExtensions.push_back(name);
		else if (type == "instance") instanceExtensions.push_back(name);

		PrintMessage(
			MessageType::TYPE_SUCCESS,
			"Found '" + type + "' extension '" + name + "'!",
			2);
	}

	size_t deviceExtSize = deviceExtensions.size();
	if (deviceExtSize == 0)
	{
		PrintMessage(
			MessageType::TYPE_ERROR,
			"Failed to find any device extensions from file '" + filePath.string() + "'!");
		return false;
	}

	size_t instanceExtSize = instanceExtensions.size();
	if (instanceExtSize == 0)
	{
		PrintMessage(
			MessageType::TYPE_ERROR,
			"Failed to find any instance extensions from file '" + filePath.string() + "'!");
		return false;
	}

	sort(deviceExtensions.begin(), deviceExtensions.end());
	sort(instanceExtensions.begin(), instanceExtensions.end());

	//
	// SAVE DEVICE EXTENSIONS
	//

	//write output
	path deviceExtensionsPath = path(parser_out / "device_extensions.txt");

	ofstream outDev(deviceExtensionsPath);
	if (!outDev.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR,
			"Failed to open file '" + deviceExtensionsPath.string() + "' for writing!");
		return false;
	}

	for (const auto& ext : deviceExtensions)
	{
		outDev << ext << "\n";
	}
	outDev.close();

	PrintMessage(
		MessageType::TYPE_SUCCESS,
		"Found and saved '" + to_string(deviceExtSize) +
		"' device extensions to '" + deviceExtensionsPath.string() + "'!");

	//
	// SAVE INSTANCE EXTENSIONS
	//

	//write output
	path instanceExtensionsPath = path(parser_out / "instance_extensions.txt");

	ofstream outIns(instanceExtensionsPath);
	if (!outIns.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR,
			"Failed to open file '" + instanceExtensionsPath.string() + "' for writing!");
		return false;
	}

	for (const auto& ext : instanceExtensions)
	{
		outIns << ext << "\n";
	}
	outIns.close();

	PrintMessage(
		MessageType::TYPE_SUCCESS,
		"Found and saved '" + to_string(instanceExtSize) +
		"' instance extensions to '" + instanceExtensionsPath.string() + "'!");

	return true;
}

bool ParseLayers()
{
	vector<string> instanceLayers{};
	
	for (const auto& file : directory_iterator(parser_in))
	{
		path filePath = path(file);
		
		if (filePath.extension() != ".json") continue;
		
		ifstream file(filePath);
		if (!file.is_open())
		{
			PrintMessage(
				MessageType::TYPE_ERROR, 
				"Failed to open file '" + filePath.string() + "' for reading!");
			return false;
		}
		
		string line{};
		string layerName{};
		
		while (getline(file, line))
		{
			auto pos = line.find("\"name\"");
			if (pos != string::npos)
			{
				size_t start = line.find("\"", pos + 6);
				size_t end = line.find("\"", start + 1);
				if (start != string::npos
					&& end != string::npos)
				{
					layerName = line.substr(start + 1, end - start - 1);
					break;
				}
			}
		}
		file.close();
		
		if (layerName.empty()) continue;
			
		if (layerName != "VK_LAYER_KHRONOS_validation" 
			&& !layerName.starts_with("VK_LAYER_KHRONOS_")
			&& !layerName.starts_with("VK_LAYER_LUNARG_"))
		{
			if (layerName.starts_with("VK_LAYER_RENDERDOC_")
				|| layerName.starts_with("VK_LAYER_NV_")
				|| layerName.starts_with("VK_LAYER_MESA_")
				|| layerName.starts_with("VK_LAYER_INTEL_")
				|| layerName.starts_with("VK_LAYER_GOOGLE_")
				|| layerName.starts_with("VK_LAYER_OBSOLETE_"))
			{
				continue;
			}
		}
			
		instanceLayers.push_back(layerName);
			
		PrintMessage(
			MessageType::TYPE_SUCCESS, 
			"Found instance layer '" + layerName + "'!",
			2);
	}
	
	size_t instanceLayersSize = instanceLayers.size();
	if (instanceLayersSize == 0)
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to find any vulkan 1.2 instance layers!");
		return false;
	}
		
	//write output
	path instanceLayerOutputPath = path(parser_out / "instance_layers.txt");
		
	ofstream instanceLayerOut(instanceLayerOutputPath);
	if (!instanceLayerOut.is_open())
	{
		PrintMessage(
			MessageType::TYPE_ERROR, 
			"Failed to open file '" + instanceLayerOutputPath.string() + "' for writing!");
		return false;
	}
		
	for (const auto& ext : instanceLayers)
	{
		instanceLayerOut << ext << "\n";
	}
	instanceLayerOut.close();
		
	PrintMessage(
		MessageType::TYPE_SUCCESS, 
		"Found and saved '" + to_string(instanceLayersSize) + 
		"' instance layers to '" + instanceLayerOutputPath.string() + "'!");
	
	return true;
}