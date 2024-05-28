#pragma once
#include "ConfigReader.h"
#include "Transform.h"

class OAnimation;
class OAnimationsReader : OConfigReader
{
public:
	explicit OAnimationsReader(const string& ConfigPath)
	    : OConfigReader(ConfigPath) {}
	~OAnimationsReader();
	vector<shared_ptr<OAnimation>> ReadAnimations();
	void SaveAnimations(const vector<shared_ptr<OAnimation>>& Animations);
};
