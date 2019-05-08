#ifndef DRAGDROP_HPP
#define DRAGDROP_HPP

#include <memory>
#include <string>
#include <vector>

class MyDropTarget;

class DragDrop
{
public:
	DragDrop();
	~DragDrop();

	void Init();
	void Kill();
	void Reset();

	const bool gotDragDrop() const noexcept { return gotText_ || gotUnicodeText_ || gotPaths_; }
	const bool gotText() const noexcept { return gotText_; }
	const bool gotUnicode() const noexcept { return gotUnicodeText_; }
	const bool gotPaths() const noexcept { return gotPaths_; }
	const std::string & text() const noexcept { return text_; }
	const std::wstring & unicodeText() const noexcept { return unicodeText_; }
	const std::vector<std::string> & paths() const noexcept { return paths_; }
private:
	bool started = false;

	friend class MyDropTarget;
	MyDropTarget *pDropTarget = nullptr;

	std::string text_;
	std::wstring unicodeText_;
	std::vector<std::string> paths_;
	bool gotText_;
	bool gotUnicodeText_;
	bool gotPaths_;
};

extern DragDrop dragDrop;

#endif