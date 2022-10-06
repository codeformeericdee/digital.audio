#include "Playback.h"

namespace Playback
{
	map<int, char*>* Playback::DriverCache::SendDrivers()
	{
		return &this->myDrivers;
	}

	void Playback::DriverCache::AddToMap(int driverType, char* driverName)
	{
		this->myDrivers.insert({ driverType, driverName });
	};

	Workstation::DriverManager UseDriverCache(DriverCache* drivers)
	{
		return Workstation::DriverManager::GetInstance(drivers->SendDrivers());
	}
}