// BEAM OpenCL Miner
// Main Function
// Copyright 2018 The Beam Team	
// Copyright 2018 Wilke Trei

#include "beamStratum.h"
#include "clHost.h"

// Defining global variables
const string StrVersionNumber = "v1.1.0";
const string StrVersionDate = "Jun 25th 2019";

inline vector<string> &split(const string &s, char delim, vector<string> &elems) {
    stringstream ss(s);
    string item;
    while(getline(ss, item, delim)) {
        elems.push_back(item);
    }
    return elems;
}


inline vector<string> split(const string &s, char delim) {
    vector<string> elems;
    return split(s, delim, elems);
}

uint32_t cmdParser(vector<string> args, string &host, string &port, string &apiCred, bool &debug, bool &beamHashI, vector<int32_t> &devices, bool &force3G ) {
	bool hostSet = false;
	bool apiSet = false;
	
	for (uint32_t i=0; i<args.size(); i++) {
		if (args[i][0] == '-') {
			if ((args[i].compare("-h")  == 0) || (args[i].compare("--help")  == 0)) {
				return 0x4;
			}

			if (args[i].compare("--server")  == 0) {
				if (i+1 < args.size()) {
					vector<string> tmp = split(args[i+1], ':');
					if (tmp.size() == 2) {
						host = tmp[0];
						port = tmp[1];
						hostSet = true;	
						i++;
						continue;
					}
				}
			}

			if (args[i].compare("--key")  == 0) {
				if (i+1 < args.size()) {
					apiCred = args[i+1];
					apiSet = true;
					i++;
					continue;
				}
			}

			if (args[i].compare("--devices")  == 0) {
				if (i+1 < args.size()) {
					vector<string> tmp = split(args[i+1], ',');
					for (int j=0; j<tmp.size(); j++) {
						devices.push_back(stoi(tmp[j]));
					}
					continue;
				}
			}

			if (args[i].compare("--force3G")  == 0) {
				force3G = true;
			}

			if (args[i].compare("--beamHashI")  == 0) {
				beamHashI = true;
			}

			if (args[i].compare("--debug")  == 0) {
				debug = true;
			}
	
			if (args[i].compare("--version")  == 0) {
				cout << StrVersionNumber << " for BEAM main network " << "(" << StrVersionDate << ")" << endl;
				exit(0);
			}
		}
	}

	uint32_t result = 0;
	if (!hostSet) result += 1;
	if (!apiSet) result += 2;

	if (devices.size() == 0) devices.assign(1,-1);
	sort(devices.begin(), devices.end());
	
	return result;
}

int main(int argc, char* argv[]) {

	

	vector<string> cmdLineArgs(argv, argv+argc);
	string host;
	string port;
	string apiCred;
	bool debug = false;
	bool fbeamHashI = false;
	bool useTLS = true;
	vector<int32_t> devices;
	bool force3G = false;

	uint32_t parsing = cmdParser(cmdLineArgs, host, port, apiCred, debug, fbeamHashI, devices, force3G);

	cout << "-====================================-" << endl;
	cout << "                                      " << endl;
	cout << "      BeamHash I / II OpenCL Miner      " << endl;
	cout << "       version:   " + StrVersionNumber << endl;
	cout << "       date:      " + StrVersionDate << endl;
	cout << "                                      " << endl;
	cout << "-====================================-" << endl;
	cout << "" << endl;	
	cout << "Parameters: " << endl;
	cout << " --server:      " << host << ":" << port << endl;	
	cout << " --key:         " << apiCred << endl;
	cout << " --beamHashI:   " << std::boolalpha << fbeamHashI << endl;
	cout << " --force3G:     " << std::boolalpha << force3G << endl;	
	cout << " --debug:       " << std::boolalpha << debug << endl;

	if (parsing != 0) {
		if (parsing & 0x1) {
			cout << "Error: Parameter --server missing" << endl;
		}

		if (parsing & 0x2) {
			cout << "Error: Parameter --key missing" << endl;
		}

		cout << "Parameters: " << endl;
		cout << " --help / -h 			Showing this message" << endl;
		cout << " --server <server>:<port>	The BEAM stratum server and port to connect to (required)" << endl;
		cout << " --key <key>			The BEAM stratum server API key (required), on a Beam mining pool the user name / wallet addres" << endl;
		cout << " --devices <numbers>		A comma seperated list of devices that should be used for mining (default: all in system)" << endl; 
		cout << " --beamHashI			Force mining BeamHash I (pre fork)" << endl;
		cout << " --debug			Enable debug mode - verbose information will be displayed" << endl;
		cout << " --force3G			Force miner to use max 3G for all installed GPUs" << endl;
		cout << " --version			Prints the version number" << endl;
		exit(0);
	}

	beamMiner::beamStratum myStratum(host, port, apiCred, debug);

	beamMiner::clHost myClHost;
	
	cout << endl;
	cout << "Setup OpenCL devices:" << endl;
	cout << "=====================" << endl;
	
	myClHost.setup(&myStratum, devices, fbeamHashI, force3G);

	cout << endl;
	cout << "Waiting for work from stratum:" << endl;
	cout << "==============================" << endl;

	myStratum.startWorking();

	while (!myStratum.hasWork()) {
		this_thread::sleep_for(std::chrono::milliseconds(200));
	}

	cout << endl;
	cout << "Start mining:" << endl;
	cout << "=============" << endl;
	myClHost.startMining();
}

#if defined(_MSC_VER) && (_MSC_VER >= 1900)

FILE _iob[] = { *stdin, *stdout, *stderr };
extern "C" FILE * __cdecl __iob_func(void) { return _iob; }

#endif
