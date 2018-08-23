#include "cpuenergy.hpp"

using namespace std;

int open_pkg(int pkg)
{
	int fd;
	string str("/dev/msr_pkg");

	str += to_string(pkg);
	fd = open(str.c_str(), O_RDONLY);

	if (fd < 0) {
		if (errno == ENXIO) {
			cerr << "No such CPU package\n";
		}
		return -1;
	}

	return fd;
}

int close_pkg(int fd)
{
	return close(fd);
}

long long read_pkg(int fd, int reg)
{
	long long data;

	if (pread(fd, &data, sizeof(data), reg) != sizeof(data)) {
		perror("read_pkg");
		return -1;
	}

	return data;
}

double get_pkg_temperature(int fd)
{
	long long ret;
	double tjMax, tVal;

	ret = read_pkg(fd, IA32_TEMPERATURE_TARGET);
	if(ret == -1) {
		throw Readerr();
	}
	tjMax = (ret >> 16) & 0xFF;

	ret = read_pkg(fd, IA32_PACKAGE_THERM_STATUS);
	if(ret == -1) {
		throw Readerr();
	}
	tVal = (ret & 0x007F0000) >> 16;

	return tjMax - tVal;
}

double get_power_units(int fd)
{
	long long ret;

	ret = read_pkg(fd, MSR_RAPL_POWER_UNIT);
	if(ret == -1) {
		throw Readerr();
	}

	return pow(0.5, ret & 0xF);
}

double get_energy_units(int fd)
{
	long long ret;

	ret = read_pkg(fd, MSR_RAPL_POWER_UNIT);
	if(ret == -1) {
		throw Readerr();
	}

	return pow(0.5, (ret >> 8) & 0x1F);
}

double get_time_units(int fd)
{
	long long ret;

	ret = read_pkg(fd, MSR_RAPL_POWER_UNIT);
	if(ret == -1) {
		throw Readerr();
	}

	return pow(0.5, (ret >> 16) & 0xF);
}

double get_thermal_spec_power(int fd)
{
	long long ret;

	ret = read_pkg(fd, MSR_PKG_POWER_INFO);
	if(ret == -1) {
		throw Readerr();
	}

	return get_power_units(fd) * (ret & 0x7FFF);
}

double get_pkg_energy(int fd)
{
	long long ret;

	ret = read_pkg(fd, MSR_PKG_ENERGY_STATUS);
	if(ret == -1) {
		throw Readerr();
	}

	return ret * get_energy_units(fd);
}

double get_pp0_energy(int fd)
{
	long long ret;

	ret = read_pkg(fd, MSR_PP0_ENERGY_STATUS);
	if(ret == -1) {
		throw Readerr();
	}

	return ret * get_energy_units(fd);
}

double get_pp1_energy(int fd)
{
	long long ret;

	ret = read_pkg(fd, MSR_PP1_ENERGY_STATUS);
	if(ret == -1) {
		throw Readerr();
	}

	return ret * get_energy_units(fd);
}
