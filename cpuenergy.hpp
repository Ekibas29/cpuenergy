#ifndef CPUENERGY_H
#define CPUENERGY_H

#include <iostream>
#include <fcntl.h>
#include <unistd.h>
#include <cmath>
#include <string>

#define MAX_PACKAGES 32
#define MSR_RAPL_POWER_UNIT		0x606
#define IA32_TEMPERATURE_TARGET 0x1A2
#define IA32_PACKAGE_THERM_STATUS   0x1B1
#define MSR_PKG_ENERGY_STATUS		0x611
#define MSR_PKG_POWER_INFO		0x614
#define MSR_PP0_ENERGY_STATUS	0x639
#define MSR_PP1_ENERGY_STATUS	0x641

#define ARCH_ATOM_SILVERMONT 0x37
#define ARCH_ATOM_AIRMONT	 0x4c
#define ARCH_ATOM_MERRIFIELD 0x4a
#define ARCH_ATOM_MOOREFIELD 0x5a
#define ARCH_ATOM_GOLDMONT	 0x5c
#define ARCH_ATOM_DENVERTON	 0x5f
#define ARCH_ATOM_GOLDMONT_P 0x7a
#define ARCH_SANDYBRIDGE	 0x2a
#define ARCH_SANDYBRIDGE_EP	 0x2d
#define ARCH_IVYBRIDGE		 0x3a
#define ARCH_IVYBRIDGE_EP	 0x3e
#define ARCH_HASWELL		 0x3c
#define ARCH_HASWELL_ULT	 0x45
#define ARCH_HASWELL_GT3E	 0x46
#define ARCH_HASWELL_EP		 0x3f
#define ARCH_BROADWELL_M	 0x3d
#define ARCH_BROADWELL	     0x47
#define ARCH_BROADWELL_EP	 0x4f
#define ARCH_BROADWELL_DE	 0x56
#define ARCH_SKYLAKE		 0x4e
#define ARCH_SKYLAKE_S		 0x5e
#define ARCH_SKYLAKE_SP		 0x55
#define ARCH_KNIGHTS_LANDING 0x57
#define ARCH_KNIGHTS_MILL	 0x85
#define ARCH_KABYLAKE	     0x8e
#define ARCH_COFFEELAKE		 0x9e

class Readerr {};

int open_pkg(int pkg);
int close_pkg(int fd);
long long read_pkg(int fd, int reg);
double get_pkg_temperature(int fd);
double get_power_units(int fd);
double get_energy_units(int fd);
double get_time_units(int fd);
double get_thermal_spec_power(int fd);
double get_pkg_energy(int fd);
double get_pp0_energy(int fd);
double get_pp1_energy(int fd);

#endif
