#include "RenderItemComponentBase.h"

void OComponentBase::Init(ORenderItem* Other)
{
	Owner = Other;
}

string OComponentBase::GetName() const
{
	return Name;
}
