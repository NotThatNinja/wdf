#include <iostream>
#include <vector>
#include <filesystem>

namespace fs = std::filesystem;

struct Arg
{
	bool option;
	std::string arg;
	std::string option_value;
};

void print_help()
{
	std::cout << "Usage: wdf [options] [paths]" << std::endl;
	std::cout << "Options:" << std::endl;
	std::cout << "  -h, --help\t\t\tShow this help message and exit" << std::endl;
	std::cout << "  -e, --exclude [path]\t\tExclude a path from the search" << std::endl;
	std::cout << "  -t, --type [type]\t\tType of file to search for" << std::endl;
}

std::vector<Arg> get_args(int argc, char* argv[])
{
	std::vector<Arg> args;
	bool option = false;
	std::string arg;
	std::string option_value = "";

	// Get args and options
	for (int i = 1; i < argc; i++)
	{
		arg = argv[i];

		if (arg[0] == '-')
		{
			option = true;

			// If option does not exist
			if (arg != "-h" && arg != "--help" && arg != "-e" && arg != "--exclude" && arg != "-t" && arg != "--type")
			{
				std::cerr << "Error: option '" << arg << "' does not exist" << std::endl;
				exit(1);
			}

			// If help option was specified
			if (arg == "-h" || arg == "--help")
			{
				print_help();
				exit(1);
			}

			// If option needs a value
			if (arg == "-e" || arg == "--exclude" || arg == "-t" || arg == "--type")
			{
				// If there is no value
				if (i + 1 >= argc)
				{
					std::cerr << "Error: option '" << arg << "' requires a value" << std::endl;
					exit(1);
				}

				option_value = argv[i + 1];

				// If option is '--type' and option value is not valid
				if ((arg == "-t" || arg == "--type") && option_value != "directory" && option_value != "d" && option_value != "file" && option_value != "f")
				{
					std::cerr << "Error: option '--type' requires a valid value" << std::endl;
					exit(1);
				}
				// Skip next iteration because it will be the option value
				i++;
			}
		}

		args.push_back(Arg{ option, arg, option_value });
	}

	return args;
}

std::vector<std::string> get_root_paths(std::vector<Arg> args)
{
	std::vector<std::string> paths;

	for (Arg arg : args)
	{
		if (!arg.option)
		{
			paths.push_back(arg.arg);
		}
	}

	if (paths.size() < 1)
	{
		// If no paths were specified, use current directory
		paths.push_back(fs::current_path().string());
	}

	return paths;
}

std::vector<std::string> get_excluded_paths(std::vector<Arg> args)
{
	std::vector<std::string> excluded_paths;

	for (Arg arg : args)
	{
		if (arg.arg == "-e" || arg.arg == "--exclude")
		{
			excluded_paths.push_back(arg.option_value);
		}
	}

	return excluded_paths;
}

char get_file_type(std::vector<Arg> args)
{
	char file_type = 'a';

	for (Arg arg : args)
	{
		if (arg.arg == "-t" || arg.arg == "--type")
		{
			if (arg.option_value == "directory" || arg.option_value == "d")
			{
				file_type = 'd';
			}
			else if (arg.option_value == "file" || arg.option_value == "f")
			{
				file_type = 'f';
			}
		}
	}

	return file_type;
}

bool is_path_excluded(fs::path path, std::vector<std::string> excluded_paths)
{
	for (std::string excluded_path : excluded_paths)
	{
		// Check if path contains excluded path
		// We convert everything to generic_string in case the user uses different path separators
		if (path.generic_string().find(fs::path(excluded_path).generic_string()) != std::string::npos)
		{
			return true;
		}
	}

	return false;
}

int main(int argc, char *argv[])
{
	std::vector<Arg> args = get_args(argc, argv);

	std::vector<std::string> root_paths = get_root_paths(args);
	std::vector<std::string> excluded_paths = get_excluded_paths(args);
	char file_type = get_file_type(args);

	std::error_code ec{};

	for (std::string root_path : root_paths)
	{
		// Check if root path exists
		if (!fs::is_directory(root_path))
		{
			std::cerr << "Error: path '" << root_path << "' does not exist" << std::endl;
			exit(1);
		}

		for (const fs::directory_entry& entry : fs::recursive_directory_iterator(root_path, fs::directory_options::skip_permission_denied))
		{
			// Check if path is excluded
			if (is_path_excluded(entry.path(), excluded_paths))
			{
				continue;
			}

			switch (file_type)
			{
				case 'a':
					// Print all files and directories
					std::cout << entry.path().u8string() << std::endl;
				break;
				case 'd':
					// Print only directories
					if (fs::is_directory(entry.path(), ec))
					{
						std::cout << entry.path().u8string() << std::endl;
					}
				break;
				case 'f':
					// Print only files
					if (fs::is_regular_file(entry.path(), ec))
					{
						std::cout << entry.path().u8string() << std::endl;
					}
				break;
			}
		}
	}
}
