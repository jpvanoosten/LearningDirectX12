/*
 *  Copyright(c) 2017 Jeremiah van Oosten
 *
 *  Permission is hereby granted, free of charge, to any person obtaining a copy
 *  of this software and associated documentation files(the "Software"), to deal
 *  in the Software without restriction, including without limitation the rights
 *  to use, copy, modify, merge, publish, distribute, sublicense, and / or sell
 *  copies of the Software, and to permit persons to whom the Software is
 *  furnished to do so, subject to the following conditions:
 *
 *  The above copyright notice and this permission notice shall be included in
 *  all copies or substantial portions of the Software.
 *
 *  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 *  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 *  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE
 *  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 *  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 *  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
 *  IN THE SOFTWARE.
 */

/**
 *  @file DX12FrameworkView.h
 *  @date August 25, 2017
 *  @author Jeremiah van Oosten
 *
 *  @brief The framework view is the primary 
 */
#include <DX12Game.h>

ref class DX12FrameworkView sealed : Windows::ApplicationModel::Core::IFrameworkView
{
public:
    // Inherited via IFrameworkView
    virtual void Initialize(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView);
    virtual void SetWindow(Windows::UI::Core::CoreWindow^ window);
    virtual void Load(Platform::String^ entryPoint);
    virtual void Run();
    virtual void Uninitialize();

private:
    friend ref class DX12FrameworkViewSource; // Only the view source needs to construct this object.

    void OnActivated(Windows::ApplicationModel::Core::CoreApplicationView^ applicationView, Windows::ApplicationModel::Activation::IActivatedEventArgs^ args);
    void OnKeyDown(Windows::UI::Core::CoreWindow^ window, Windows::UI::Core::KeyEventArgs^ keyEventArgs);
    void OnKeyUp(Windows::UI::Core::CoreWindow^ window, Windows::UI::Core::KeyEventArgs^ keyEventArgs);
    void OnClosed(Windows::UI::Core::CoreWindow^ window, Windows::UI::Core::CoreWindowEventArgs^ keyEventArgs);

    DX12FrameworkView(DX12Game* pGame);
    DX12Game* m_pGame;
};