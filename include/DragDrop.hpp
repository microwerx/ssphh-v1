#ifndef DRAGDROP_HPP
#define DRAGDROP_HPP

#include <memory>

class DragDropImpl;

class DragDrop
{
public:
	DragDrop();
	~DragDrop();

	void Init();
	void Kill();
private:
	bool started = false;
	DragDropImpl *pImpl = nullptr;
};

extern DragDrop dragDrop;

#endif