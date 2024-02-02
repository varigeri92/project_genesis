#pragma once
#include "../AssetDatabase/Guid.h"

class GnsObject
{
public:
	GnsObject(){guid = gns::core::Guid::GetNewGuid();}
private:
	gns::core::guid guid;
public:
	virtual gns::core::guid GetGuid() const { return guid; }
};
