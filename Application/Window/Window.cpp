//
// Created by Cybea on 02/12/2023.
//

#include "Window.h"
const wstring& OWindow::GetName() const
{
	return Name;
}
uint32_t OWindow::GetWidth() const
{
	return Width;
}
uint32_t OWindow::GetHeight() const
{
	return Height;
}
UINT OWindow::GetCurrentBackBufferIndex() const
{
	return CurrentBackBufferIndex;
}