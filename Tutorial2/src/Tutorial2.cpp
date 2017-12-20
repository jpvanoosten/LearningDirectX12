#include <Tutorial2.h>

#include <d3dx12.h>

#include <Application.h>
#include <CommandQueue.h>
#include <Window.h>

Tutorial2::Tutorial2( const std::wstring& name, int width, int height, bool vSync )
    : super(name, width, height, vSync)
{
}

bool Tutorial2::LoadContent()
{
    return true;
}

void Tutorial2::UnloadContent()
{

}

void Tutorial2::OnUpdate(UpdateEventArgs& e)
{
    static uint64_t frameCount = 0;
    static double totalTime = 0.0;
    
    totalTime += e.ElapsedTime;
    frameCount++;

    if (totalTime > 1.0)
    {
        double fps = frameCount / totalTime;

        char buffer[512];
        sprintf_s(buffer, "FPS: %f\n", fps);
        OutputDebugStringA(buffer);

        frameCount = 0;
        totalTime = 0.0;
    }
}

void Tutorial2::OnRender(RenderEventArgs& e)
{

    auto commandQueue = Application::Get().GetCommandQueue(D3D12_COMMAND_LIST_TYPE_DIRECT);
    auto commandList = commandQueue->GetCommandList();

    UINT currentBackBufferIndex = m_pWindow->GetCurrentBackBufferIndex();
    auto backBuffer = m_pWindow->GetCurrentBackBuffer();

    // Clear the render target.
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET);

        commandList->ResourceBarrier(1, &barrier);

        FLOAT clearColor[] = { 0.4f, 0.6f, 0.9f, 1.0f };
        auto rtv = m_pWindow->GetCurrentRenderTargetView();

        commandList->ClearRenderTargetView(rtv, clearColor, 0, nullptr);

    }

    // Present
    {
        CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::Transition(
            backBuffer.Get(),
            D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT);
        commandList->ResourceBarrier(1, &barrier);

        m_FenceValues[currentBackBufferIndex] = commandQueue->ExecuteCommandList(commandList);
        currentBackBufferIndex = m_pWindow->Present();

        commandQueue->WaitForFenceValue(m_FenceValues[currentBackBufferIndex]);
    }
}

void Tutorial2::OnKeyPressed(KeyEventArgs& e)
{
    switch (e.Key)
    {
    case KeyCode::Escape:
        Application::Get().Quit(0);
        break;
    case KeyCode::F11:
        m_pWindow->ToggleFullscreen();
        break;
    }
}

void Tutorial2::OnResize(ResizeEventArgs& e)
{
    char buffer[512];
    sprintf_s(buffer, "Resize: %d x %d\n", e.Width, e.Height);
    OutputDebugStringA(buffer);
}
