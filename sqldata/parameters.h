
// Parameters class - Loads parameters from command line 
// Copyright (c) 2012 SQLines. All rights reserved

#ifndef sqlines_parameters_h
#define sqlines_parameters_h

#include <map>

#define PARAMETER_START_TOKEN	'-'				

// Option to print help on available parameters (? is also suported)
#define HELP_PARAMETER			"-?"				

typedef std::map<std::string, std::string> ParametersMap;
typedef std::pair<std::string, std::string> ParametersPair;

//////////////////////////////////////////////////////////////////////

class Parameters
{
	// Key-value map of parameters
	ParametersMap _map;

	// Default configuration file
	const char *_cfg_file;

public:
	Parameters();

	// Return the value of the parameter if it is set to True/Yes, or False/No
	const char* GetTrue(const char *name);
	const char* GetFalse(const char *name);

	// Get integer value or the default
	int GetInt(const char *name, int def);

	// Load parameters from command line 
	int Load(int argc, char** argv);
	// Load parameters from a string and configuration file
	int LoadStr(const char *input);

	// Get the value by key
	char* Get(const char *key);

	// Set the default configuration file
	void SetConfigFile(const char *file) { _cfg_file = file; }

private:
	// Load command line parameters
	int LoadCommandLine(int argc, char** argv);
	// Load parameters from a string only
	int LoadStrItem(const char *input);
	// Load parameters from a configuration file
	int LoadConfigFile();
};

#endif // sqlines_parameters_h
