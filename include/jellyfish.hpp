#ifndef JELLYFISH_HPP
#define JELLYFISH_HPP

#include <viperfish_widget.hpp>

// Jellyfish is a turntable widget
class Jellyfish : public Vf::Widget
{
  public:
	Jellyfish();
	Jellyfish(const std::string &name);
	virtual ~Jellyfish() override;

	using SharedPtr = std::shared_ptr<Jellyfish>;
	using UniquePtr = std::unique_ptr<Jellyfish>;

	template <class... _Types>
	static SharedPtr MakeShared(const std::string &name, _Types &&... _Args)
	{
		Jellyfish *w = new Jellyfish(name, std::forward<_Types>(_Args)...);
		return SharedPtr(w);
	}

	template <class... _Types>
	static UniquePtr MakeUnique(const std::string &name, _Types &&... _Args)
	{
		Jellyfish *w = new Jellyfish(name, std::forward<_Types>(_Args)...);
		return UniquePtr(w);
	}

	virtual void OnInit(const std::vector<std::string> &args) override;
	virtual void OnKill() override;
};

#endif
