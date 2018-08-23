#include <fstream>
#include <sys/time.h>
#include "cpuenergy.hpp"

using namespace std;

double wtime()
{
	struct timeval t;
	gettimeofday(&t, NULL);
	return (double)t.tv_sec + (double)t.tv_usec * 1E-6;
}

int cpu_arch()
{
	int arch = -1, cnt = 3;
	char ch;
	string str, vendor, family;
	ifstream from("/proc/cpuinfo");

	if(!from) {
		cout << "Can not open /proc/cpuinfo" << endl;
		return -1;
	}

	while(cnt) {
		str.clear();
		from >> str;
		if(str.length() == 0)
			break;

		if (str == "vendor_id") {
			from >> ch >> vendor;

			if (vendor != "GenuineIntel") {
				cout << "Invalid vendor id" << endl;
				return -1;
			}
			cnt--;
		}

		if (str == "cpu") {
			str.clear();
			from >> str;
			if (str == "family") {
				from >> ch >> family;

				if (family != "6") {
					cout << "Invalid CPU family" << endl;
					return -1;
				}
				cnt--;
			}
		}

		if (str == "model") {
			from >> ch >> arch;
			cnt--;
		}
	}

	cout << "Intel processor based on the ";

	switch(arch) {
		case ARCH_SANDYBRIDGE:
			cout << "Sandybridge";
			break;
		case ARCH_SANDYBRIDGE_EP:
			cout << "Sandybridge-EP";
			break;
		case ARCH_IVYBRIDGE:
			cout << "Ivybridge";
			break;
		case ARCH_IVYBRIDGE_EP:
			cout << "Ivybridge-EP";
			break;
		case ARCH_HASWELL:
		case ARCH_HASWELL_ULT:
		case ARCH_HASWELL_GT3E:
			cout << "Haswell";
			break;
		case ARCH_HASWELL_EP:
			cout << "Haswell-EP";
			break;
		case ARCH_BROADWELL_M:
		case ARCH_BROADWELL:
			cout << "Broadwell";
			break;
		case ARCH_BROADWELL_EP:
		case ARCH_BROADWELL_DE:
			cout << "Broadwell-EP" ;
			break;
		case ARCH_SKYLAKE:
		case ARCH_SKYLAKE_S:
			cout << "Skylake";
			break;
		case ARCH_SKYLAKE_SP:
			cout << "Skylake-SP";
			break;
		case ARCH_KABYLAKE:
			cout << "Kaby Lake";
			break;
		case ARCH_COFFEELAKE:
			cout << "Coffee Lake";
			break;
		case ARCH_KNIGHTS_LANDING:
			cout << "Knight's Landing";
			break;
		case ARCH_KNIGHTS_MILL:
			cout << "Knight's Mill";
			break;
		case ARCH_ATOM_SILVERMONT:
			cout << "Silvermont";
			break;
		case ARCH_ATOM_AIRMONT:
			cout << "Airmont";
			break;
		case ARCH_ATOM_MOOREFIELD:
			cout << "Moorefield";
			break;
		case ARCH_ATOM_MERRIFIELD:
			cout << "Merrifield";
			break;
		case ARCH_ATOM_GOLDMONT:
			cout << "Goldmont";
			break;
		case ARCH_ATOM_GOLDMONT_P:
			cout << "Goldmont Plus";
			break;
		case ARCH_ATOM_DENVERTON:
			cout << "Denverton";
			break;
		default:
			cout << "microarchitecture " << arch << " which is not supported by Intel RAPL";
			arch = -1;
			break;
	}

	if(arch != -1)
		cout << " microarchitecture" << endl;

	from.close();
	return arch;
}

