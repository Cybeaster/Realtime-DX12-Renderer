
#include "AnimationsReader.h"

#include "Animations/Animations.h"
OAnimationsReader::~OAnimationsReader()
{
}
vector<shared_ptr<OAnimation>> OAnimationsReader::ReadAnimations()
{
	vector<shared_ptr<OAnimation>> result;
	for (const auto& val : PTree.get_child("Animations") | std::views::values)
	{
		auto name = GetAttribute(val, "Name");
		vector<SAnimationFrame> frames;
		for (const auto& frame : val.get_child("Frames") | std::views::values)
		{
			SAnimationFrame animFrame;
			STransform transform;
			GetFloat3(frame, "Position", transform.Position);
			GetFloat3(frame, "Rotation", transform.Rotation);
			GetFloat3(frame, "Scale", transform.Scale);
			animFrame.Transform = transform;
			animFrame.Duration = frame.get<float>("Duration");
			frames.push_back(animFrame);
		}
		auto animation = make_shared<OAnimation>();
		animation->SetName(UTF8ToWString(name));
		animation->SetFrames(frames);
		result.push_back(animation);
	}
	return result;
}

void OAnimationsReader::SaveAnimations(const vector<shared_ptr<OAnimation>>& Animations)
{
	PTree.clear();
	for (const auto& anim : Animations)
	{
		boost::property_tree::ptree animTree;
		animTree.put("Name", WStringToUTF8(anim->GetName()));
		boost::property_tree::ptree frames;
		for (const auto& frame : anim->GetFrames())
		{
			boost::property_tree::ptree frameTree;
			frameTree.put("Duration", frame.Duration);
			frameTree.put("Position.X", frame.Transform.Position.x);
			frameTree.put("Position.Y", frame.Transform.Position.y);
			frameTree.put("Position.Z", frame.Transform.Position.z);

			frameTree.put("Rotation.X", frame.Transform.Rotation.x);
			frameTree.put("Rotation.Y", frame.Transform.Rotation.y);
			frameTree.put("Rotation.Z", frame.Transform.Rotation.z);

			frameTree.put("Scale.X", frame.Transform.Scale.x);
			frameTree.put("Scale.Y", frame.Transform.Scale.y);
			frameTree.put("Scale.Z", frame.Transform.Scale.z);

			frames.push_back(std::make_pair("", frameTree));
		}
		animTree.add_child("Frames", frames);
		PTree.add_child("Animations", animTree);
	}
	write_json(FileName, PTree);
}