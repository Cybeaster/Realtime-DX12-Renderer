
#include "AnimationsReader.h"

#include "Animations/Animations.h"
#include "MathUtils.h"
OAnimationsReader::~OAnimationsReader()
{
}
vector<shared_ptr<OAnimation>> OAnimationsReader::ReadAnimations()
{
	ReloadConfig();
	vector<shared_ptr<OAnimation>> result;
	for (const auto& val : PTree.get_child("Animations") | std::views::values)
	{
		auto name = GetAttribute(val, "Name");
		vector<SAnimationFrame> frames;
		for (const auto& frame : val.get_child("Frames") | std::views::values)
		{
			SAnimationFrame animFrame;
			STransform transform;

			DirectX::XMFLOAT3 position;
			GetFloat3(frame, "Position", position);
			transform.Position = Load(position);

			DirectX::XMFLOAT3 rotation;
			GetFloat3(frame, "Rotation", rotation);
			transform.Rotation = Load(rotation);

			DirectX::XMFLOAT3 scale;
			GetFloat3(frame, "Scale", scale);
			transform.Scale = Load(scale);

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
	PTree.put("Animations", "");
	auto animations = PTree.get_child("Animations");
	for (const auto& anim : Animations)
	{
		boost::property_tree::ptree animTree;
		animTree.put("Name", WStringToUTF8(anim->GetName()));
		boost::property_tree::ptree frames;
		for (const auto& frame : anim->GetFrames())
		{
			boost::property_tree::ptree frameTree;
			frameTree.put("Duration", frame.Duration);
			auto pos = frame.Transform.GetFloat3Position();
			frameTree.put("Position.X", pos.x);
			frameTree.put("Position.Y", pos.y);
			frameTree.put("Position.Z", pos.z);

			auto rot = frame.Transform.GetFloat3Rotation();
			frameTree.put("Rotation.X", rot.x);
			frameTree.put("Rotation.Y", rot.y);
			frameTree.put("Rotation.Z", rot.z);

			auto scale = frame.Transform.GetFloat3Scale();
			frameTree.put("Scale.X", scale.x);
			frameTree.put("Scale.Y", scale.y);
			frameTree.put("Scale.Z", scale.z);

			frames.push_back(std::make_pair("", frameTree));
		}
		animTree.add_child("Frames", frames);
		animations.push_back(std::make_pair("", animTree));
	}
	PTree.put_child("Animations", animations);
	write_json(FileName, PTree);
}