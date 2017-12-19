/**
* The application class is used to create windows for our application.
*/
#pragma once

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl.h>

#include <memory>
#include <string>

class Window;
class Game;

class Application
{
public:

    /**
    * Create the application singleton with the application instance handle.
    */
    static void Create(HINSTANCE hInst);

    /**
    * Destroy the application instance and all windows created by this application instance.
    */
    static void Destroy();
    /**
    * Get the application singleton.
    */
    static Application& Get();

    /**
    * Create a new DirectX11 render window instance.
    * @param windowName The name of the window. This name will appear in the title bar of the window. This name should be unique.
    * @param clientWidth The width (in pixels) of the window's client area.
    * @param clientHeight The height (in pixels) of the window's client area.
    * @param vSync Should the rendering be synchronized with the vertical refresh rate of the screen.
    * @param windowed If true, the window will be created in windowed mode. If false, the window will be created full-screen.
    * @returns The created window instance. If an error occurred while creating the window an invalid
    * window instance is returned. If a window with the given name already exists, that window will be
    * returned.
    */
    std::shared_ptr<Window> CreateRenderWindow(const std::wstring& windowName, int clientWidth, int clientHeight, bool vSync = true );

    /**
    * Destroy a window given the window name.
    */
    void DestroyWindow(const std::wstring& windowName);
    /**
    * Destroy a window given the window reference.
    */
    void DestroyWindow(std::shared_ptr<Window> window);

    /**
    * Find a window by the window name.
    */
    std::shared_ptr<Window> GetWindowByName(const std::wstring& windowName);

    /**
    * Run the application loop and message pump.
    * @return The error code if an error occurred.
    */
    int Run(std::shared_ptr<Game> pGame);

    /**
    * Request to quit the application and close all windows.
    * @param exitCode The error code to return to the invoking process.
    */
    void Quit(int exitCode = 0);

    Microsoft::WRL::ComPtr<ID3D12Device2> GetDevice() const;

protected:

    // Create an application instance.
    Application(HINSTANCE hInst);
    // Destroy the application instance and all windows associated with this application.
    virtual ~Application();

    Microsoft::WRL::ComPtr<IDXGIAdapter4> GetAdapter(bool bUseWarp);
    Microsoft::WRL::ComPtr<ID3D12Device2> CreateDevice(Microsoft::WRL::ComPtr<IDXGIAdapter4> adapter);

private:
    // The application instance handle that this application was created with.
    HINSTANCE m_hInstance;

    Microsoft::WRL::ComPtr<IDXGIAdapter4> m_dxgiAdapter;
    Microsoft::WRL::ComPtr<ID3D12Device2> m_d3d12Device;

};