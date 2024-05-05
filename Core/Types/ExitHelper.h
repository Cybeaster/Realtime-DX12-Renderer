#pragma once
#include <functional>


struct SExitHelper
{
	template<typename Func, typename... Args>
	explicit SExitHelper(Func&& Function, Args&&... Arguments)
		: Function(std::bind(std::forward<Func>(Function), std::forward<Args>(Arguments)...))
	{
	}

	~SExitHelper()
	{
		Function();
	}

public:
	std::function<void()> Function;
};