static int rapl_info(int arch)
{
	try
	{
		int fd[MAX_PACKAGES], cnt = 0, pp1_flag = 0;
		double power_units,time_units, t_val, time, delta;
		double pkg_1[MAX_PACKAGES],pkg_2[MAX_PACKAGES];
		double pp0_1[MAX_PACKAGES],pp0_2[MAX_PACKAGES];
		double pp1_1[MAX_PACKAGES],pp1_2[MAX_PACKAGES];
		double thermal_spec_power;

		switch(arch) {
			case ARCH_ATOM_MOOREFIELD:
			case ARCH_ATOM_SILVERMONT:
			case ARCH_ATOM_AIRMONT:
			case ARCH_KNIGHTS_LANDING:
			case ARCH_KNIGHTS_MILL:
			case ARCH_SANDYBRIDGE_EP:
			case ARCH_IVYBRIDGE_EP:
			case ARCH_HASWELL_EP:
			case ARCH_BROADWELL_DE:
			case ARCH_BROADWELL_EP:
			case ARCH_SKYLAKE_SP:
				pp1_flag = 0;
				break;
			case ARCH_SANDYBRIDGE:
			case ARCH_IVYBRIDGE:
			case ARCH_HASWELL:
			case ARCH_HASWELL_ULT:
			case ARCH_HASWELL_GT3E:
			case ARCH_BROADWELL_M:
			case ARCH_BROADWELL:
			case ARCH_ATOM_GOLDMONT:
			case ARCH_ATOM_GOLDMONT_P:
			case ARCH_ATOM_DENVERTON:
			case ARCH_SKYLAKE:
			case ARCH_SKYLAKE_S:
			case ARCH_KABYLAKE:
			case ARCH_COFFEELAKE:
				pp1_flag = 1;
				break;
		}

		for(int i = 0; i < MAX_PACKAGES; i++) {
			fd[i] = open_pkg(i);
			if(fd[i] == -1) {
				if(cnt == 0) {
					perror("open_pkg");
					return -1;
				}
				else break;
			}

			power_units = get_power_units(fd[i]);
			time_units = get_time_units(fd[i]);
			thermal_spec_power = get_thermal_spec_power(fd[i]);

			time = wtime();
			pkg_1[i] = get_pkg_energy(fd[i]);
			pp0_1[i] = get_pp0_energy(fd[i]);
			if (pp1_flag) {
				pp1_1[i] = get_pp1_energy(fd[i]);
			}

			cnt++;
		}

		usleep(10000);

		for(int i = 0; i < cnt; i++) {
			printf("Package %d:\n", i);
			printf("\tPower units = %.3fW\n", power_units);
			printf("\tEnergy units = %.8fJ\n", get_energy_units(fd[i]));
			printf("\tTime units = %.8fs\n", time_units);
			printf("\tPackage thermal spec: %.0fW\n\n", thermal_spec_power);

			t_val = get_pkg_temperature(fd[i]);
			printf("\tTemperature = %.0fC\n", t_val);

			delta = wtime() - time;
			pkg_2[i] = get_pkg_energy(fd[i]);
			pp0_2[i]= get_pp0_energy(fd[i]);

			printf("\tPackage power: %.6fW\n", (pkg_2[i]-pkg_1[i])/delta);
			printf("\tCores power (PP0): %.6fW\n",
				(pp0_2[i]-pp0_1[i])/delta);

			if (pp1_flag) {
				pp1_2[i] = get_pp1_energy(fd[i]);
				printf("\tiGPU power (PP1): %.6fW\n",
					(pp1_2[i]-pp1_1[i])/delta);
			}

			close_pkg(fd[i]);
		}

		return 0;
	}
	catch(Readerr)
	{
		return -2;
	}
}

int main()
{
	int ret;
	int arch;

	arch = cpu_arch();

	if (arch < 0) {
		return -3;
	}
	else {
		ret = rapl_info(arch);
		switch(ret) {
			case -1:
				printf("Can not open msr_pkg. The msr_driver module must be installed\n");
				break;
			case -2:
				printf("Can not read msr_pkg\n");
				break;
			default:
				break;
		}
	}

	return ret;
}
